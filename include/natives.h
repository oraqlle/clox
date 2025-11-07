/**
 * @brief Implementation of Lox natives
 *
 * @file natives.h
 */

#ifndef clox_natives_h
#define clox_natives_h

#include "common.h"
#include "object.h"
#include "value.h"

/**
 * @brief Defines a NativeObj and registers in a VM
 *        such that it can be looked up and called
 *        from within a Lox script or REPL.
 */
void defineNative(VM *vm, Compiler *compiler, const char *name, NativeFn func,
                  uint8_t arity);

/**
 * @brief Binding to clock() function from <time.h>.
 *        This value returned from clokc() is divided
 *        by the number of clock cycles per second as
 *        defined by CLOCKS_PER_SEC.
 */
Value clockNative(VM *vm, Compiler *compiler, size_t argCount, Value *args);

/**
 * @brief Reads text data from stdin or from a
 *        file supplied as a path string in a
 *        lox program.
 */
Value readsNative(VM *vm, Compiler *compiler, size_t argCount, Value *args);

#endif // clox_natives_h
