#include "table.h"
#include <catch2/catch_test_macros.hpp>

// Link clox library objects as C and not as C++
extern "C" {
#include <string.h>

#include "common.h"
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

TEST_CASE("Set Table Entries", "[table][entry]") {}

TEST_CASE("Finding Table Entries", "[table][lookup]") {}
