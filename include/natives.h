/**
 * @brief Implementation of Lox natives
 *
 * @file natives.h
 */

#ifndef clox_natives_h
#define clox_natives_h

#include "common.h"
#include "value.h"

/**
 * @brief Binding to clock() function from <time.h>.
 *        This value returned from clokc() is divided
 *        by the number of clock cycles per second as
 *        defined by CLOCKS_PER_SEC.
 */
Value clockNative(size_t argCount, Value *args);

/**
 * @brief Reads text data from stdin or from a
 *        file supplied as a path string in a
 *        lox program.
 */
Value readsNative(size_t argCount, Value *args);

#endif // clox_natives_h
