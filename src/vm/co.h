#ifndef __CO_H
#define __CO_H

#include "../parser/stmt.h"

#include <stdint.h>

#include "../utils/hashmap.h"

typedef struct MECodeObject {
    uint8_t* bytecode;
    size_t size;
    HashMap* globals;
} MECodeObject;

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