#ifndef clox_object_h
#define clox_object_h

#include "chunk.h"
#include "common.h"
#include "table.h"
#include "value.h"
#include <stdint.h>

/**
 * @brief Obtains the type tag of an object
 */
#define OBJ_TYPE(object) (AS_OBJ(object)->type)

/**
 * @brief Checks if value is a bound method object
 */
#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)

/**
 * @brief Checks if value is a class object
 */
#define IS_CLASS(value) isObjType(value, OBJ_CLASS)

/**
 * @brief Checks if value is a closure object
 */
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)

/**
 * @brief Checks if value is a function object
 */
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)

/**
 * @brief Checks if value is a class instance object
 */
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)

/**
 * brief Checks if value is a native OS function
 */
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)

/**
 * @brief Checks if value is a string
 */
#define IS_STRING(value) isObjType(value, OBJ_STRING)

/**
 * @brief Helper macro for casting value to a bound method object
 */
#define AS_BOUND_METHOD(value) ((ObjBoundMethod *)AS_OBJ(value))

/**
 * @brief Helper macro for casting value to a class object
 */
#define AS_CLASS(value) ((ObjClass *)AS_OBJ(value))

/**
 * @brief Helper macro for casting value to a closure object
 */
#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))

/**
 * @brief Helper macro for casting value to function object
 */
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))

/**
 * @brief Helper macro for casting value to class instance object.
 */
#define AS_INSTANCE(value) ((ObjInstance *)AS_OBJ(value))

/**
 * brief Helper macro for casting value to native function object
 */
#define AS_NATIVE_OBJ(value) ((ObjNative *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->func)

/**
 * @brief Helper macros for extracting Lox strings and string data
 */
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)

/**
 * @brief Type of heap object
 */
typedef enum {
    OBJ_BOUND_METHOD,
    OBJ_CLASS,
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_INSTANCE,
    OBJ_NATIVE,
    OBJ_STRING,
    OBJ_UPVALUE,
} ObjType;

/**
 * @brief Heap allocated objects in Lox
 */
struct Obj {
    ObjType type;
    bool isMarked;
    struct Obj *next;
};

/**
 * @brief Function object type with it's own bytecode chunk
 */
typedef struct {
    Obj obj;
    uint8_t arity;
    size_t upvalueCount;
    Chunk chunk;
    ObjString *name;
} ObjFunction;

/**
 * @brief Type of native/OS functions hoisted from C into Lox
 */
typedef Value (*NativeFn)(size_t argCount, Value *arg);

/**
 * @brief Native function object
 */
typedef struct {
    Obj obj;
    uint8_t arity;
    NativeFn func;
} ObjNative;

/**
 * @brief Lox internal representation of strings
 */
struct ObjString {
    Obj obj;
    size_t length;
    char *chars;
    uint32_t hash;
};

/**
 * @brief Lox internal representation of Upvalues using the Object System
 * such that values are hooked into the GC.
 */
typedef struct {
    Obj obj;
    Value *location;
    Value closed;
    struct ObjUpvalue *next;
} ObjUpvalue;

/**
 * @brief Lox internal representation of closures
 */
typedef struct {
    Obj obj;
    ObjFunction *func;
    ObjUpvalue **upvalues;
    size_t upvalueCount;
} ObjClosure;

typedef struct {
    Obj obj;
    ObjString *name;
    Table methods;
} ObjClass;

typedef struct {
    Obj obj;
    ObjClass *klass;
    Table fields;
} ObjInstance;

typedef struct {
    Obj obj;
    Value receiver;
    ObjClosure *method;
} ObjBoundMethod;

/**
 * @brief Constructs a new bound method object
 */
ObjBoundMethod *newBoundMethod(VM *vm, Compiler *compiler, Value receiver,
                               ObjClosure *method);

/**
 * @brief Constructs a function object
 */
ObjFunction *newFunction(VM *vm, Compiler *compiler);

/**
 * @brief Constructs a class instance object
 */
ObjInstance *newInstance(VM *vm, Compiler *compiler, ObjClass *klass);

/**
 * @brief Constructs a class object
 */
ObjClass *newClass(VM *vm, Compiler *compiler, ObjString *name);

/**
 * @brief Constructs a closure object
 */
ObjClosure *newClosure(VM *vm, Compiler *compiler, ObjFunction *func);

/**
 * @brief Constructs new native/OS function object
 */
ObjNative *newNative(VM *vm, Compiler *compiler, NativeFn func, uint8_t arity);

/**
 * @brief Takes ownership of raw char data it is passed
 */
ObjString *takeString(VM *vm, Compiler *compiler, size_t length, char *chars);

/**
 * @brief Copies a string literal from scanned texted into string object
 */
ObjString *copyString(VM *vm, Compiler *compiler, size_t length, const char *chars);

/**
 * @brief Constructs an Upvalue from a variable
 */
ObjUpvalue *newUpvalue(VM *vm, Compiler *compiler, Value *slot);

/**
 * @brief Helper function for displaying objects.
 */
void printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && OBJ_TYPE(value) == type;
}

#endif // clox_object_h
