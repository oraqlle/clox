#include <catch2/catch_test_macros.hpp>

// Link clox library objects as C and not as C++
extern "C" {
#include "natives.h"
#include "object.h"
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

TEST_CASE("base", "[.][reads][native]") {

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

TEST_CASE("Construction and registration of reads() native",
          "[reads][native][construction]") {

    /**
     * Initialise a VM
     */
    VM vm;
    initVM(&vm);

    SECTION("Direct ObjNative construction for reads()") {
        ObjNative *ntv = newNative(&vm, NULL, readsNative, 1);

        REQUIRE(ntv->arity == 1);
        REQUIRE(ntv->func == readsNative);
    }

    SECTION("Registration of reads() in VM") { SUCCEED("Done"); }

    /**
     * Ensure to free VM memory
     */
    freeVM(&vm, NULL);
}

TEST_CASE("Mocked inputs for reads()", "[reads][native][invocation]") {

    /**
     * Initialise a VM
     */
    VM vm;
    initVM(&vm);

    SECTION("Plain string input") { SUCCEED("Done"); }

    SECTION("Input contains numbers") { SUCCEED("Done"); }

    SECTION("Input contains quotes") { SUCCEED("Done"); }

    SECTION("Input contains escape characters") { SUCCEED("Done"); }

    SECTION("Empty (single newline)") { SUCCEED("Done"); }

    SECTION("Too large of input") { SUCCEED("Done"); }

    SECTION("Input contains whitespace") { SUCCEED("Done"); }

    SECTION("Input contains invalid characters") { SUCCEED("Done"); }

    SECTION("End-of-File") { SUCCEED("Done"); }

    SECTION("REPL env") { SUCCEED("Done"); }

    /**
     * Ensure to free VM memory
     */
    freeVM(&vm, NULL);
}
