#include "co.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "../utils/hashmap.h"
#include "../utils/darray.h"
#include "../utils/utf8.h"

#include "objects/functionobject.h"
#include "objects/floatobject.h"
#include "objects/longobject.h"
#include "objects/noneobject.h"
#include "objects/strobject.h"

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

static uint16_t co_add_literal(MECodeObject* co, LiteralExpr* literal) {
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
    
    uint16_t idx = darray_size(co->co_consts);
    darray_pushd(co->co_consts, obj);
    return idx;
}

static void co_compile_expr(MECodeObject* co, Expr* expr) {
    if (!expr)
        return;

    uintptr_t idx = 0;
    switch (expr->kind) {
        case EXPR_LITERAL: {
            idx = co_add_literal(co, expr->literal);
            co_bc_opoperand(co, CO_OP_LOAD_CONST, idx, 2);
            lnotab_forward(co, 3, expr->line);
            break;
        }
        case EXPR_VARIABLE: {
            StringView var_name = expr->variable->name;
            
            if (!co->in_function) {
                if (hashmap_get(co->co_h_globals, expr->variable->name.data, expr->variable->name.byte_len, NULL)) {
                    hashmap_get(co->co_h_globals, expr->variable->name.data, expr->variable->name.byte_len, (uintptr_t*)&idx);
                    co_bc_opoperand(co, CO_OP_LOAD_GLOBAL, idx, 2);
                } else if (hashmap_get(co->co_h_locals, expr->variable->name.data, expr->variable->name.byte_len, NULL)) {
                    hashmap_get(co->co_h_locals, expr->variable->name.data, expr->variable->name.byte_len, (uintptr_t*)&idx);
                    co_bc_opoperand(co, CO_OP_LOAD_VARIABLE, idx, 2);
                }
            } else {
                if (hashmap_get(co->co_h_locals, expr->variable->name.data, expr->variable->name.byte_len, NULL)) {
                    hashmap_get(co->co_h_locals, expr->variable->name.data, expr->variable->name.byte_len, (uintptr_t*)&idx);
                    co_bc_opoperand(co, CO_OP_LOAD_VARIABLE, idx, 2);
                } else if (hashmap_get(co->co_h_globals, expr->variable->name.data, expr->variable->name.byte_len, NULL)) {
                    hashmap_get(co->co_h_globals, expr->variable->name.data, expr->variable->name.byte_len, (uintptr_t*)&idx);
                    co_bc_opoperand(co, CO_OP_LOAD_GLOBAL, idx, 2);
                }
            }
            lnotab_forward(co, 3, expr->line);
            break;
        }
        case EXPR_BINARY: {
                co_compile_expr(co, expr->binary->lhs);
                co_compile_expr(co, expr->binary->rhs);
                
                co_bc_opoperand(co, CO_OP_BINARY_OP, expr->binary->op, 1);
                lnotab_forward(co, 2, expr->line);
                
                if (expr->binary->op == BIN_ASSIGN) {
                    if (hashmap_get(co->co_h_locals, expr->binary->lhs->variable->name.data, expr->binary->lhs->variable->name.byte_len, NULL)) {
                        hashmap_get(co->co_h_locals, expr->binary->lhs->variable->name.data, expr->binary->lhs->variable->name.byte_len, (uintptr_t*)&idx);
                        co_bc_opoperand(co, CO_OP_STORE_VARIABLE, idx, 2);
                    } else if (hashmap_get(co->co_h_globals, expr->binary->lhs->variable->name.data, expr->binary->lhs->variable->name.byte_len, NULL)) {
                        hashmap_get(co->co_h_globals, expr->binary->lhs->variable->name.data, expr->binary->lhs->variable->name.byte_len, (uintptr_t*)&idx);
                        co_bc_opoperand(co, CO_OP_STORE_GLOBAL, idx, 2);
                    }
                }

            break;
        }
        case EXPR_UNARY: {
            co_compile_expr(co, expr->unary->operand);
            
            co_bc_opoperand(co, CO_OP_UNARY_OP, expr->unary->op, 1);
            lnotab_forward(co, 2, expr->line);
            break;
        }
        case EXPR_CALL: {
            if (hashmap_get(co->co_h_locals, expr->call->name.data, expr->call->name.byte_len, NULL)) {
                hashmap_get(co->co_h_locals, expr->call->name.data, expr->call->name.byte_len, (uintptr_t*)&idx);
                co_bc_opoperand(co, CO_OP_LOAD_VARIABLE, idx, 2);
            } else if (hashmap_get(co->co_h_globals, expr->call->name.data, expr->call->name.byte_len, NULL)) {
                hashmap_get(co->co_h_globals, expr->call->name.data, expr->call->name.byte_len, (uintptr_t*)&idx);
                co_bc_opoperand(co, CO_OP_LOAD_GLOBAL, idx, 2);
            }

            lnotab_forward(co, 3, expr->line);

            for (int i = darray_size(expr->call->args) - 1; i >= 0; i--)
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
    
    uintptr_t idx = 0;

    switch (stmt->kind) {
        case STMT_EXPR:
            co_compile_expr(co, stmt->expr_stmt);
            if (stmt->expr_stmt->binary->op != BIN_ASSIGN)
                co_bc_op(co, CO_OP_POP);

            lnotab_forward(co, 1, stmt->line);
            break;
        case STMT_DECL: {
            if (stmt->decl_stmt->initializer)
                co_compile_expr(co, stmt->decl_stmt->initializer);
            else
                co_bc_opoperand(co, CO_OP_LOAD_CONST, 0, 2);

            if (!co->in_function) {
                if (hashmap_get(co->co_h_globals, stmt->decl_stmt->name.data, stmt->decl_stmt->name.byte_len, NULL)) {
                    hashmap_get(co->co_h_globals, stmt->decl_stmt->name.data, stmt->decl_stmt->name.byte_len, (uintptr_t*)&idx);
                    co_bc_opoperand(co, CO_OP_STORE_GLOBAL, idx, 2);
                } else {
                    idx = hashmap_size(co->co_h_globals);
                    hashmap_set(co->co_h_globals, stmt->decl_stmt->name.data, stmt->decl_stmt->name.byte_len, (uintptr_t)idx);
                    co_bc_opoperand(co, CO_OP_STORE_GLOBAL, idx, 2);
                }
            } else {
                if (hashmap_get(co->co_h_locals, stmt->decl_stmt->name.data, stmt->decl_stmt->name.byte_len, NULL)) {
                    hashmap_get(co->co_h_locals, stmt->decl_stmt->name.data, stmt->decl_stmt->name.byte_len, (uintptr_t*)&idx);
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
            co_compile_expr(co, stmt->if_stmt->condition);
            
            // Save position for the conditional jump
            uint16_t then_branch_start = co->co_size;
            co_bc_opoperand(co, CO_OP_JUMP_IF_FALSE, 0, 2);
            lnotab_forward(co, 3, stmt->line);

            // Compile the "then" branch
            for (size_t i = 0; i < darray_size(stmt->if_stmt->then_branch); i++) {
                co_compile_stmt(co, stmt->if_stmt->then_branch[i]);
            }

            uint16_t else_branch_start = 0;
            if (stmt->if_stmt->else_branch) {
                // If there's an else branch, add a jump to skip it after "then"
                else_branch_start = co->co_size;
                co_bc_opoperand(co, CO_OP_JUMP_REL, 0, 2);
                lnotab_forward(co, 3, stmt->line);
            }

            uint16_t else_pos = co->co_size;
            uint16_t if_offset = else_pos - then_branch_start - 3;
            memcpy(&co->co_bytecode[then_branch_start + 1], &if_offset, 2);

            if (stmt->if_stmt->else_branch) {
                co_compile_stmt(co, stmt->if_stmt->else_branch);
                
                uint16_t end_pos = co->co_size;
                uint16_t else_offset = end_pos - else_branch_start - 3;
                memcpy(&co->co_bytecode[else_branch_start + 1], &else_offset, 2);
            }

            break;
        }
        case STMT_FUNCTION_DECL: {
            MECodeObject* func_co = malloc(sizeof(MECodeObject));
            func_co->co_name = malloc(stmt->function_decl->name.byte_len + 1);
            memcpy(func_co->co_name, stmt->function_decl->name.data, stmt->function_decl->name.byte_len);
            func_co->co_name[stmt->function_decl->name.byte_len] = '\0';
            func_co->co_h_globals = co->co_h_globals;
            func_co->co_h_locals = hashmap_new();
            func_co->co_consts = darray_new(MEObject*);
            func_co->co_globals = darray_new(MEObject*);
            func_co->co_locals = darray_new(MEObject*);
            darray_pushd(func_co->co_consts, me_none);
            darray_pushd(func_co->co_locals, me_none);
            func_co->co_lnotab = darray_new(uint8_t);
            func_co->co_capacity = 128;
            func_co->co_bytecode = (uint8_t*)malloc(func_co->co_capacity);
            memset(func_co->co_bytecode, 0, func_co->co_capacity);
            func_co->co_size = 0;
            func_co->in_function = 1;
            
            for (size_t i = 0; i < darray_size(stmt->function_decl->params); i++) {
                Expr* param = stmt->function_decl->params[i];
                if (param->kind == EXPR_VARIABLE) {
                    uint16_t param_idx = hashmap_size(func_co->co_h_locals);
                    hashmap_set(func_co->co_h_locals, param->variable->name.data, param->variable->name.byte_len, (uintptr_t)param_idx);
                }
            }
            
            for (size_t i = 0; i < darray_size(stmt->function_decl->body); i++)
                co_compile_stmt(func_co, stmt->function_decl->body[i]);
            
            // RETURN NONE ALWAYS IF THERE IS NO RETURN STMT
            if (func_co->co_size == 0 || func_co->co_bytecode[func_co->co_size - 1] != CO_OP_RETURN) {
                co_bc_opoperand(func_co, CO_OP_LOAD_CONST, 0, 2);
                co_bc_op(func_co, CO_OP_RETURN);
                lnotab_forward(func_co, 4, stmt->line);
            }
            
            MEObject* func_obj = me_function_new(func_co, darray_size(stmt->function_decl->params));
            uint16_t func_idx = darray_size(co->co_consts);
            darray_pushd(co->co_consts, func_obj);
            
            co_bc_opoperand(co, CO_OP_LOAD_CONST, func_idx, 2);
            lnotab_forward(co, 3, stmt->line);
            // co_bc_opoperand(co, CO_OP_MAKE_FUNCTION, darray_size(stmt->function_decl->params), 1);
            // lnotab_forward(co, 2, stmt->line);
            
            // ADD FUNCTION TO ITS LOCALS FOR RECURSIVE CALLS
            uintptr_t name_idx;
            if (!co->in_function) {
                if (hashmap_get(co->co_h_globals, stmt->function_decl->name.data, stmt->function_decl->name.byte_len, NULL)) {
                    hashmap_get(co->co_h_globals, stmt->function_decl->name.data, stmt->function_decl->name.byte_len, (uintptr_t*)&name_idx);
                } else {
                    name_idx = hashmap_size(co->co_h_globals);
                    hashmap_set(co->co_h_globals, stmt->function_decl->name.data, stmt->function_decl->name.byte_len, (uintptr_t)name_idx);
                }
                co_bc_opoperand(co, CO_OP_STORE_GLOBAL, name_idx, 2);
            } else { // This is not allowed and code should never reach here
                if (hashmap_get(co->co_h_locals, stmt->function_decl->name.data, stmt->function_decl->name.byte_len, NULL)) {
                    hashmap_get(co->co_h_locals, stmt->function_decl->name.data, stmt->function_decl->name.byte_len, (uintptr_t*)&name_idx);
                } else {
                    name_idx = hashmap_size(co->co_h_locals);
                    hashmap_set(co->co_h_locals, stmt->function_decl->name.data, stmt->function_decl->name.byte_len, (uintptr_t)name_idx);
                }
                co_bc_opoperand(co, CO_OP_STORE_VARIABLE, name_idx, 2);
            }
            lnotab_forward(co, 3, stmt->line);
            
            break;
        }
        case STMT_WHILE: {
            uint32_t loop_start = co->co_size;

            co_compile_expr(co, stmt->while_stmt->condition);
            
            uint32_t jump_out_pos = co->co_size;
            co_bc_opoperand(co, CO_OP_JUMP_IF_FALSE, 0, 2); // Placeholder
            lnotab_forward(co, 3, stmt->line);
            
            int old_loop_start = co->loop_start;
            int old_loop_end_jump = co->loop_end_jump;
            int old_loop_end_pos = co->loop_end_pos;

            co->loop_start = loop_start;
            co->loop_end_jump = jump_out_pos;
            
            for (size_t i = 0; i < darray_size(stmt->while_stmt->body); i++)
                co_compile_stmt(co, stmt->while_stmt->body[i]);

            co_bc_opoperand(co, CO_OP_JUMP_REL, loop_start - co->co_size - 3, 2);
            lnotab_forward(co, 3, stmt->line);
            
            co->loop_end_pos = co->co_size;

            uint16_t jump_out_offset = co->loop_end_pos - jump_out_pos - 3;
            memcpy(&co->co_bytecode[jump_out_pos + 1], &jump_out_offset, 2);

            for (size_t i = 0; i < darray_size(co->break_patches); i++) {
                uint32_t break_pos = co->break_patches[i];
                int16_t break_offset = co->loop_end_pos - break_pos - 3;
                memcpy(&co->co_bytecode[break_pos + 1], &break_offset, 2);
            }
            
            // printf("Loop start: %u, end jump: %u, end pos: %u\n", co->loop_start, co->loop_end_jump, co->loop_end_pos);
            co->loop_start = old_loop_start;
            co->loop_end_jump = old_loop_end_jump;
            co->loop_end_pos = old_loop_end_pos;

            
            break;
        }
        case STMT_BREAK: {            
            darray_push(co->break_patches, co->co_size);
            co_bc_opoperand(co, CO_OP_JUMP_REL, 0xFFFF, 2); // 0 is not valid, if there is a loop in if statement it will be problematic
            lnotab_forward(co, 3, stmt->line);
            break;
        }
        case STMT_CONTINUE: {
            co_bc_opoperand(co, CO_OP_JUMP_REL, co->loop_start - co->co_size - 3, 2);
            lnotab_forward(co, 3, stmt->line);
            break;
        }
        default:
            break;
    }
}

void co_disasm(MECodeObject* co) {
    if (!co)
        return;

    uint32_t ip = 0;
    while (ip < co->co_size)
    {
        uint8_t op = co->co_bytecode[ip];
        printf("%04u: ", ip);
        
        switch (op) {
            case CO_OP_LOAD_CONST: {
                printf("LOAD_CONST ");
                uint16_t idx = *(uint16_t*)(co->co_bytecode + ip + 1);
                printf("%u\n", idx);
                ip += 2;
                break;
            }
            case CO_OP_LOAD_VARIABLE: {
                printf("LOAD_VARIABLE ");
                uint16_t idx = *(uint16_t*)(co->co_bytecode + ip + 1);
                printf("%u\n", idx);
                ip += 2;
                break;
            }
            case CO_OP_STORE_VARIABLE: {
                printf("STORE_VARIABLE ");
                uint16_t idx = *(uint16_t*)(co->co_bytecode + ip + 1);
                printf("%u\n", idx);
                ip += 2;
                break;
            }
            case CO_OP_STORE_GLOBAL: {
                printf("STORE_GLOBAL ");
                uint16_t idx = *(uint16_t*)(co->co_bytecode + ip + 1);
                printf("%u\n", idx);
                ip += 2;
                break;
            }
            case CO_OP_LOAD_GLOBAL: {
                printf("LOAD_GLOBAL ");
                uint16_t idx = *(uint16_t*)(co->co_bytecode + ip + 1);
                printf("%u\n", idx);
                ip += 2;
                break;
            }
            case CO_OP_CALL_FUNCTION:
                printf("CALL_FUNCTION ");
                uint8_t arg_count = co->co_bytecode[ip + 1];
                printf("%u\n", arg_count);
                ip++;
                break;
            case CO_OP_RETURN:
                printf("RETURN\n");
                break;
            case CO_OP_JUMP_IF_FALSE:
                printf("JUMP_IF_FALSE ");
                uint16_t jump_if_false_offset = *(uint16_t*)(co->co_bytecode + ip + 1);
                printf("%u\n", jump_if_false_offset);
                ip += 2;
                break;
            case CO_OP_JUMP_REL:
                printf("JUMP_REL ");
                int16_t jump_offset = *(int16_t*)(co->co_bytecode + ip + 1);
                printf("%d\n", jump_offset);
                ip += 2;
                break;
            case CO_OP_UNARY_OP:
                printf("UNARY_OP ");
                uint8_t unary_op = co->co_bytecode[ip + 1];
                printf("%u\n", unary_op);
                ip++;
                break;
            case CO_OP_BINARY_OP:
                printf("BINARY_OP ");
                uint8_t binary_op = co->co_bytecode[ip + 1];
                printf("%u\n", binary_op);
                ip++;
                break;
            case CO_OP_POP:
                printf("POP\n");
                break;
            default:
                printf("UNKNOWN OP %u\n", op);
                break;
        }

        ip += 1;
    }

    for (size_t i = 0; i < darray_size(co->co_consts); i++) {
        if (!me_function_check(co->co_consts[i]))
            continue;

        MEFunctionObject* func = (MEFunctionObject*)co->co_consts[i];
        printf("--------------------\n");
        printf("Function: %.*s, nargs: %zu\n", (int)utf8_strsize(func->co->co_name), func->co->co_name, func->nargs);
        printf("Bytecode:\n");
        co_disasm(func->co);
        printf("--------------------\n");
    }
}

MECodeObject* co_new(const char* filename, Stmt** stmts) {
    MECodeObject* co = malloc(sizeof(MECodeObject));
    size_t filename_size = utf8_strsize(filename) + 1; 
    co->co_name = malloc(filename_size);
    memcpy(co->co_name, filename, filename_size);
    
    co->co_h_globals = hashmap_new();
    co->co_h_locals = hashmap_new();
    co->co_consts = darray_new(MEObject*);
    co->co_globals = darray_new(MEObject*);
    co->co_locals = darray_new(MEObject*);
    darray_pushd(co->co_consts, me_none);
    co->co_lnotab = darray_new(uint8_t);
    co->co_capacity = ME_CO_INITIAL_CAPACITY;
    co->co_bytecode = (uint8_t*)malloc(co->co_capacity);
    memset(co->co_bytecode, 0, co->co_capacity);
    co->co_size = 0;
    co->in_function = 0;
    co->loop_start = 0;
    co->loop_end_jump = 0;
    co->loop_end_pos = 0;
    co->break_patches = darray_new(uint32_t);

    for (size_t i = 0; i < darray_size(stmts); i++)
        co_compile_stmt(co, stmts[i]);

    return co;
}

void co_free(MECodeObject* co) {
    if (!co)
        return;

    free(co->co_name);

    hashmap_free(co->co_h_globals);
    hashmap_free(co->co_h_locals);
    darray_free(co->co_consts);
    darray_free(co->co_globals);
    darray_free(co->co_locals);
    darray_free(co->co_lnotab);

    if (co->co_bytecode)
        free(co->co_bytecode);

    if (co->break_patches)
        darray_free(co->break_patches);

    free(co);
}