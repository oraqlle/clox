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

    size_t const bytes_alloc_before = vm.bytesAllocated;
    size_t const string_size = sizeof(ObjString);
    size_t const native_size = sizeof(ObjNative);

    SECTION("Direct ObjNative construction for reads()") {
        ObjNative *ntv = newNative(&vm, NULL, readsNative, 0);

        REQUIRE(ntv->arity == 0);
        REQUIRE(ntv->func == readsNative);

        REQUIRE(&ntv->obj == vm.objects);
        REQUIRE(ntv->obj.type == ObjType::OBJ_NATIVE);

        /**
         * VM should have only allocated bytes for
         * the following:
         *
         * * Creating ObjNative for reads()
         */
        REQUIRE(vm.bytesAllocated == bytes_alloc_before + native_size);
    }

    /**
     * Note: This test case can be removed once
     * reads() is defined directly by a call to
     * initVM(), which will require an update to
     * the test case checking the VM's state after
     * initialisation.
     */
    SECTION("Registration of reads() in VM") {
        defineNative(&vm, NULL, "reads", readsNative, 0);

        /**
         * VM should have only allocated bytes for
         * the following:
         *
         * * Copying the symbol "reads",
         * * Creating a ObjString from "reads",
         * * Creating ObjNative for reads()
         */
        size_t const bytes_alloc_after =
            bytes_alloc_before + native_size + string_size + strlen("reads") + 1;
        REQUIRE(vm.bytesAllocated == bytes_alloc_after);

        REQUIRE(vm.globals.count == 2);
        REQUIRE(vm.globals.capacity == 8);
        REQUIRE(vm.globals.entries != NULL);

        REQUIRE(vm.strings.count == 3);
        REQUIRE(vm.strings.capacity == 8);
        REQUIRE(vm.strings.entries != NULL);

        for (size_t i = 0; i < vm.strings.capacity; i++) {
            Entry *entry = &vm.strings.entries[i];

            if (entry->key != NULL) {
                REQUIRE(((strcmp(entry->key->chars, "init") == 0) ||
                         (strcmp(entry->key->chars, "clock") == 0) ||
                         (strcmp(entry->key->chars, "reads") == 0)));
            }

            REQUIRE(IS_NIL(entry->value));
        }
    }

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
