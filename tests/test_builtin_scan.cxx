#include <catch2/catch_test_macros.hpp>

// Link clox library objects as C and not as C++
extern "C" {
#include "object.h"
#include "value.h"
#include "vm.h"
}

TEST_CASE("ObjNative creation for `scan()`", "[scan][builtin]") {

    // Initialise a VM for each section
    VM vm;
    initVM(&vm);

    SECTION("Create") {
        SUCCEED("Done");
    }

    // Ensure to free VM memory
    freeVM(&vm, NULL);
}

TEST_CASE("abc") {

    // Initialise a VM for each section
    VM vm;
    initVM(&vm);

    SECTION("Create 2") {
        SUCCEED("Done");
    }

    // Ensure to free VM memory
    freeVM(&vm, NULL);
}
