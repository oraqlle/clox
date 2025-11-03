#include <catch2/catch_test_macros.hpp>

// Link clox library objects as C and not as C++
extern "C" {
#include "object.h"
#include "value.h"
#include "vm.h"
}

/**
 * Future Test Cases
 *
 * * Plain string
 * * Contains numbers
 * * Contains single quotes
 * * Contains double quotes
 * * Contains escaped characters
 * * Single newline (direct enter)
 * * Too large of input
 * * Whitespace at the start of the string
 * * Whitespace at the end of the string
 * * Whitespace at both ends of the string
 * * Invalid characters (blank characters)
 * * Invalid characters (unicode etc.)
 * * Unexpected characters (EOF)
 * * In REPL environment
 */

TEST_CASE("base", "[.][scan][builtin]") {

    /**
     * Initialise a VM
     */
    VM vm;
    initVM(&vm);

    SECTION("Create") { SUCCEED("Done"); }

    // Ensure to free VM memory
    freeVM(&vm, NULL);
}

TEST_CASE("ObjNative creation for `scan()`", "[scan][builtin][construct]") {

    /**
     * Initialise a VM
     */
    VM vm;
    initVM(&vm);

    SECTION("Create") { SUCCEED("Done"); }

    /**
     * Ensure to free VM memory
     */
    freeVM(&vm, NULL);
}

TEST_CASE("Calling scan() with plain string", "[scan][builtin][call]") {

    /**
     * Initialise a VM
     */
    VM vm;
    initVM(&vm);

    SECTION("Create") { SUCCEED("Done"); }

    /**
     * Ensure to free VM memory
     */
    freeVM(&vm, NULL);
}
