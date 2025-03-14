/** @brief Bytecode representation of Lox
 *
 * @file chunk.h
 */

#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

/**
 * @brief Bytecode opcode values.
 */
typedef enum {
    OP_CONSTANT, //< Maybe rename to OP_LITERAL
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_LOCAL,
    OP_SET_GLOBAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_GET_SUPER,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_INVOKE,
    OP_SUPER_INVOKE,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    OP_RETURN,
    OP_CLASS,
    OP_INHERIT,
    OP_METHOD,
} OpCode;

/**
 * @brief Dynamic array of opcodes. An array is considered a 'Chunk' of the larger
 * bytecode program.
 */
typedef struct {
    size_t count;
    size_t capacity;
    uint8_t *code;
    size_t *lines;
    ValueArray constants;
} Chunk;

/**
 * @brief Initialize chunk.
 */
void initChunk(Chunk *chunk);

/**
 * @brief Append new opcode byte to chunk.
 */
void writeChunk(VM *vm, Compiler *compiler, Chunk *chunk, uint8_t byte, size_t line);

/**
 * @brief Adds constant to bytecode chunk's value pool.
 */
uint8_t addConstant(VM *vm, Compiler *compiler, Chunk *chunk, Value value);

/**
 * @brief Frees chunk.
 */
void freeChunk(VM *vm, Compiler *compiler, Chunk *chunk);

#endif // clox_chunk_h
