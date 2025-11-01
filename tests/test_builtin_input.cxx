#include <catch2/catch_test_macros.hpp>

extern "C" {
#include "object.h"
#include "value.h"
#include "vm.h"
}

TEST_CASE("VM Creation", "[VM]") {
    size_t ObjString_size = sizeof(ObjString);
    size_t ObjNative_size = sizeof(ObjNative);
    size_t Entry_size = sizeof(Entry);

    // During the initiallisation of the VM, GC tracked
    // memory will be created for the following heap
    // objects:
    //
    // * ObjString of "init"                (40B)
    // * Copying of "init" -> "init\0"      (5B)
    // * ObjString of "clock"               (40B)
    // * Copying of "clock" -> "clock\0"    (6B)
    // * ObjNative of clockNative()         (32B)
    // * Entry for VM::globals table     8x (24B)
    // * Entry for VM::strings table     8x (24B) +
    // --------------------------------------------
    //                                      (507B)

    size_t allocation_estimate = (5                    /* "init" */
                                + 6                    /* "clock" */
                                + (ObjString_size * 2)
                                + (ObjNative_size)
                                + (Entry_size * 8)
                                + (Entry_size * 8)
                               );

    VM vm;
    initVM(&vm);

    // Stack is empty
    CHECK(vm.stackTop == vm.stack);
    CHECK(vm.frameCount == 0);
    CHECK(vm.openUpvalues == NULL);

    // Only builtins are created
    CHECK(vm.objects != NULL);

    // Initial GC heap allocations
    CHECK(vm.bytesAllocated == allocation_estimate);
    CHECK(vm.nextGC == 1024 * 1024);

    // GC won't have triggered yet
    CHECK(vm.greyCount == 0);
    CHECK(vm.greyCapacity == 0);
    CHECK(vm.greyStack == NULL);

    // Globals table only has clockNative global
    CHECK(vm.globals.count == 1);
    CHECK(vm.globals.capacity == 8);
    CHECK(vm.globals.entries != NULL);

    // Strings table contains the "init" string
    // and the "clock" string for lookup of
    // clockNative
    CHECK(vm.strings.count == 2);
    CHECK(vm.strings.capacity == 8);
    CHECK(vm.strings.entries != NULL);

    // 'init' string is set
    CHECK(strcmp(vm.initString->chars, "init") == 0);

    printf("Entry size: %zu\n", sizeof(Entry));
    printf("ObjString size: %zu\n", sizeof(ObjString));
    printf("ObjNative size: %zu\n", sizeof(ObjNative));

    printf("%u\n", vm.strings.capacity);
    for (size_t i = 0; i < vm.strings.capacity; i++) {
        Entry *e = &vm.strings.entries[i];

        if (e->key != NULL) {
            printf("%s : ", e->key->chars);
            printValue(e->value);
            printf("\n");
        } else {
            printf("<empty>\n");
        }
    }

    freeVM(&vm, NULL);
}
