#include <catch2/catch_test_macros.hpp>

// Link clox library objects as C and not as C++
extern "C" {
#include "object.h"
#include "value.h"
#include "vm.h"
}

TEST_CASE("VM Initialisation", "[VM][Init]") {

    // Initialise a VM
    VM vm;
    initVM(&vm);

    // Expect memory footprint of various Obj subtypes
    // and the (hash) Table Entry type.
    size_t ObjString_size = sizeof(ObjString);
    size_t ObjNative_size = sizeof(ObjNative);
    size_t Entry_size = sizeof(Entry);

    // clang-format off
    // During the initialisation of the VM, GC tracked
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
    size_t allocation_estimate =
                                (5   /* "init" */
                               + 6   /* "clock" */
                               + (ObjString_size * 2)
                               + (ObjNative_size)
                               + (Entry_size * 8)
                               + (Entry_size * 8));
    // clang-format on

    // VM Stack is Empty
    CHECK(vm.stackTop == vm.stack);
    CHECK(vm.frameCount == 0);
    CHECK(vm.openUpvalues == NULL);

    // Only "clock" builtin is created
    CHECK(vm.objects != NULL);
    CHECK(vm.objects->type == ObjType::OBJ_NATIVE);
    //CHECK(AS_NATIVE_OBJ(vm.objects)->func == clockNative)
    CHECK(vm.objects->next == NULL);

    // Initial heap allocations
    CHECK(vm.bytesAllocated == allocation_estimate);
    CHECK(vm.nextGC == 1024 * 1024); // TODO: Make magic a #define

    // Ensure no Garbage Collection passes have run yet
    CHECK(vm.greyCount == 0);
    CHECK(vm.greyCapacity == 0);
    CHECK(vm.greyStack == NULL);

    // VM::globals Table initialisation.
    // Globals table only has clockNative global
    CHECK(vm.globals.count == 1);
    CHECK(vm.globals.capacity == 8);
    CHECK(vm.globals.entries != NULL);

    // VM::strings Table initialization.
    // Strings table contains the "init" string and the
    // "clock" string for lookup of clockNative
    CHECK(vm.strings.count == 2);
    CHECK(vm.strings.capacity == 8);
    CHECK(vm.strings.entries != NULL);

    // VM::initString set to "init"
    CHECK(strcmp(vm.initString->chars, "init") == 0);

    // Create ASSERTS for this section
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

    // Ensure to free VM memory
    freeVM(&vm, NULL);
}
