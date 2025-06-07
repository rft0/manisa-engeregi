#include "co.h"

#include <stdlib.h>
#include <stdio.h>

#include "../utils/hashmap.h"
#include "../utils/darray.h"

static void lnotab_forward(MECodeObject* co, uint8_t offset, int line) {
    static int last_line = 0;
    darray_pushd(co->co_lnotab, offset);

    int line_delta = darray_size(co->co_lnotab) > 1 ? line - last_line : line;
    if (line_delta > 255) {
        darray_pushd(co->co_lnotab, 255);
        line_delta -= 255;

        while (line_delta > 255) {
            darray_pushd(co->co_lnotab, 0);
            darray_pushd(co->co_lnotab, 255);
            line_delta -= 255;
        }
    } else {
        darray_pushd(co->co_lnotab, line_delta);
    }

    last_line = line;
}

int lnotab_get_line_from_ip(uint8_t* lnotab, uint32_t ip) {
    int offset = 0;
    int line = 1;
    int i = 0;
    while (offset < ip) {
        offset += lnotab[i];
        line += lnotab[i + 1];
        i += 2;
    }

    return line;
}

static void co_bc_op(MECodeObject* co, uint8_t op) {
    if (co->co_size + 1 > co->co_capacity) {
        co->co_capacity *= 2;
        co->co_bytecode = (uint8_t*)realloc(co->co_bytecode, co->co_capacity);
    }

    co->co_bytecode[co->co_size++] = op;
}

static void co_bc_opoperand(MECodeObject* co, uint8_t op, uint16_t operand) {
    if (co->co_size + 3 > co->co_capacity) {
        co->co_capacity *= 2;
        co->co_bytecode = (uint8_t*)realloc(co->co_bytecode, co->co_capacity);
    }

    co->co_bytecode[co->co_size++] = op;
    memcpy(&co->co_bytecode[co->co_size], &operand, sizeof(uint16_t));
    co->co_size += sizeof(uint16_t);
}

MECodeObject* co_new(Stmt** stmts) {
    MECodeObject* co = malloc(sizeof(MECodeObject));
    co->co_locals = darray_new(MEObject*);
    co->co_globals = hashmap_new();
    co->co_lnotab = darray_new(uint8_t);
    co->co_capacity = 128;
    co->co_bytecode = (uint8_t*)malloc(co->co_capacity);;
    co->co_size = 0;


    return co;
}

void co_free(MECodeObject* co) {
    if (!co)
        return;

    darray_free(co->co_locals);
    hashmap_free(co->co_globals);
    darray_free(co->co_lnotab);

    if (co->co_bytecode)
        free(co->co_bytecode);

    free(co);
}