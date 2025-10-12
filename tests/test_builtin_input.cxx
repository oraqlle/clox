#include <catch2/catch_test_macros.hpp>

extern "C" {
    #include "compiler.h"
    #include "vm.h"
}

unsigned int Factorial(unsigned int number) {
    return number <= 1 ? number : Factorial(number - 1) * number;
}

TEST_CASE("Factorials are computed", "[factorial]") {
    REQUIRE(Factorial(1) == 1);
    REQUIRE(Factorial(2) == 2);
    REQUIRE(Factorial(3) == 6);
    REQUIRE(Factorial(10) == 3628800);
}

TEST_CASE("VM Creation", "[VM]") {
    VM vm;
    initVM(&vm);

    Parser parser;
    parser.hadError = false;
    parser.panicMode = false;

    Compiler compiler;
    initCompiler(&compiler, NULL, TYPE_SCRIPT, &parser, &vm);

    // Stack is empty
    REQUIRE(vm.stackTop == vm.stack);
    REQUIRE(vm.frameCount == 0);
    REQUIRE(vm.openUpvalues == NULL);

    // No objects created
    REQUIRE(vm.objects == NULL);
    REQUIRE(vm.bytesAllocated == 0);

    // GC is empty
    REQUIRE(vm.nextGC == 1024 * 1024);
    REQUIRE(vm.greyCount == 0);
    REQUIRE(vm.greyCapacity == 0);
    REQUIRE(vm.greyStack == NULL);

    // Globals table only has "clock" global
    REQUIRE(vm.globals.count == 1);
    REQUIRE(vm.globals.capacity == 0);
    REQUIRE(vm.globals.entries == NULL);

    // Strings table is empty
    REQUIRE(vm.strings.count == 0);
    REQUIRE(vm.strings.capacity == 0);
    REQUIRE(vm.strings.entries == NULL);

    // 'init' string is set
    REQUIRE(strcmp(vm.initString->chars, "init") == 0);

    freeVM(&vm, &compiler);
}
