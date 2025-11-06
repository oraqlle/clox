#include <stdio.h>
#include <time.h>

#include "object.h"
#include "value.h"
#include "vm.h"

void defineNative(VM *vm, Compiler *compiler, const char *name, NativeFn func,
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

Value clockNative(size_t argCount, Value *args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

Value readsNative(size_t argCount, Value *args) {
    FILE *in = stdin;
    int c = fgetc(stdin);

    if (c != EOF) {
        return TRUE_VAL;
    }

    return FALSE_VAL;
}
