#include <catch2/catch_test_macros.hpp>
#include <cstring>

// Link clox library objects as C and not as C++
extern "C" {
#include "common.h"
#include "natives.h"
#include "object.h"
#include "value.h"
#include "vm.h"
}

TEST_CASE("VM Initialisation", "[VM][init]") {

    /**
     * Initialise a VM
     */
    VM vm;
    initVM(&vm);

    /**
     * Expected memory footprint of various Obj subtypes
     * and the (hash) Table Entry type.
     */
    size_t ObjString_size = sizeof(ObjString);
    size_t ObjNative_size = sizeof(ObjNative);
    size_t Entry_size = sizeof(Entry);

    // clang-format off

    /**
     * During the initialisation of the VM, GC tracked
     * memory will be created for the following heap
     * objects:
     *
     * * ObjString of "init"                (40B)
     * * Copying of "init" -> "init\0"      (5B)
     * * ObjString of "clock"               (40B)
     * * Copying of "clock" -> "clock\0"    (6B)
     * * ObjNative of clockNative()         (32B)
     * * Entry for VM::globals table     8x (24B)
     * * Entry for VM::strings table     8x (24B) +
     * --------------------------------------------
     *                                      (507B)
     */
    size_t allocation_estimate =
                                (5   /* "init" */
                               + 6   /* "clock" */
                               + (ObjString_size * 2)
                               + (ObjNative_size)
                               + (Entry_size * 8)
                               + (Entry_size * 8));
    // clang-format on

    /**
     * VM Stack is Empty
     */
    REQUIRE(vm.stackTop == vm.stack);
    REQUIRE(vm.frameCount == 0);
    REQUIRE(vm.openUpvalues == NULL);

    /**
     * Initial heap allocations
     */
    REQUIRE(vm.bytesAllocated == allocation_estimate);
    REQUIRE(vm.nextGC == CLOX_INIT_GC_PASS);

    /**
     * Ensure no Garbage Collection passes have run yet
     */
    REQUIRE(vm.greyCount == 0);
    REQUIRE(vm.greyCapacity == 0);
    REQUIRE(vm.greyStack == NULL);

    /**
     * VM::globals Table initialisation.
     * Globals table only has clockNative global
     */
    REQUIRE(vm.globals.count == 1);
    REQUIRE(vm.globals.capacity == 8);
    REQUIRE(vm.globals.entries != NULL);

    /**
     * VM::strings Table initialization.
     * Strings table contains the "init" string and the
     * "clock" string for lookup of clockNative
     */
    REQUIRE(vm.strings.count == 2);
    REQUIRE(vm.strings.capacity == 8);
    REQUIRE(vm.strings.entries != NULL);

    /**
     * VM::initString set to "init"
     */
    REQUIRE(strcmp(vm.initString->chars, "init") == 0);

    /**
     * Current objects should only be the ObjString
     * pointed to by VM::initString and the
     * ObjStrings + ObjNatives for natives
     *
     * TODO: Make into loop
     */
    Obj *obj = vm.objects;
    REQUIRE(obj != NULL);

    // clockNative
    REQUIRE(obj->type == ObjType::OBJ_NATIVE);
    REQUIRE(((ObjNative *)obj)->func == clockNative);
    REQUIRE(obj->next != NULL);

    // Native "clock" symbol
    obj = obj->next;
    REQUIRE(obj->type == ObjType::OBJ_STRING);
    REQUIRE(strcmp(((ObjString *)obj)->chars, "clock") == 0);
    REQUIRE(obj->next != NULL);

    // "init"
    obj = obj->next;
    REQUIRE(obj->type == ObjType::OBJ_STRING);
    REQUIRE(strcmp(((ObjString *)obj)->chars, "init") == 0);
    REQUIRE(obj->next == NULL);

    /**
     * Only symbols for natives and the init string
     * are stored in the VM::strings table
     */
    for (size_t i = 0; i < vm.strings.capacity; i++) {
        Entry *entry = &vm.strings.entries[i];

        if (entry->key != NULL) {
            REQUIRE(((strcmp(entry->key->chars, "init") == 0) ||
                     (strcmp(entry->key->chars, "clock") == 0)));
        }

        REQUIRE(IS_NIL(entry->value));
    }

    /**
     * Ensure to free VM memory
     */
    freeVM(&vm, NULL);
}
