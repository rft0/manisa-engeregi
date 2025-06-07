#ifndef __CO_H
#define __CO_H

#include <stdint.h>

#include "../utils/hashmap.h"

#include "../parser/stmt.h"

#include "object.h"

typedef struct MECodeObject {
    uint8_t* co_bytecode;
    size_t co_size;
    size_t co_capacity;
    HashMap* co_globals;
    MEObject** co_locals;
    uint8_t* co_lnotab;
} MECodeObject;

typedef enum {
    CO_OP_LOAD_LITERAL,
    CO_OP_LOAD_GLOBAL,
    CO_OP_LOAD_VARIABLE,
    CO_OP_STORE_GLOBAL,
    CO_OP_STORE_VARIABLE,
    CO_OP_BINARY_OP,
    CO_OP_UNARY_OP,
    CO_OP_CALL_FUNCTION,
    CO_OP_RETURN,
} MECodeOp;

MECodeObject* co_new(Stmt** stmts);

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