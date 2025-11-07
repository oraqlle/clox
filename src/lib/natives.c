#include <stdio.h>
#include <time.h>

#include "common.h"
#include "memory.h"
#include "natives.h"
#include "object.h"
#include "table.h"
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

Value clockNative(VM *vm, Compiler *compiler, size_t argCount, Value *args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

Value readsNative(VM *vm, Compiler *compiler, size_t argCount, Value *args) {

    // Larger static buffer to initially read input into
    static char reads_buf[READS_MAX_BUF_SIZE] = {0};

    size_t idx = 0;
    int in = 0;

    while ((in = fgetc(stdin)) && (in != EOF) && (in != '\n')) {
        char chr = (char)in;
        reads_buf[idx] = chr;
        idx += 1;
    }

    char *chars = ALLOCATE(vm, compiler, char, idx + 1);
    memcpy(chars, &reads_buf[0], idx);
    chars[idx] = '\0';
    ObjString *str = takeString(vm, compiler, idx, chars);

    return OBJ_VAL(str);
}
