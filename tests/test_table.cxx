#include <catch2/catch_test_macros.hpp>

// Link clox library objects as C and not as C++
extern "C" {
#include <string.h>

#include "common.h"
#include "natives.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

// Copy-pasted hash function from object.c
// Copied to prevent adding the source hashString
// method to the API of object.h
static uint32_t hashStr(const char *key, size_t length) {
    uint32_t hash = 2166136261U;

    for (size_t i = 0; i < length; i++) {
        hash ^= (uint8_t)key[0];
        hash *= 16777619;
    }

    return hash;
}

// Test Value has to linked like C so C++
// compilers do not complain about struct
// initialisation
#ifdef NAN_BOXING
Value val = NIL_VAL;
#else
Value val = {VAL_NIL, {0}};
#endif
}

TEST_CASE("Finding Interned Strings", "[table][lookup]") {

    VM vm;
    initVM(&vm);

    SECTION("Empty String") {
        const char *str = "";
        size_t len = strlen(str);
        uint32_t hash = hashStr(str, len);

        ObjString *obj = tableFindString(&vm.strings, str, len, hash);

        REQUIRE(obj == NULL);
    }

    SECTION("Existing String") {
        const char *str = "clock";
        size_t len = strlen(str);
        uint32_t hash = hashStr(str, len);

        ObjString *obj = tableFindString(&vm.strings, str, len, hash);

        REQUIRE(obj != NULL);
        REQUIRE(obj->hash == hash);
        REQUIRE(obj->length == len);
        REQUIRE(strcmp(obj->chars, str) == 0);
    }

    SECTION("Non-existing string") {
        const char *str = "format";
        size_t len = strlen(str);
        uint32_t hash = hashStr(str, len);

        ObjString *obj = tableFindString(&vm.strings, str, len, hash);

        REQUIRE(obj == NULL);
    }

    SECTION("Invalid string pointer") {
        const char *str = NULL;
        // Cannot hash or strlen a NULL pointer string

        ObjString *obj = tableFindString(&vm.strings, str, 0, 0);

        REQUIRE(obj == NULL);
    }
}

TEST_CASE("Finding Table Entries", "[table][lookup]") {

    VM vm;
    initVM(&vm);

#ifdef NAN_BOXING
Value val = NIL_VAL;
#else
Value val = {VAL_NIL, {0}};
#endif

    SECTION("Lookup in VM::globals table") {

        SECTION("Empty string key") {

            const char *str = "";
            size_t len = strlen(str);
            uint32_t hash = hashStr(str, len);

            ObjString key = {
                /* obj */ {/* type */ OBJ_STRING,
                           /* isMarked */ false,
                           /* next */ NULL},
                /* length */ len,
                /* chars */ const_cast<char *>(str),
                /* hash */ hash,
            };

            REQUIRE_FALSE(tableGet(&vm.globals, &key, &val));
            REQUIRE(IS_NIL(val));
        }

        SECTION("Existing string key") {

            const char *str = "clock";
            size_t len = strlen(str);
            uint32_t hash = hashStr(str, len);

            ObjString *key = tableFindString(&vm.strings, str, len, hash);

            REQUIRE(tableGet(&vm.globals, key, &val));
            REQUIRE(IS_NATIVE(val));

            ObjNative *ntv = ((ObjNative *)AS_OBJ(val));
            REQUIRE(ntv->arity == 0);
            REQUIRE(ntv->func == clockNative);
        }

        SECTION("Non-existing string key") {

            const char *str = "format";
            size_t len = strlen(str);
            uint32_t hash = hashStr(str, len);

            ObjString key = {
                /* obj */ {/* type */ OBJ_STRING,
                           /* isMarked */ false,
                           /* next */ NULL},
                /* length */ len,
                /* chars */ const_cast<char *>(str),
                /* hash */ hash,
            };

            REQUIRE_FALSE(tableGet(&vm.globals, &key, &val));
            REQUIRE(IS_NIL(val));
        }

        SECTION("Invalid string key") {

            const char *str = NULL;

            ObjString key = {
                /* obj */ {/* type */ OBJ_STRING,
                           /* isMarked */ false,
                           /* next */ NULL},
                /* length */ 0,
                /* chars */ const_cast<char *>(str),
                /* hash */ 0,
            };

            REQUIRE_FALSE(tableGet(&vm.globals, &key, &val));
            REQUIRE(IS_NIL(val));
        }
    }

    SECTION("Lookup in VM::strings table") {

        SECTION("Empty string key") {

            const char *str = "";
            size_t len = strlen(str);
            uint32_t hash = hashStr(str, len);

            ObjString key = {
                /* obj */ {/* type */ OBJ_STRING,
                           /* isMarked */ false,
                           /* next */ NULL},
                /* length */ len,
                /* chars */ const_cast<char *>(str),
                /* hash */ hash,
            };

            REQUIRE_FALSE(tableGet(&vm.strings, &key, &val));
            REQUIRE(IS_NIL(val));
        }

        SECTION("Existing string key") {

            const char *str = "clock";
            size_t len = strlen(str);
            uint32_t hash = hashStr(str, len);

            ObjString *key = tableFindString(&vm.strings, str, len, hash);

            REQUIRE(tableGet(&vm.strings, key, &val));
            REQUIRE(IS_NIL(val));

            ObjString *obj = ((ObjString *)AS_OBJ(val));
            REQUIRE(key != obj); // Won't be the same as different object was used to intern table entry
            REQUIRE(key->length == len);
            REQUIRE(key->hash == hash);
            REQUIRE(strcmp(key->chars, str) == 0);
        }

        SECTION("Non-existing string key") {

            const char *str = "format";
            size_t len = strlen(str);
            uint32_t hash = hashStr(str, len);

            ObjString key = {
                /* obj */ {/* type */ OBJ_STRING,
                           /* isMarked */ false,
                           /* next */ NULL},
                /* length */ len,
                /* chars */ const_cast<char *>(str),
                /* hash */ hash,
            };

            REQUIRE_FALSE(tableGet(&vm.strings, &key, &val));
            REQUIRE(IS_NIL(val));
        }

        SECTION("Invalid string key") {

            const char *str = NULL;

            ObjString key = {
                /* obj */ {/* type */ OBJ_STRING,
                           /* isMarked */ false,
                           /* next */ NULL},
                /* length */ 0,
                /* chars */ const_cast<char *>(str),
                /* hash */ 0,
            };

            REQUIRE_FALSE(tableGet(&vm.strings, &key, &val));
            REQUIRE(IS_NIL(val));
        }
    }
}

TEST_CASE("Set Table Entries", "[table][entry]") {

    VM vm;
    initVM(&vm);

#ifdef NAN_BOXING
Value val = NIL_VAL;
#else
Value val = {VAL_NIL, {0}};
#endif

    SECTION("Setting VM::globals table") {

        SECTION("Empty string key") {

            const char *str = "";
            size_t len = strlen(str);
            uint32_t hash = hashStr(str, len);

            ObjString key = {
                /* obj */ {/* type */ OBJ_STRING,
                           /* isMarked */ false,
                           /* next */ NULL},
                /* length */ len,
                /* chars */ const_cast<char *>(str),
                /* hash */ hash,
            };

            REQUIRE(tableSet(&vm, NULL, &vm.globals, &key, val));
        }

        SECTION("Existing string") {

            const char *str = "clock";
            size_t len = strlen(str);
            uint32_t hash = hashStr(str, len);
            ObjString *key = tableFindString(&vm.strings, str, len, hash);

            REQUIRE_FALSE(tableSet(&vm, NULL, &vm.globals, key, val));
        }

        SECTION("Non-existing string key") {

            const char *str = "format";
            size_t len = strlen(str);
            uint32_t hash = hashStr(str, len);

            ObjString key = {
                /* obj */ {/* type */ OBJ_STRING,
                           /* isMarked */ false,
                           /* next */ NULL},
                /* length */ len,
                /* chars */ const_cast<char *>(str),
                /* hash */ hash,
            };

            REQUIRE(tableSet(&vm, NULL, &vm.globals, &key, val));
        }

        SECTION("Invalid string key") {

            const char *str = NULL;

            ObjString key = {
                /* obj */ {/* type */ OBJ_STRING,
                           /* isMarked */ false,
                           /* next */ NULL},
                /* length */ 0,
                /* chars */ const_cast<char *>(str),
                /* hash */ 0,
            };

            REQUIRE_FALSE(tableSet(&vm, NULL, &vm.globals, &key, val));
        }
    }

    SECTION("Setting VM::strings table") {

        SECTION("Empty string key") {

            const char *str = "";
            size_t len = strlen(str);
            uint32_t hash = hashStr(str, len);

            ObjString key = {
                /* obj */ {/* type */ OBJ_STRING,
                           /* isMarked */ false,
                           /* next */ NULL},
                /* length */ len,
                /* chars */ const_cast<char *>(str),
                /* hash */ hash,
            };

            REQUIRE(tableSet(&vm, NULL, &vm.strings, &key, val));
        }

        SECTION("Existing string") {

            // Possibly a malformed test case due to needing
            // the pointer of the exact string object to
            // successfully compare keys due to string interning
            // SKIP()

            const char *str = "clock";
            size_t len = strlen(str);
            uint32_t hash = hashStr(str, len);
            ObjString *key = tableFindString(&vm.strings, str, len, hash);

            REQUIRE_FALSE(tableSet(&vm, NULL, &vm.strings, key, val));
        }

        SECTION("Non-existing string key") {

            const char *str = "format";
            size_t len = strlen(str);
            uint32_t hash = hashStr(str, len);

            ObjString key = {
                /* obj */ {/* type */ OBJ_STRING,
                           /* isMarked */ false,
                           /* next */ NULL},
                /* length */ len,
                /* chars */ const_cast<char *>(str),
                /* hash */ hash,
            };

            REQUIRE(tableSet(&vm, NULL, &vm.strings, &key, val));
        }

        SECTION("Invalid string key") {

            const char *str = NULL;

            ObjString key = {
                /* obj */ {/* type */ OBJ_STRING,
                           /* isMarked */ false,
                           /* next */ NULL},
                /* length */ 0,
                /* chars */ const_cast<char *>(str),
                /* hash */ 0,
            };

            REQUIRE_FALSE(tableSet(&vm, NULL, &vm.strings, &key, val));
        }
    }
}
