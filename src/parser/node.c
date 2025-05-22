#include "node.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../utils/darray.h"

Stmt* stmt_new(StmtKind kind, int line, int col) {
    Stmt* s = malloc(sizeof(Stmt));
    if (!s)
        return NULL;

    s->kind = kind;
    s->line = line;
    s->col = col;

    return s;
}

Stmt* stmt_new_compound(Stmt** stmts, int line, int col) {
    Stmt* s = stmt_new(STMT_COMPOUND, line, col);
    if (!s)
        return NULL;

    s->compound = malloc(sizeof(CompoundStmt));
    if (!s->compound) {
        free(s);
        return NULL;
    }

    s->compound->stmts = stmts;

    return s;
}

Stmt* stmt_new_decl(StringView name, Expr* initializer, int is_const, int line, int col) {
    Stmt* s = stmt_new(STMT_DECL, line, col);
    if (!s)
        return NULL;

    s->decl_stmt = malloc(sizeof(DeclStmt));
    if (!s->decl_stmt) {
        free(s);
        return NULL;
    }

    s->decl_stmt->name = name;
    s->decl_stmt->initializer = initializer;
    s->decl_stmt->is_const = is_const;

    return s;
}

Stmt* stmt_new_expr(Expr* expr, int line, int col) {
    Stmt* s = stmt_new(STMT_EXPR, line, col);
    if (!s)
        return NULL;

    s->expr_stmt = expr;

    return s;
}

Stmt* stmt_new_while(Expr* condition, Stmt** body, int line, int col) {
    Stmt* s = stmt_new(STMT_WHILE, line, col);
    if (!s)
        return NULL;

    s->while_stmt = malloc(sizeof(WhileStmt));
    if (!s->while_stmt) {
        free(s);
        return NULL;
    }

    s->while_stmt->condition = condition;
    s->while_stmt->body = body;

    return s;
}

Stmt* stmt_new_if(Expr* condition, Stmt** then_branch, Stmt* else_branch, int line, int col) {
    Stmt* s = stmt_new(STMT_IF, line, col);
    if (!s)
        return NULL;

    s->if_stmt = malloc(sizeof(IfStmt));
    if (!s->if_stmt) {
        free(s);
        return NULL;
    }

    s->if_stmt->condition = condition;
    s->if_stmt->then_branch = then_branch;
    s->if_stmt->else_branch = else_branch;

    return s;
}

Stmt* stmt_new_method_decl(StringView name, Expr** params, Stmt** body, int line, int col) {
    Stmt* s = stmt_new(STMT_METHOD_DECL, line, col);
    if (!s)
        return NULL;

    s->method_decl = malloc(sizeof(MethodDeclStmt));
    if (!s->method_decl) {
        free(s);
        return NULL;
    }

    s->method_decl->name = name;
    s->method_decl->params = params;
    s->method_decl->body = body;

    return s;
}


Stmt* stmt_new_return(Expr* value, int line, int col) {
    Stmt* s = stmt_new(STMT_RETURN, line, col);
    if (!s)
        return NULL;

    s->return_stmt = malloc(sizeof(ReturnStmt));
    if (!s->return_stmt) {
        free(s);
        return NULL;
    }

    s->return_stmt->value = value;

    return s;
}

Stmt* stmt_new_break(int line, int col) {
    Stmt* s = stmt_new(STMT_BREAK, line, col);
    if (!s)
        return NULL;

    s->dummy = NULL;

    return s;
}

Stmt* stmt_new_continue(int line, int col) {
    Stmt* s = stmt_new(STMT_CONTINUE, line, col);
    if (!s)
        return NULL;

    s->dummy = NULL;

    return s;
}

void stmt_free(Stmt* s) {
    if (!s)
        return;

    switch (s->kind) {
        case STMT_COMPOUND:
            darray_for(s->compound->stmts) stmt_free(s->compound->stmts[__i]);
            darray_free(s->compound->stmts);
            free(s->compound);
            break;
        case STMT_DECL:
            expr_free(s->decl_stmt->initializer);
            free(s->decl_stmt);
            break;
        case STMT_EXPR:
            expr_free(s->expr_stmt);
            break;
        case STMT_WHILE:
            expr_free(s->while_stmt->condition);
            darray_for(s->while_stmt->body) stmt_free(s->while_stmt->body[__i]);
            darray_free(s->while_stmt->body);
            free(s->while_stmt);
            break;
        case STMT_IF:
            expr_free(s->if_stmt->condition);
            darray_for(s->if_stmt->then_branch) stmt_free(s->if_stmt->then_branch[__i]);
            darray_free(s->if_stmt->then_branch);
            stmt_free(s->if_stmt->else_branch);
            free(s->if_stmt);
            break;
        case STMT_METHOD_DECL:
            darray_for(s->method_decl->params) expr_free(s->method_decl->params[__i]);
            darray_free(s->method_decl->params);
            darray_for(s->method_decl->body) stmt_free(s->method_decl->body[__i]);
            darray_free(s->method_decl->body);
            free(s->method_decl);
            break;
        case STMT_RETURN:
            expr_free(s->return_stmt->value);
            free(s->return_stmt);
            break;
        case STMT_BREAK:
        case STMT_CONTINUE:
            break;
    }

    free(s);
}

void stmt_dump(Stmt* s) {
    if (!s)
        return;

    switch (s->kind) {
        case STMT_COMPOUND:
            printf("Compound([");
            for (size_t i = 0; i < darray_size(s->compound->stmts); ++i) {
                stmt_dump(s->compound->stmts[i]);
                if (i < darray_size(s->compound->stmts) - 1)
                    printf(", ");
            }
            printf("])\n");
            break;
        case STMT_DECL:
            printf("Declaration(%.*s, %s, ", (int)s->decl_stmt->name.len, s->decl_stmt->name.data, s->decl_stmt->is_const ? "const" : "var");
            expr_dump(s->decl_stmt->initializer);
            printf(")\n");
            break;
        case STMT_EXPR:
            printf("Expression(");
            expr_dump(s->expr_stmt);
            printf(")\n");
            break;
        case STMT_WHILE:
            printf("While(");
            expr_dump(s->while_stmt->condition);
            printf(", [");
            for (size_t i = 0; i < darray_size(s->while_stmt->body); ++i) {
                stmt_dump(s->while_stmt->body[i]);
                if (i < darray_size(s->while_stmt->body) - 1)
                    printf(", ");
            }
            printf("])\n");
            break;
        case STMT_IF:
            printf("If(");
            expr_dump(s->if_stmt->condition);
            printf(", [");
            for (size_t i = 0; i < darray_size(s->if_stmt->then_branch); ++i) {
                stmt_dump(s->if_stmt->then_branch[i]);
                if (i < darray_size(s->if_stmt->then_branch) - 1)
                    printf(", ");
            }
            printf("], ");
            stmt_dump(s->if_stmt->else_branch);
            printf(")\n");
            break;
        case STMT_METHOD_DECL:
            printf("MethodDecl(%.*s, [", (int)s->method_decl->name.len, s->method_decl->name.data);
            for (size_t i = 0; i < darray_size(s->method_decl->params); ++i) {
                expr_dump(s->method_decl->params[i]);
                if (i < darray_size(s->method_decl->params) - 1)
                    printf(", ");
            }
            printf("], [");
            for (size_t i = 0; i < darray_size(s->method_decl->body); ++i) {
                stmt_dump(s->method_decl->body[i]);
                if (i < darray_size(s->method_decl->body) - 1)
                    printf(", ");
            }
            printf("])\n");
            break;
        case STMT_RETURN:
            printf("Return(");
            expr_dump(s->return_stmt->value);
            printf(")\n");
            break;
        case STMT_BREAK:
            printf("Break()\n");
            break;
        case STMT_CONTINUE:
            printf("Continue()\n");
            break;
    }
}

Expr* expr_new(ExprKind kind, int line, int col) {
    Expr* e = malloc(sizeof(Expr));
    if (!e)
        return NULL;

    e->kind = kind;
    e->line = line;
    e->col = col;

    return e;
}

Expr* expr_new_literal(LiteralType type, StringView value, int line, int col) {
    Expr* e = expr_new(EXPR_LITERAL, line, col);
    if (!e)
        return NULL;

    e->literal = malloc(sizeof(LiteralExpr));
    if (!e->literal) {
        free(e);
        return NULL;
    }

    e->literal->type = type;
    e->literal->value = value;

    return e;
}

Expr* expr_new_variable(StringView name, int line, int col) {
    Expr* e = expr_new(EXPR_VARIABLE, line, col);
    if (!e)
        return NULL;

    e->variable = malloc(sizeof(VariableExpr));
    if (!e->variable) {
        free(e);
        return NULL;
    }

    e->variable->name = name;

    return e;
}

Expr* expr_new_unary(UnaryOp op, Expr* operand, int line, int col) {
    Expr* e = expr_new(EXPR_UNARY, line, col);
    if (!e)
        return NULL;

    e->unary = malloc(sizeof(UnaryExpr));
    if (!e->unary) {
        free(e);
        return NULL;
    }

    e->unary->op = op;
    e->unary->operand = operand;

    return e;
}

Expr* expr_new_binary(BinaryOp op, Expr* lhs, Expr* rhs, int line, int col) {
    Expr* e = expr_new(EXPR_BINARY, line, col);
    if (!e)
        return NULL;

    e->binary = malloc(sizeof(BinaryExpr));
    if (!e->binary) {
        free(e);
        return NULL;
    }

    e->binary->lhs = lhs;
    e->binary->rhs = rhs;
    e->binary->op = op;

    return e;
}

Expr* expr_new_call(StringView name, Expr** args, int line, int col) {
    Expr* e = expr_new(EXPR_CALL, line, col);
    if (!e)
        return NULL;

    e->call = malloc(sizeof(CallExpr));
    if (!e->call) {
        free(e);
        return NULL;
    }

    e->call->name = name;
    e->call->args = args;

    return e;
}

void expr_free(Expr* e) {
    if (!e)
        return;

    switch (e->kind) {
        case EXPR_LITERAL:
            free(e->literal);
            break;
        case EXPR_VARIABLE:
            free(e->variable);
            break;
        case EXPR_UNARY:
            expr_free(e->unary->operand);
            free(e->unary);
            break;
        case EXPR_BINARY:
            expr_free(e->binary->lhs);
            expr_free(e->binary->rhs);
            free(e->binary);
            break;
        case EXPR_CALL:
            darray_for(e->call->args) expr_free(e->call->args[__i]);
            darray_free(e->call->args);
            free(e->call);
            break;
    }

    free(e);
}

void expr_dump(Expr* e) {
    if (!e)
        return;

    switch (e->kind) {
        case EXPR_LITERAL:
            printf("Literal(%.*s)", (int)e->literal->value.len, e->literal->value.data);
            break;
        case EXPR_VARIABLE:
            printf("Variable(%.*s)", (int)e->variable->name.len, e->variable->name.data);
            break;
        case EXPR_UNARY:
            printf("Unary(%d, ", e->unary->op);
            expr_dump(e->unary->operand);
            printf(")");
            break;
        case EXPR_BINARY:
            printf("Binary(");
            expr_dump(e->binary->lhs);
            printf(", %d, ", e->binary->op);
            expr_dump(e->binary->rhs);
            printf(")");
            break;
        case EXPR_CALL:
            printf("Call(%.*s, [", (int)e->call->name.len, e->call->name.data);
            for (size_t i = 0; i < darray_size(e->call->args); ++i) {
                expr_dump(e->call->args[i]);
                if (i < darray_size(e->call->args) - 1)
                    printf(", ");
            }
            printf("])");
            break;
    }
}