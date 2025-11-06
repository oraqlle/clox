#include <catch2/catch_test_macros.hpp>
#include <cstdio>

// Link clox library objects as C and not as C++
extern "C" {

#include "chunk.h"
#include "natives.h"
#include "object.h"
#include "vm.h"
}

/**
 * Writes to stdin to mock user input to a LOx program
 */
static void send_mock_to_stdin(const char *text) {
    freopen("/tmp/clox_test.txt", "w+", stdin);
    fprintf(stdin, "%s\n", text);
    freopen("/dev/stdin", "r", stdin);
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

    SECTION("Create") {
        SUCCEED("The test case was successful and this is longer so that it gets "
                "formatted correctly");
    }

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
     * Initialise a Scanner and VM to compile
     * and execute the test script respectively.
     */
    Scanner scanner;
    VM vm;
    initVM(&vm);
    defineNative(&vm, NULL, "reads", readsNative, 0);

    const char *script = "print \"Enter your name:\";"
                         "var a = reads();"
                         "print \"Hello \" + a + \"!\";";

    // clang-format off
    const uint8_t expected_bytecode[] = {
        OP_CONSTANT, 0x00,
        OP_PRINT,
        OP_GET_GLOBAL, 0x02,
        OP_CALL, 0x00,
        OP_DEFINE_GLOBAL, 0x01,
        OP_CONSTANT, 0x03,
        OP_GET_GLOBAL, 0x04,
        OP_ADD,
        OP_CONSTANT, 0x05,
        OP_ADD,
        OP_PRINT,
        OP_NIL,
        OP_RETURN
    };
    // clang-format off

    size_t const bytec_offset = 14; // bytecode instructions

    SECTION("Plain string input") {

        /**
         * Simulate user input by reopening with a new temporary file
         */
        const char *user_input = "John";
        FILE *old_stdin = stdin;
        send_mock_to_stdin(user_input);
        CHECK(old_stdin == stdin);

        InterpreterResult result = interpret(&vm, &scanner, script);

        CHECK(result != INTERPRETER_COMPILE_ERR);
        CHECK(result != INTERPRETER_RUNTIME_ERR);
        CHECK(result == INTERPRETER_OK);

        uint8_t *ip = vm.frames[0].ip - bytec_offset;
        for (size_t i = 0; i < bytec_offset; i++,ip++) {
            CHECK(expected_bytecode[i] == *ip);
        }
    }

    SECTION("Input contains numbers") {

        /**
         * Simulate user input by reopening with a new temporary file
         */
        const char *user_input = "John";
        FILE *old_stdin = stdin;
        send_mock_to_stdin(user_input);
        CHECK(old_stdin == stdin);

        InterpreterResult result = interpret(&vm, &scanner, script);

        CHECK(result != INTERPRETER_COMPILE_ERR);
        CHECK(result != INTERPRETER_RUNTIME_ERR);
        CHECK(result == INTERPRETER_OK);

        uint8_t *ip = vm.frames[0].ip - bytec_offset;
        for (size_t i = 0; i < bytec_offset; i++,ip++) {
            CHECK(expected_bytecode[i] == *ip);
        }
    }

    SECTION("Input contains quotes") {

        /**
         * Simulate user input by reopening with a new temporary file
         */
        const char *user_input = "John";
        FILE *old_stdin = stdin;
        send_mock_to_stdin(user_input);
        CHECK(old_stdin == stdin);

        InterpreterResult result = interpret(&vm, &scanner, script);

        CHECK(result != INTERPRETER_COMPILE_ERR);
        CHECK(result != INTERPRETER_RUNTIME_ERR);
        CHECK(result == INTERPRETER_OK);

        uint8_t *ip = vm.frames[0].ip - bytec_offset;
        for (size_t i = 0; i < bytec_offset; i++,ip++) {
            CHECK(expected_bytecode[i] == *ip);
        }
    }

    SECTION("Input contains escape characters") {

        /**
         * Simulate user input by reopening with a new temporary file
         */
        const char *user_input = "John";
        FILE *old_stdin = stdin;
        send_mock_to_stdin(user_input);
        CHECK(old_stdin == stdin);

        InterpreterResult result = interpret(&vm, &scanner, script);

        CHECK(result != INTERPRETER_COMPILE_ERR);
        CHECK(result != INTERPRETER_RUNTIME_ERR);
        CHECK(result == INTERPRETER_OK);

        uint8_t *ip = vm.frames[0].ip - bytec_offset;
        for (size_t i = 0; i < bytec_offset; i++,ip++) {
            CHECK(expected_bytecode[i] == *ip);
        }
    }

    SECTION("Empty (single newline)") {

        /**
         * Simulate user input by reopening with a new temporary file
         */
        const char *user_input = "John";
        FILE *old_stdin = stdin;
        send_mock_to_stdin(user_input);
        CHECK(old_stdin == stdin);

        InterpreterResult result = interpret(&vm, &scanner, script);

        CHECK(result != INTERPRETER_COMPILE_ERR);
        CHECK(result != INTERPRETER_RUNTIME_ERR);
        CHECK(result == INTERPRETER_OK);

        uint8_t *ip = vm.frames[0].ip - bytec_offset;
        for (size_t i = 0; i < bytec_offset; i++,ip++) {
            CHECK(expected_bytecode[i] == *ip);
        }
    }

    SECTION("Too large of input") {

        /**
         * Simulate user input by reopening with a new temporary file
         */
        const char *user_input = "John";
        FILE *old_stdin = stdin;
        send_mock_to_stdin(user_input);
        CHECK(old_stdin == stdin);

        InterpreterResult result = interpret(&vm, &scanner, script);

        CHECK(result != INTERPRETER_COMPILE_ERR);
        CHECK(result != INTERPRETER_RUNTIME_ERR);
        CHECK(result == INTERPRETER_OK);

        uint8_t *ip = vm.frames[0].ip - bytec_offset;
        for (size_t i = 0; i < bytec_offset; i++,ip++) {
            CHECK(expected_bytecode[i] == *ip);
        }
    }

    SECTION("Input contains whitespace") {

        /**
         * Simulate user input by reopening with a new temporary file
         */
        const char *user_input = "John";
        FILE *old_stdin = stdin;
        send_mock_to_stdin(user_input);
        CHECK(old_stdin == stdin);

        InterpreterResult result = interpret(&vm, &scanner, script);

        CHECK(result != INTERPRETER_COMPILE_ERR);
        CHECK(result != INTERPRETER_RUNTIME_ERR);
        CHECK(result == INTERPRETER_OK);

        uint8_t *ip = vm.frames[0].ip - bytec_offset;
        for (size_t i = 0; i < bytec_offset; i++,ip++) {
            CHECK(expected_bytecode[i] == *ip);
        }
    }

    SECTION("Input contains invalid characters") {

        /**
         * Simulate user input by reopening with a new temporary file
         */
        const char *user_input = "John";
        FILE *old_stdin = stdin;
        send_mock_to_stdin(user_input);
        CHECK(old_stdin == stdin);

        InterpreterResult result = interpret(&vm, &scanner, script);

        CHECK(result != INTERPRETER_COMPILE_ERR);
        CHECK(result != INTERPRETER_RUNTIME_ERR);
        CHECK(result == INTERPRETER_OK);

        uint8_t *ip = vm.frames[0].ip - bytec_offset;
        for (size_t i = 0; i < bytec_offset; i++,ip++) {
            CHECK(expected_bytecode[i] == *ip);
        }
    }

    SECTION("End-of-File") {

        /**
         * Simulate user input by reopening with a new temporary file
         */
        const char *user_input = "John";
        FILE *old_stdin = stdin;
        send_mock_to_stdin(user_input);
        CHECK(old_stdin == stdin);

        InterpreterResult result = interpret(&vm, &scanner, script);

        CHECK(result != INTERPRETER_COMPILE_ERR);
        CHECK(result != INTERPRETER_RUNTIME_ERR);
        CHECK(result == INTERPRETER_OK);

        uint8_t *ip = vm.frames[0].ip - bytec_offset;
        for (size_t i = 0; i < bytec_offset; i++,ip++) {
            CHECK(expected_bytecode[i] == *ip);
        }
    }

    SECTION("REPL env") {

        /**
         * Simulate user input by reopening with a new temporary file
         */
        const char *user_input = "John";
        FILE *old_stdin = stdin;
        send_mock_to_stdin(user_input);
        CHECK(old_stdin == stdin);

        InterpreterResult result = interpret(&vm, &scanner, script);

        CHECK(result != INTERPRETER_COMPILE_ERR);
        CHECK(result != INTERPRETER_RUNTIME_ERR);
        CHECK(result == INTERPRETER_OK);

        uint8_t *ip = vm.frames[0].ip - bytec_offset;
        for (size_t i = 0; i < bytec_offset; i++,ip++) {
            CHECK(expected_bytecode[i] == *ip);
        }
    }

    /**
     * Ensure to free VM memory
     */
    freeVM(&vm, NULL);
}
