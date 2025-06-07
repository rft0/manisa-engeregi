#include "co.h"

#include <stdlib.h>
#include <stdio.h>

#include "../utils/hashmap.h"
#include "../utils/darray.h"

#include "objects/longobject.h"
#include "objects/floatobject.h"
#include "objects/strobject.h"
#include "objects/noneobject.h"

#define ME_CO_INITIAL_CAPACITY 256

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

static void co_bc_opoperand(MECodeObject* co, uint8_t op, uint32_t operand, uint16_t operand_size) {
    if (co->co_size + 3 > co->co_capacity) {
        co->co_capacity *= 2;
        co->co_bytecode = (uint8_t*)realloc(co->co_bytecode, co->co_capacity);
    }

    co->co_bytecode[co->co_size++] = op;
    memcpy(&co->co_bytecode[co->co_size], &operand, operand_size);
    co->co_size += operand_size;
}

static uint16_t co_add_literal(MEObject** arr, LiteralExpr* literal) {
    MEObject* obj = NULL;
    
    switch (literal->type) {
        case LITERAL_STRING: {
            char* temp = malloc(literal->value.byte_len + 1);
            memcpy(temp, literal->value.data, literal->value.byte_len);
            temp[literal->value.byte_len] = '\0';
            obj = me_str_from_str(temp);
            free(temp);
            if (obj == NULL)
                return 0;

            break;
        }
        case LITERAL_INT: {
            char* temp = malloc(literal->value.len + 1);
            memcpy(temp, literal->value.data, literal->value.len);
            temp[literal->value.len] = '\0';
            int value = atoi(temp);
            free(temp);
            obj = me_long_from_long(value);
            if (obj == NULL)
                return 0;

            break;
        }
        case LITERAL_FLOAT: {
            char* temp = malloc(literal->value.len + 1);
            memcpy(temp, literal->value.data, literal->value.len);
            temp[literal->value.len] = '\0';
            double value = atof(temp);
            free(temp);
            obj = me_float_from_double(value);
            break;
        }
        case LITERAL_NONE:
            obj = me_none;
            break;
    }
    
    if (obj == NULL)
        return 0;
    
    uint16_t idx = darray_size(arr);
    darray_pushd(arr, obj);
    return idx;
}

static void co_compile_expr(MECodeObject* co, Expr* expr) {
    if (!expr)
        return;
    
    uint16_t idx = 0;
    switch (expr->kind) {
        case EXPR_LITERAL: {
            idx = co_add_literal(co->co_consts, expr->literal);
            co_bc_opoperand(co, CO_OP_LOAD_CONST, idx, 2);
            lnotab_forward(co, 3, expr->line);
            break;
        }
        case EXPR_VARIABLE: {
            StringView var_name = expr->variable->name;
            
            if (hashmap_get(co->co_h_globals, expr->variable->name.data, expr->variable->name.byte_len, NULL)) {
                idx = (uint16_t)(size_t)hashmap_get(co->co_h_globals, expr->variable->name.data, expr->variable->name.byte_len, (uintptr_t*)&idx);
                co_bc_opoperand(co, CO_OP_LOAD_GLOBAL, idx, 2);
            } else if (hashmap_get(co->co_h_locals, expr->variable->name.data, expr->variable->name.byte_len, NULL)) {
                idx = (uint16_t)(size_t)hashmap_get(co->co_h_locals, expr->variable->name.data, expr->variable->name.byte_len, (uintptr_t*)&idx);
                co_bc_opoperand(co, CO_OP_LOAD_VARIABLE, idx, 2);
            }

            lnotab_forward(co, 3, expr->line);
            break;
        }
        case EXPR_BINARY: {
            co_compile_expr(co, expr->binary->lhs);
            co_compile_expr(co, expr->binary->rhs);
            
            co_bc_opoperand(co, CO_OP_BINARY_OP, expr->binary->op, 1);
            lnotab_forward(co, 2, expr->line);
            break;
        }
        case EXPR_UNARY: {
            co_compile_expr(co, expr->unary->operand);
            
            co_bc_opoperand(co, CO_OP_UNARY_OP, expr->unary->op, 1);
            lnotab_forward(co, 2, expr->line);
            break;
        }
        case EXPR_CALL: {
            if (hashmap_get(co->co_h_locals, expr->call->name.data, expr->call->name.byte_len, (uintptr_t*)&idx)) {
                idx = (uint16_t)(size_t)hashmap_get(co->co_h_locals, expr->call->name.data, expr->call->name.byte_len, (uintptr_t*)&idx);
                co_bc_opoperand(co, CO_OP_LOAD_VARIABLE, idx, 2);
            } else if (hashmap_get(co->co_h_globals, expr->call->name.data, expr->call->name.byte_len, (uintptr_t*)&idx)) {
                // THIS IS NOT USED BY NOW BECAUSE WE ASSUME ALL FUNCTIONS ARE LOCALS OF A MODULE BUT IF THERE IS MORE THAN ONE COMPILE UNIT THIS WILL BE NECCESSARY 
                idx = (uint16_t)(size_t)hashmap_get(co->co_h_globals, expr->call->name.data, expr->call->name.byte_len, (uintptr_t*)&idx);
                co_bc_opoperand(co, CO_OP_LOAD_GLOBAL, idx, 2);
            }

            lnotab_forward(co, 3, expr->line);

            for (size_t i = 0; i < darray_size(expr->call->args); i++)
                co_compile_expr(co, expr->call->args[i]);

            co_bc_opoperand(co, CO_OP_CALL_FUNCTION, darray_size(expr->call->args), 1);
            lnotab_forward(co, 2, expr->line);
            break;
        }
    }
}

static void co_compile_stmt(MECodeObject* co, Stmt* stmt) {
    if (!stmt)
        return;
    
    uint16_t idx = 0;

    switch (stmt->kind) {
        case STMT_EXPR:
            co_compile_expr(co, stmt->expr_stmt);
            co_bc_op(co, CO_OP_POP);
            lnotab_forward(co, 1, stmt->line);
            break;
        case STMT_DECL: {
            if (stmt->decl_stmt->initializer)
                co_compile_expr(co, stmt->decl_stmt->initializer);
            else
                co_bc_opoperand(co, CO_OP_LOAD_CONST, 0, 2);

            if (!co->in_function) {
                if (hashmap_get(co->co_h_globals, stmt->decl_stmt->name.data, stmt->decl_stmt->name.byte_len, (uintptr_t*)&idx)) {
                    idx = (uint16_t)(size_t)hashmap_get(co->co_h_globals, stmt->decl_stmt->name.data, stmt->decl_stmt->name.byte_len, (uintptr_t*)&idx);
                    co_bc_opoperand(co, CO_OP_STORE_GLOBAL, idx, 2);
                } else {
                    idx = hashmap_size(co->co_h_globals);
                    hashmap_set(co->co_h_globals, stmt->decl_stmt->name.data, stmt->decl_stmt->name.byte_len, (uintptr_t)idx);
                    co_bc_opoperand(co, CO_OP_STORE_GLOBAL, idx, 2);
                }
            } else {
                if (hashmap_get(co->co_h_locals, stmt->decl_stmt->name.data, stmt->decl_stmt->name.byte_len, (uintptr_t*)&idx)) {
                    idx = (uint16_t)(size_t)hashmap_get(co->co_h_locals, stmt->decl_stmt->name.data, stmt->decl_stmt->name.byte_len, (uintptr_t*)&idx);
                    co_bc_opoperand(co, CO_OP_STORE_VARIABLE, idx, 2);
                } else {
                    idx = hashmap_size(co->co_h_locals);
                    hashmap_set(co->co_h_locals, stmt->decl_stmt->name.data, stmt->decl_stmt->name.byte_len, (uintptr_t)idx);
                    co_bc_opoperand(co, CO_OP_STORE_VARIABLE, idx, 2);
                }
            }

            break;
        }
        case STMT_COMPOUND: {
            for (size_t i = 0; i < darray_size(stmt->compound->stmts); i++)
                co_compile_stmt(co, stmt->compound->stmts[i]);

            break;
        }
        case STMT_RETURN: {
            if (stmt->return_stmt->value)
                co_compile_expr(co, stmt->return_stmt->value);
            else
                co_bc_opoperand(co, CO_OP_LOAD_CONST, 0, 2);

            co_bc_op(co, CO_OP_RETURN);
            lnotab_forward(co, 1, stmt->line);
            break;
        }
        case STMT_IF: {

            
            break;
        }
        case STMT_FUNCTION_DECL: {

        }
        case STMT_WHILE:
        case STMT_BREAK:
        case STMT_CONTINUE:
            break;
        default:
            break;
    }
}

MECodeObject* co_new(Stmt** stmts) {
    MECodeObject* co = malloc(sizeof(MECodeObject));
    co->co_h_globals = hashmap_new();
    co->co_h_locals = hashmap_new();
    co->co_consts = darray_new(MEObject*);
    co->co_locals = darray_new(MEObject*);
    darray_pushd(co->co_consts, me_none);
    darray_pushd(co->co_locals, me_none);
    co->co_lnotab = darray_new(uint8_t);
    co->co_capacity = ME_CO_INITIAL_CAPACITY;
    co->co_bytecode = (uint8_t*)malloc(co->co_capacity);
    memset(co->co_bytecode, 0, co->co_capacity);
    co->co_size = 0;
    co->in_function = 0;

    for (size_t i = 0; i < darray_size(stmts); i++)
        co_compile_stmt(co, stmts[i]);

    return co;
}

void co_free(MECodeObject* co) {
    if (!co)
        return;

    hashmap_free(co->co_h_globals);
    hashmap_free(co->co_h_locals);
    darray_free(co->co_locals);
    darray_free(co->co_lnotab);

    if (co->co_bytecode)
        free(co->co_bytecode);

    free(co);
}