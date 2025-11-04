#include <time.h>

#include "natives.h"
#include "value.h"

Value clockNative(size_t argCount, Value *args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

Value scanNative(size_t argCount, Value *args) {
    return NIL_VAL;
}
