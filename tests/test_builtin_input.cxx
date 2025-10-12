#include <catch2/catch_test_macros.hpp>

extern "C" {
    #include "compiler.h"
    #include "vm.h"
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
    CHECK(vm.stackTop == vm.stack);
    CHECK(vm.frameCount == 0);
    CHECK(vm.openUpvalues == NULL);

    // Only builtins are created
    CHECK(vm.objects == NULL);
    CHECK(vm.bytesAllocated == 0);

    // GC is empty
    CHECK(vm.nextGC == 1024 * 1024);
    CHECK(vm.greyCount == 0);
    CHECK(vm.greyCapacity == 0);
    CHECK(vm.greyStack == NULL);

    // Globals table only has "clock" global
    CHECK(vm.globals.count == 1);
    CHECK(vm.globals.capacity == 0);
    CHECK(vm.globals.entries == NULL);

    // Strings table is empty
    CHECK(vm.strings.count == 0);
    CHECK(vm.strings.capacity == 0);
    CHECK(vm.strings.entries == NULL);

    // 'init' string is set
    CHECK(strcmp(vm.initString->chars, "init") == 0);

    freeVM(&vm, &compiler);
}
