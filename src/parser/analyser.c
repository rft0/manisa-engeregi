#include "analyser.h"

#include <stdlib.h>
#include <stdio.h>

#include "../utils/hashmap.h"
#include "../utils/darray.h"
#include "../utils/str.h"

#include "../diag/diag.h"
#include "stmt.h"

typedef struct Symbol {
    StringView name;
    int is_const;
    int is_initialized;
    int line;
    int col;
} Symbol;

typedef struct Method {
    StringView name;
    int param_count;
    int line;
    int col;
} Method;

typedef struct Scope {
    struct Scope* parent;
    HashMap* symbols;
    int depth;
} Scope;

typedef struct Analyser {
    const char* filename;
    Stmt** stmts;
    int index;

    Scope* current_scope;
    int inside_loop;
    int inside_function;
} Analyser;

// Forward decls
static void analyser_init(Analyser* analyser, const char* filename, Stmt** stmts);

static void analyse_stmt(Analyser* analyser, Stmt* stmt);
static void analyse_expr(Analyser* analyser, Expr* expr, int check_init);

static void analyse_compound(Analyser* analyser, Stmt* stmt);
static void analyse_decl(Analyser* analyser, Stmt* stmt);
static void analyse_expr_stmt(Analyser* analyser, Stmt* stmt);
static void analyse_if(Analyser* analyser, Stmt* stmt);
static void analyse_while(Analyser* analyser, Stmt* stmt);
static void analyse_function_decl(Analyser* analyser, Stmt* stmt);
static void analyse_return(Analyser* analyser, Stmt* stmt);
static void analyse_break_continue(Analyser* analyser, Stmt* stmt);

static Scope* scope_new(Scope* parent);
static void scope_free(Scope* scope);
static void scope_enter(Analyser* ctx);
static void scope_exit(Analyser* ctx);

static Symbol* symbol_new(StringView name, int is_const, int line, int col);
static void symbol_free(Symbol* symbol);
static Symbol* scope_lookup(Scope* scope, StringView name);
static Symbol* scope_lookup_current(Scope* scope, StringView name);
static int scope_define(Scope* scope, Symbol* symbol);

static void analyser_init(Analyser* analyser, const char* filename, Stmt** stmts) {
    analyser->filename = filename;
    analyser->stmts = stmts;
    analyser->index = 0;
    analyser->current_scope = NULL;
    analyser->inside_loop = 0;
    analyser->inside_function = 0;
    
    scope_enter(analyser);
}

static Scope* scope_new(Scope* parent) {
    Scope* scope = malloc(sizeof(Scope));
    scope->parent = parent;
    scope->symbols = hashmap_new();
    scope->depth = parent ? parent->depth + 1 : 0;

    return scope;
}

void symbol_free_iter(const void* key, size_t key_len, uintptr_t value, void* user_data) {
    Symbol* symbol = (Symbol*)value;
    symbol_free(symbol);
}

// Debug function remove this later me.
void symbol_print_iter(const void* key, size_t key_len, uintptr_t value, void* user_data) {
    Symbol* symbol = (Symbol*)value;
    printf("Symbol: %.*s (const: %d, initialized: %d, line: %d, col: %d)\n",
        (int)key_len, (const char*)key, symbol->is_const, symbol->is_initialized,
        symbol->line, symbol->col);
}

static void scope_free(Scope* scope) {
    if (!scope) return;
    
    hashmap_iterate(scope->symbols, symbol_free_iter, NULL);

    hashmap_free(scope->symbols);
    free(scope);
}

static void scope_enter(Analyser* ctx) {
    ctx->current_scope = scope_new(ctx->current_scope);
}

static void scope_exit(Analyser* ctx) {
    Scope* old = ctx->current_scope;
    ctx->current_scope = old->parent;
    scope_free(old);
}

static Symbol* symbol_new(StringView name, int is_const, int line, int col) {
    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->name = name;
    symbol->is_const = is_const;
    symbol->is_initialized = 0;
    symbol->line = line;
    symbol->col = col;
    return symbol;
}

static void symbol_free(Symbol* symbol) {
    free(symbol);
}

static Symbol* scope_lookup_current(Scope* scope, StringView name) {
    if (!scope) return NULL;
    
    uintptr_t symbol_ptr;
    if (hashmap_get(scope->symbols, name.data, name.byte_len, &symbol_ptr))
        return (Symbol*)symbol_ptr;
    
    return NULL;
}

static Symbol* scope_lookup(Scope* scope, StringView name) {
    if (!scope)
        return NULL;
    
    Symbol* symbol = scope_lookup_current(scope, name);
    if (symbol)
        return symbol;
    
    return scope_lookup(scope->parent, name);
}

static int scope_define(Scope* scope, Symbol* symbol) {
    if (scope_lookup_current(scope, symbol->name))
        return 0;
    
    return hashmap_set(scope->symbols, symbol->name.data, symbol->name.byte_len, (uintptr_t)symbol);
}

static void analyse_compound(Analyser* analyser, Stmt* stmt) {
    scope_enter(analyser);
    for (int i = 0; i < darray_size(stmt->compound->stmts); i++) {
        analyse_stmt(analyser, stmt->compound->stmts[i]);
    }
    scope_exit(analyser);
}

static void analyse_decl(Analyser* analyser, Stmt* stmt) {
    if (stmt->decl_stmt->initializer)
        analyse_expr(analyser, stmt->decl_stmt->initializer, 1);

    Symbol* symbol = symbol_new(stmt->decl_stmt->name, stmt->decl_stmt->is_const, stmt->line, stmt->col);
    symbol->is_initialized = stmt->decl_stmt->initializer != NULL;

    if (!scope_define(analyser->current_scope, symbol)) {
        diags_new_diag(DIAG_SEMANTIC, DIAG_ERROR, analyser->filename, 
            stmt->line, stmt->col, 
            "Variable '%.*s' already defined in this scope", 
            (int)stmt->decl_stmt->name.byte_len, stmt->decl_stmt->name.data);
        symbol_free(symbol);
    }
}

static void analyse_expr_stmt(Analyser* analyser, Stmt* stmt) {
    analyse_expr(analyser, stmt->expr_stmt, 0);
}

static void analyse_if(Analyser* analyser, Stmt* stmt) {
    analyse_expr(analyser, stmt->if_stmt->condition, 1);
    
    for (int i = 0; i < darray_size(stmt->if_stmt->then_branch); i++)
        analyse_stmt(analyser, stmt->if_stmt->then_branch[i]);
    
    if (stmt->if_stmt->else_branch)
        analyse_stmt(analyser, stmt->if_stmt->else_branch);
}

static void analyse_while(Analyser* analyser, Stmt* stmt) {
    analyse_expr(analyser, stmt->while_stmt->condition, 1);
    
    analyser->inside_loop++;

    for (int i = 0; i < darray_size(stmt->while_stmt->body); i++)
        analyse_stmt(analyser, stmt->while_stmt->body[i]);

    analyser->inside_loop--;
}

static void analyse_function_decl(Analyser* analyser, Stmt* stmt) {
    scope_enter(analyser);
    analyser->inside_function++;

    if (analyser->inside_function > 1)
        diags_new_diag(DIAG_SEMANTIC, DIAG_ERROR, analyser->filename, stmt->line, stmt->col, "Nested method declarations are not allowed");

    for (int i = 0; i < darray_size(stmt->function_decl->params); i++) {
        Expr* param = stmt->function_decl->params[i];
        if (param->kind == EXPR_VARIABLE) {
            Symbol* symbol = symbol_new(param->variable->name, 0, param->line, param->col);
            symbol->is_initialized = 1; // Parameters are always initialized

            if (!scope_define(analyser->current_scope, symbol)) {
                diags_new_diag(DIAG_SEMANTIC, DIAG_ERROR, analyser->filename, 
                    param->line, param->col,
                    "Parameter '%.*s' already defined", 
                    (int)param->variable->name.byte_len, param->variable->name.data);
                symbol_free(symbol);
            }
        }
    }

    for (int i = 0; i < darray_size(stmt->function_decl->body); i++)
        analyse_stmt(analyser, stmt->function_decl->body[i]);

    
    analyser->inside_function--;
    scope_exit(analyser);
}

static void analyse_return(Analyser* analyser, Stmt* stmt) {
    if (!analyser->inside_function) {
        diags_new_diag(DIAG_SEMANTIC, DIAG_ERROR, analyser->filename, 
            stmt->line, stmt->col, "Return statement outside of method");
    }

    if (stmt->return_stmt->value) {
        analyse_expr(analyser, stmt->return_stmt->value, 1);
    }
}

static void analyse_break_continue(Analyser* analyser, Stmt* stmt) {
    if (!analyser->inside_loop) {
        diags_new_diag(DIAG_SEMANTIC, DIAG_ERROR, analyser->filename, 
            stmt->line, stmt->col, 
            stmt->kind == STMT_BREAK ? "Break outside of loop" : "Continue outside of loop");
    }
}

static void analyse_stmt(Analyser* analyser, Stmt* stmt) {
    if (!stmt)
        return;

    switch (stmt->kind) {
        case STMT_COMPOUND:
            analyse_compound(analyser, stmt);
            break;
        case STMT_DECL:
            analyse_decl(analyser, stmt);
            break;
        case STMT_EXPR:
            analyse_expr_stmt(analyser, stmt);
            break;
        case STMT_IF:
            analyse_if(analyser, stmt);
            break;
        case STMT_WHILE:
            analyse_while(analyser, stmt);
            break;
        case STMT_FUNCTION_DECL:
            analyse_function_decl(analyser, stmt);
            break;
        case STMT_RETURN:
            analyse_return(analyser, stmt);
            break;
        case STMT_BREAK:
        case STMT_CONTINUE:
            analyse_break_continue(analyser, stmt);
            break;
    }
}

static void analyse_expr(Analyser* analyser, Expr* expr, int check_init) {
    if (!expr) return;
    
    switch (expr->kind) {
        case EXPR_VARIABLE: {
            Symbol* symbol = scope_lookup(analyser->current_scope, expr->variable->name);
            if (!symbol) {
                diags_new_diag(DIAG_SEMANTIC, DIAG_ERROR, analyser->filename, 
                    expr->line, expr->col,
                    "Undefined variable '%.*s'", 
                    (int)expr->variable->name.byte_len, expr->variable->name.data);
            } else if (check_init && !symbol->is_initialized) {
                diags_new_diag(DIAG_SEMANTIC, DIAG_ERROR, analyser->filename, 
                    expr->line, expr->col,
                    "Variable '%.*s' used before initialization", 
                    (int)expr->variable->name.byte_len, expr->variable->name.data);
            }
            break;
        }
        
        case EXPR_CALL: {
            darray_for(expr->call->args) {
                analyse_expr(analyser, expr->call->args[__i], 1);
            }
            break;
        }
        
        case EXPR_BINARY: {
            analyse_expr(analyser, expr->binary->lhs, 1);
            analyse_expr(analyser, expr->binary->rhs, 1);
            
            if (expr->binary->op == BIN_ASSIGN) {
                if (expr->binary->lhs->kind == EXPR_VARIABLE) {
                    Symbol* symbol = scope_lookup(analyser->current_scope, expr->binary->lhs->variable->name);
                    if (symbol) {
                        if (symbol->is_const) {
                            diags_new_diag(DIAG_SEMANTIC, DIAG_ERROR, analyser->filename, 
                                expr->line, expr->col,
                                "Cannot assign to const variable '%.*s'", 
                                (int)symbol->name.byte_len, symbol->name.data);
                        }
                        symbol->is_initialized = 1;
                    }
                }
            }
            break;
        }
        
        case EXPR_UNARY:
            analyse_expr(analyser, expr->unary->operand, 1);
            break;
        case EXPR_LITERAL:
            break;
    }
}

void analyse(const char* filename, Stmt** stmts) {
    Analyser analyser;
    analyser_init(&analyser, filename, stmts);
    
    scope_enter(&analyser);
    
    darray_for(stmts) analyse_stmt(&analyser, stmts[__i]);
    
    scope_exit(&analyser);
}