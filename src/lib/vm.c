#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

static void resetStack(VM *vm) {
    vm->stackTop = vm->stack;
    vm->frameCount = 0;
    vm->openUpvalues = NULL;
}

static void runtimeError(VM *vm, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (intmax_t i = (intmax_t)(vm->frameCount - 1); i >= 0; i--) {
        CallFrame *frame = &vm->frames[i];
        ObjFunction *func = frame->closure->func;
        size_t instruction = (size_t)(frame->ip - func->chunk.code - 1);
        size_t line = func->chunk.lines[instruction];

        fprintf(stderr, "[line %zu] in ", line);

        if (func->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", func->name->chars);
        }
    }

    resetStack(vm);
}

static void defineNative(VM *vm, Compiler *compiler, const char *name, NativeFn func,
                         uint8_t arity) {

    if (arity == UINT8_MAX) {
        fprintf(stderr, "Can't have more than 255 parameters in native function %s.\n",
                name);
    }

    push(vm, OBJ_VAL(copyString(vm, compiler, strlen(name), name)));
    push(vm, OBJ_VAL(newNative(vm, compiler, func, arity)));
    tableSet(vm, compiler, &vm->globals, AS_STRING(vm->stack[0]), vm->stack[1]);
    pop(vm);
    pop(vm);
}

static Value peek(VM *vm, int distance) { return vm->stackTop[-1 - distance]; }

static bool call(VM *vm, ObjClosure *closure, uint8_t argCount) {
    if (argCount != closure->func->arity) {
        runtimeError(vm, "Expected %d arguments but got %d.", closure->func->arity,
                     argCount);
        return false;
    }

    if (vm->frameCount == FRAMES_MAX) {
        runtimeError(vm, "Stack overflow");
    }

    CallFrame *frame = &vm->frames[vm->frameCount++];
    frame->closure = closure;
    frame->ip = closure->func->chunk.code;
    frame->slots = vm->stackTop - argCount - 1;

    return true;
}

static bool callValue(VM *vm, Value callee, uint8_t argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_CLOSURE:
                return call(vm, AS_CLOSURE(callee), argCount);
            case OBJ_NATIVE: {
                ObjNative *native = AS_NATIVE_OBJ(callee);

                if (argCount != native->arity) {
                    runtimeError(vm, "Expected %d arguments but got %d.", native->arity,
                                 argCount);
                    return false;
                }

                Value result = native->func(argCount, vm->stackTop - argCount);
                vm->stackTop -= argCount + 1;
                push(vm, result);
                return true;
            }
            default:
                break; // Non-callable object type
        }
    }

    runtimeError(vm, "Can only call functions and classes.");
    return false;
}

static ObjUpvalue *captureUpvalue(VM *vm, Compiler *compiler, Value *local) {
    ObjUpvalue *prevUpvalue = NULL;
    ObjUpvalue *upvalue = vm->openUpvalues;

    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = (ObjUpvalue *)upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }

    ObjUpvalue *createdUpvalue = newUpvalue(vm, compiler, local);
    createdUpvalue->next = (struct ObjUpvalue *)upvalue;

    if (prevUpvalue == NULL) {
        vm->openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = (struct ObjUpvalue *)createdUpvalue;
    }

    return createdUpvalue;
}

static void closeUpvalues(VM *vm, Value *last) {
    while (vm->openUpvalues != NULL && vm->openUpvalues->location >= last) {
        ObjUpvalue *upvalue = vm->openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm->openUpvalues = (ObjUpvalue *)upvalue->next;
    }
}

static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate(VM *vm, Compiler *compiler) {
    ObjString *b = AS_STRING(pop(vm));
    ObjString *a = AS_STRING(pop(vm));

    size_t length = a->length + b->length;
    char *chars = ALLOCATE(vm, compiler, char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString *string = takeString(vm, compiler, length, chars);
    push(vm, OBJ_VAL(string));
}

static Value clockNative(size_t argCount, Value *args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

void initVM(VM *vm) {
    resetStack(vm);
    vm->objects = NULL;

    vm->greyCount = 0;
    vm->greyCapacity = 0;
    vm->greyStack = NULL;

    initTable(&vm->globals);
    initTable(&vm->strings);

    defineNative(vm, NULL, "clock", clockNative, 0);
}

void freeVM(VM *vm, Compiler *compiler) {
    freeObjects(vm, compiler);
    freeTable(vm, compiler, &vm->globals);
    freeTable(vm, compiler, &vm->strings);
}

static InterpreterResult run(VM *vm, Compiler *compiler) {
    CallFrame *frame = &vm->frames[vm->frameCount - 1];

#define READ_BYTE() (*frame->ip++)

#define READ_CONSTANT() ((frame)->closure->func->chunk.constants.values[READ_BYTE()])

#define READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define READ_STRING() AS_STRING(READ_CONSTANT())

#define BINARY_OP(valueType, op)                                                         \
    do {                                                                                 \
        if (!IS_NUMBER(peek(vm, 0)) || !IS_NUMBER(peek(vm, 1))) {                        \
            runtimeError(vm, "Operands must be numbers.");                               \
            return INTERPRETER_RUNTIME_ERR;                                              \
        }                                                                                \
        double b = AS_NUMBER(pop(vm));                                                   \
        double a = AS_NUMBER(pop(vm));                                                   \
        push(vm, valueType(a op b));                                                     \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value *slot = vm->stack; slot < vm->stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");

        disassembleInstruction(&frame->closure->func->chunk,
                               (size_t)(frame->ip - frame->closure->func->chunk.code));
#endif

        uint8_t instruction;

        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(vm, constant);
                break;
            }
            case OP_NIL:
                push(vm, NIL_VAL);
                break;
            case OP_TRUE:
                push(vm, BOOL_VAL(true));
                break;
            case OP_FALSE:
                push(vm, BOOL_VAL(false));
                break;
            case OP_POP:
                pop(vm);
                break;
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(vm, frame->slots[slot]);
                break;
            }
            case OP_GET_GLOBAL: {
                ObjString *name = READ_STRING();
                Value value;

                if (!tableGet(&vm->globals, name, &value)) {
                    runtimeError(vm, "Undefined variable '%s'.", name->chars);
                    return INTERPRETER_RUNTIME_ERR;
                }

                push(vm, value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                ObjString *name = READ_STRING();
                tableSet(vm, compiler, &vm->globals, name, peek(vm, 0));
                pop(vm);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(vm, 0);
                break;
            }
            case OP_SET_GLOBAL: {
                ObjString *name = READ_STRING();

                if (tableSet(vm, compiler, &vm->globals, name, peek(vm, 0))) {
                    tableDelete(&vm->globals, name);
                    runtimeError(vm, "Undefined variable '%s'.", name->chars);
                    return INTERPRETER_RUNTIME_ERR;
                }

                break;
            }
            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                push(vm, *frame->closure->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = peek(vm, 0);
                break;
            }
            case OP_EQUAL: {
                Value b = pop(vm);
                Value a = pop(vm);
                push(vm, BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OP_GREATER:
                BINARY_OP(BOOL_VAL, >);
                break;
            case OP_LESS:
                BINARY_OP(BOOL_VAL, <);
                break;
            case OP_ADD: {
                if (IS_STRING(peek(vm, 0)) && IS_STRING(peek(vm, 1))) {
                    concatenate(vm, compiler);
                } else if (IS_NUMBER(peek(vm, 0)) && IS_NUMBER(peek(vm, 1))) {
                    double b = AS_NUMBER(pop(vm));
                    double a = AS_NUMBER(pop(vm));
                    push(vm, NUMBER_VAL(a + b));
                } else {
                    runtimeError(vm, "Operands must be two numbers or two strings.");
                    return INTERPRETER_RUNTIME_ERR;
                }
                break;
            }
            case OP_SUBTRACT:
                BINARY_OP(NUMBER_VAL, -);
                break;
            case OP_MULTIPLY:
                BINARY_OP(NUMBER_VAL, *);
                break;
            case OP_DIVIDE:
                BINARY_OP(NUMBER_VAL, /);
                break;
            case OP_NOT:
                push(vm, BOOL_VAL(isFalsey(pop(vm))));
                break;
            case OP_NEGATE: {
                if (!IS_NUMBER(peek(vm, 0))) {
                    runtimeError(vm, "Operand must be a number.");
                    return INTERPRETER_RUNTIME_ERR;
                }
                push(vm, NUMBER_VAL(-AS_NUMBER(pop(vm))));
                break;
            }
            case OP_PRINT: {
                printValue(pop(vm));
                printf("\n");
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();

                if (isFalsey(peek(vm, 0))) {
                    frame->ip += offset;
                }

                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }
            case OP_CALL: {
                uint8_t argCount = READ_BYTE();

                if (!callValue(vm, peek(vm, argCount), argCount)) {
                    return INTERPRETER_RUNTIME_ERR;
                }

                frame = &vm->frames[vm->frameCount - 1];
                break;
            }
            case OP_CLOSURE: {
                ObjFunction *func = AS_FUNCTION(READ_CONSTANT());
                ObjClosure *closure = newClosure(vm, compiler, func);
                push(vm, OBJ_VAL(closure));

                for (size_t idx = 0; idx < closure->upvalueCount; idx++) {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();

                    if (isLocal) {
                        closure->upvalues[idx] =
                            captureUpvalue(vm, compiler, frame->slots + index);
                    } else {
                        closure->upvalues[idx] = frame->closure->upvalues[index];
                    }
                }

                break;
            }
            case OP_CLOSE_UPVALUE:
                closeUpvalues(vm, vm->stackTop - 1);
                pop(vm);
                break;
            case OP_RETURN: {
                Value result = pop(vm);
                closeUpvalues(vm, frame->slots);
                vm->frameCount -= 1;

                if (vm->frameCount == 0) {
                    pop(vm);

                    // Exit interpreter
                    return INTERPRETER_OK;
                }

                vm->stackTop = frame->slots;
                push(vm, result);
                frame = &vm->frames[vm->frameCount - 1];
                break;
            }
        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_STRING
#undef BINARY_OP
}

InterpreterResult interpret(VM *vm, Scanner *scanner, const char *source) {
    ObjFunction *func = compile(scanner, source, vm);

    if (func == NULL) {
        return INTERPRETER_COMPILE_ERR;
    }

    push(vm, OBJ_VAL(func));
    ObjClosure *closure = newClosure(vm, NULL, func);
    pop(vm);
    push(vm, OBJ_VAL(closure));
    call(vm, closure, 0);

    return run(vm, NULL);
}

void push(VM *vm, Value value) {
    *vm->stackTop = value;
    vm->stackTop++;
}

Value pop(VM *vm) {
    vm->stackTop--;
    return *vm->stackTop;
}
