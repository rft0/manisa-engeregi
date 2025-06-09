#ifndef __CO_H
#define __CO_H

#include <stdint.h>

#include "../utils/hashmap.h"

#include "../parser/stmt.h"

#include "object.h"

typedef struct MECodeObject {
    char* co_name;
    uint8_t* co_bytecode;
    size_t co_size;
    size_t co_capacity;
    HashMap* co_h_globals;
    HashMap* co_h_locals;
    MEObject** co_consts;
    MEObject** co_locals;
    uint8_t* co_lnotab;
    int in_function;
    uint32_t loop_start;
    uint32_t loop_end_jump;
    uint32_t loop_end_pos;
    uint32_t* break_patches;
} MECodeObject;

typedef enum {
    CO_OP_NOP,
    CO_OP_LOAD_CONST,
    CO_OP_LOAD_GLOBAL,
    CO_OP_LOAD_VARIABLE,
    CO_OP_STORE_GLOBAL,
    CO_OP_STORE_VARIABLE,
    CO_OP_BINARY_OP,
    CO_OP_UNARY_OP,
    CO_OP_CALL_FUNCTION,
    CO_OP_RETURN,
    CO_OP_POP,
    CO_OP_JUMP_REL,
    CO_OP_JUMP_IF_FALSE,
    CO_OP_JUMP_IF_TRUE,
    CO_OP_MAKE_FUNCTION,
    CO_OP_SETUP_LOOP,
    CO_OP_BREAK_LOOP,
    CO_OP_CONTINUE_LOOP,
} MECodeOp;

MECodeObject* co_new(const char* filename, Stmt** stmts);
void co_disasm(MECodeObject* co);

int lnotab_get_line_from_ip(uint8_t* lnotab, uint32_t ip);

#endif

/*
BYTECODE:

LL idx                      - Load Literal
LG idx                      - Load Global
SG idx                      - Store Global
LV idx                      - Load Variable
SV idx                      - Store Variable
BIN op                      - Binary Operation with op
UN op                       - Unary Operation with op
CALL n                      - Call Function with n arguments



*/