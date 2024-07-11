#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"

int main(void) {
    VM vm;
    initVM(&vm);

    Chunk chunk;
    initChunk(&chunk);

    uint8_t constant = addConstant(&chunk, 1.2);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);

    writeChunk(&chunk, OP_RETURN, 123);

    disassembleChunk(&chunk, "test chunk");
    interpret(&vm, &chunk);

    freeChunk(&chunk);

    freeVM(&vm);

    return 0;
}