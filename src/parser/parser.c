#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include "../utils/darray.h"

#include "../diag/diag.h"

typedef struct _Parser {
    const char* filename; 
    Token** tokens;
    Token* c;
    int index;
    jmp_buf loop_jmp;
} Parser;

// Forward decls
Expr* parse_expr(Parser* parser);
Expr* parse_assignment(Parser* parser);
Expr* parse_logical_or(Parser* parser);
Expr* parse_logical_and(Parser* parser);
Expr* parse_bitwise_or(Parser* parser);
Expr* parse_bitwise_xor(Parser* parser);
Expr* parse_bitwise_and(Parser* parser);
Expr* parse_equality(Parser* parser);
Expr* parse_comparison(Parser* parser);
Expr* parse_bitshift(Parser* parser);
Expr* parse_term(Parser* parser);
Expr* parse_factor(Parser* parser);
Expr* parse_unary(Parser* parser);
Expr* parse_postfix(Parser* parser);
Expr* parse_primary(Parser* parser);

Stmt* parse_stmt(Parser* parser);
Stmt* parse_if(Parser* parser);
Stmt* parse_while(Parser* parser);
Stmt* parse_while(Parser* parser);
Stmt* parse_decl(Parser* parser);
Stmt* parse_compound(Parser* parser);

// Helper functions
void parser_advance(Parser* parser) {
    parser->index++;
    parser->c = parser->tokens[parser->index];
}

Token* parser_peek(Parser* parser) {
    return parser->tokens[parser->index + 1];
}

int parser_match(Parser* parser, TokenType type) {
    if (parser->c && parser->c->type == type) {
        parser_advance(parser);
        return 1;
    }

    return 0;
}

int parser_check(Parser* parser, TokenType type) {
    if (!parser->c) return 0;
    return parser->c->type == type;
}

void parser_expect(Parser* parser, TokenType type, const char* msg) {
    if (parser->c && parser->c->type == type && parser->c->type != TOKEN_EOF) {
        parser_advance(parser);
    } else {
        diags_new_diag(DIAG_PARSER, DIAG_ERROR, parser->filename, parser->c->line, parser->c->col, msg);

        // We encountered an error at mid (possibly) of a statement so we must recover untill the end of the statement
        while (parser->c && parser->c->type != TOKEN_EOF) {
            if (parser->index > 0 && parser->tokens[parser->index - 1]->type == TOKEN_SEMI)
                longjmp(parser->loop_jmp, 1);
            
            switch (parser->c->type) {
                case TOKEN_KW_LET:
                case TOKEN_KW_CONST:
                case TOKEN_KW_IF:
                case TOKEN_KW_WHILE:
                case TOKEN_KW_RETURN:
                case TOKEN_LBRACE:
                    longjmp(parser->loop_jmp, 1);
                default:
                    break;
            }
            
            parser_advance(parser);
        }

        // Some says this is not a good practice who cares this is the easiest way to handle this job
        longjmp(parser->loop_jmp, 1);
    }
}

void parser_init(Parser* parser, const char* filename, Token** tokens) {
    parser->filename = filename;
    parser->tokens = tokens;
    parser->index = 0;
    parser->c = tokens[0];
}

// ----------------------------------
// parse_expr starts
// ----------------------------------

Expr* parse_expr(Parser* parser) {
    return parse_assignment(parser);
}

// Parse assignment expressions (lowest precedence)
Expr* parse_assignment(Parser* parser) {
    Expr* expr = parse_logical_or(parser);

    if (!expr)
        return NULL;
    
    //! TODO: Will be used LUT here
    if (parser_match(parser, TOKEN_ASSIGN)          ||
        parser_match(parser, TOKEN_ASSIGN_ADD)      ||
        parser_match(parser, TOKEN_ASSIGN_SUB)      ||
        parser_match(parser, TOKEN_ASSIGN_MUL)      ||
        parser_match(parser, TOKEN_ASSIGN_DIV)      ||
        parser_match(parser, TOKEN_ASSIGN_MOD)      ||
        parser_match(parser, TOKEN_ASSIGN_BIT_AND)  ||
        parser_match(parser, TOKEN_ASSIGN_BIT_OR)   ||
        parser_match(parser, TOKEN_ASSIGN_BIT_XOR)) {
        
        Token* op = parser->tokens[parser->index - 1];
        Expr* value = parse_assignment(parser);  // Right associative
        
        // Check that LHS is a valid assignment target (variable)
        if (expr->kind != EXPR_VARIABLE) {
            fprintf(stderr, "Invalid assignment target at line %d, col %d\n", expr->line, expr->col);
            exit(1);
        }
        
        // Convert assignment operators to corresponding binary operations
        BinaryOp binop;
        switch (op->type) {
            case TOKEN_ASSIGN:
                // Simple assignment handled separately
                return expr_new_binary(BIN_EQ, expr, value, expr->line, expr->col);
            case TOKEN_ASSIGN_ADD:
                binop = BIN_ADD;
                break;
            case TOKEN_ASSIGN_SUB:
                binop = BIN_SUB;
                break;
            case TOKEN_ASSIGN_MUL:
                binop = BIN_MUL;
                break;
            case TOKEN_ASSIGN_DIV:
                binop = BIN_DIV;
                break;
            case TOKEN_ASSIGN_MOD:
                binop = BIN_MOD;
                break;
            case TOKEN_ASSIGN_BIT_AND:
                binop = BIN_BIT_AND;
                break;
            case TOKEN_ASSIGN_BIT_OR:
                binop = BIN_BIT_OR;
                break;
            case TOKEN_ASSIGN_BIT_XOR:
                binop = BIN_BIT_XOR;
                break;
            default:
                fprintf(stderr, "Unknown assignment operator at line %d, col %d\n", op->line, op->col);
                exit(1);
        }
        
        // For compound assignments (+=, -=, etc.), create a binary expression
        Expr* binary = expr_new_binary(binop, expr, value, expr->line, expr->col);
        return expr_new_binary(BIN_EQ, expr, binary, expr->line, expr->col);
    }
    
    return expr;
}

Expr* parse_logical_or(Parser* parser) {
    Expr* expr = parse_logical_and(parser);
    
    while (parser_match(parser, TOKEN_LOGICAL_OR)) {
        Expr* right = parse_logical_and(parser);
        expr = expr_new_binary(BIN_OR, expr, right, expr->line, expr->col);
    }
    
    return expr;
}

Expr* parse_logical_and(Parser* parser) {
    Expr* expr = parse_bitwise_or(parser);
    
    while (parser_match(parser, TOKEN_LOGICAL_AND)) {
        Expr* right = parse_bitwise_or(parser);
        expr = expr_new_binary(BIN_AND, expr, right, expr->line, expr->col);
    }
    
    return expr;
}

Expr* parse_bitwise_or(Parser* parser) {
    Expr* expr = parse_bitwise_xor(parser);
    
    while (parser_match(parser, TOKEN_BIT_OR)) {
        Expr* right = parse_bitwise_xor(parser);
        expr = expr_new_binary(BIN_BIT_OR, expr, right, expr->line, expr->col);
    }
    
    return expr;
}

Expr* parse_bitwise_xor(Parser* parser) {
    Expr* expr = parse_bitwise_and(parser);
    
    while (parser_match(parser, TOKEN_BIT_XOR)) {
        Expr* right = parse_bitwise_and(parser);
        expr = expr_new_binary(BIN_BIT_XOR, expr, right, expr->line, expr->col);
    }
    
    return expr;
}

Expr* parse_bitwise_and(Parser* parser) {
    Expr* expr = parse_equality(parser);
    
    while (parser_match(parser, TOKEN_BIT_AND)) {
        Expr* right = parse_equality(parser);
        expr = expr_new_binary(BIN_BIT_AND, expr, right, expr->line, expr->col);
    }
    
    return expr;
}

Expr* parse_equality(Parser* parser) {
    Expr* expr = parse_comparison(parser);
    
    while (parser_match(parser, TOKEN_COMP_EQ) || parser_match(parser, TOKEN_COMP_NEQ)) {
        Token* op = parser->tokens[parser->index - 1];
        Expr* right = parse_comparison(parser);
        
        BinaryOp binop = (op->type == TOKEN_COMP_EQ) ? BIN_EQ : BIN_NEQ;
        expr = expr_new_binary(binop, expr, right, expr->line, expr->col);
    }
    
    return expr;
}

Expr* parse_comparison(Parser* parser) {
    Expr* expr = parse_bitshift(parser);
    
    //! TODO: Will be used LUT here
    while (parser_match(parser, TOKEN_COMP_LT)  || 
           parser_match(parser, TOKEN_COMP_LTE) ||
           parser_match(parser, TOKEN_COMP_GT)  ||
           parser_match(parser, TOKEN_COMP_GTE)) {
        Token* op = parser->tokens[parser->index - 1];
        Expr* right = parse_bitshift(parser);
        
        BinaryOp binop;
        switch (op->type) {
            case TOKEN_COMP_LT:
                binop = BIN_LT;
                break;
            case TOKEN_COMP_LTE:
                binop = BIN_LTE;
                break;
            case TOKEN_COMP_GT:
                binop = BIN_GT;
                break;
            case TOKEN_COMP_GTE:
                binop = BIN_GTE;
                break;
            default:
                fprintf(stderr, "Unknown comparison operator at line %d, col %d\n", op->line, op->col);
                exit(1);
        }
        
        expr = expr_new_binary(binop, expr, right, expr->line, expr->col);
    }
    
    return expr;
}

// Parse bit shift expressions (<<, >>)
Expr* parse_bitshift(Parser* parser) {
    Expr* expr = parse_term(parser);
    
    while (parser_match(parser, TOKEN_BIT_LSHIFT) || parser_match(parser, TOKEN_BIT_RSHIFT)) {
        Token* op = parser->tokens[parser->index - 1];
        Expr* right = parse_term(parser);
        
        BinaryOp binop = (op->type == TOKEN_BIT_LSHIFT) ? BIN_BIT_LSHIFT : BIN_BIT_RSHIFT;
        expr = expr_new_binary(binop, expr, right, expr->line, expr->col);
    }
    
    return expr;
}

// Parse term expressions (+, -)
Expr* parse_term(Parser* parser) {
    Expr* expr = parse_factor(parser);
    
    while (parser_match(parser, TOKEN_OP_ADD) || parser_match(parser, TOKEN_OP_SUB)) {
        Token* op = parser->tokens[parser->index - 1];
        Expr* right = parse_factor(parser);
        
        BinaryOp binop = (op->type == TOKEN_OP_ADD) ? BIN_ADD : BIN_SUB;
        expr = expr_new_binary(binop, expr, right, expr->line, expr->col);
    }
    
    return expr;
}

// Parse factor expressions (*, /, %)
Expr* parse_factor(Parser* parser) {
    Expr* expr = parse_unary(parser);
    
    while (parser_match(parser, TOKEN_OP_MUL) || 
           parser_match(parser, TOKEN_OP_DIV) ||
           parser_match(parser, TOKEN_OP_MOD)) {
        Token* op = parser->tokens[parser->index - 1];
        Expr* right = parse_unary(parser);
        
        BinaryOp binop;
        switch (op->type) {
            case TOKEN_OP_MUL:
                binop = BIN_MUL;
                break;
            case TOKEN_OP_DIV:
                binop = BIN_DIV;
                break;
            case TOKEN_OP_MOD:
                binop = BIN_MOD;
                break;
            default:
                fprintf(stderr, "Unknown factor operator at line %d, col %d\n", op->line, op->col);
                exit(1);
        }
        
        expr = expr_new_binary(binop, expr, right, expr->line, expr->col);
    }
    
    return expr;
}

// Parse unary expressions (!, ~, +, -, ++, --)
Expr* parse_unary(Parser* parser) {
    // Prefix unary operators
    if (parser_match(parser, TOKEN_LOGICAL_NOT) ||
        parser_match(parser, TOKEN_BIT_NOT) ||
        parser_match(parser, TOKEN_OP_ADD) ||
        parser_match(parser, TOKEN_OP_SUB) ||
        parser_match(parser, TOKEN_UNARY_INC) ||
        parser_match(parser, TOKEN_UNARY_DEC)) {
        
        Token* op = parser->tokens[parser->index - 1];
        Expr* right = parse_unary(parser);  // Right-associative
        
        UnaryOp unary_op;
        switch (op->type) {
            case TOKEN_LOGICAL_NOT:
                unary_op = UNARY_LOGICAL_NOT;
                break;
            case TOKEN_BIT_NOT:
                unary_op = UNARY_BIT_NOT;
                break;
            case TOKEN_OP_ADD:
                unary_op = UNARY_POSITIVE;
                break;
            case TOKEN_OP_SUB:
                unary_op = UNARY_NEGATIVE;
                break;
            case TOKEN_UNARY_INC:
                unary_op = UNARY_PRE_INC;
                break;
            case TOKEN_UNARY_DEC:
                unary_op = UNARY_PRE_DEC;
                break;
            default:
                fprintf(stderr, "Unknown unary operator at line %d, col %d\n", op->line, op->col);
                exit(1);
        }
        
        return expr_new_unary(unary_op, right, op->line, op->col);
    }
    
    return parse_postfix(parser);
}

// Parse postfix expressions (++, --)
Expr* parse_postfix(Parser* parser) {
    Expr* expr = parse_primary(parser);
    
    // Handle postfix operators
    if (parser_match(parser, TOKEN_UNARY_INC)) {
        return expr_new_unary(UNARY_POST_INC, expr, expr->line, expr->col);
    } else if (parser_match(parser, TOKEN_UNARY_DEC)) {
        return expr_new_unary(UNARY_POST_DEC, expr, expr->line, expr->col);
    }
    
    return expr;
}

// Parse primary expressions (literals, variables, function calls, grouped expressions)
Expr* parse_primary(Parser* parser) {
    // Parse literals
    if (parser_match(parser, TOKEN_LIT_STRING)) {
        Token* token = parser->tokens[parser->index - 1];
        return expr_new_literal(LITERAL_STRING, token->value, token->line, token->col);
    }
    
    if (parser_match(parser, TOKEN_LIT_INTEGER)) {
        Token* token = parser->tokens[parser->index - 1];
        return expr_new_literal(LITERAL_INT, token->value, token->line, token->col);
    }
    
    if (parser_match(parser, TOKEN_LIT_FLOAT)) {
        Token* token = parser->tokens[parser->index - 1];
        return expr_new_literal(LITERAL_FLOAT, token->value, token->line, token->col);
    }
    
    // Parse variables and function calls
    if (parser_match(parser, TOKEN_IDENTIFIER)) {
        Token* token = parser->tokens[parser->index - 1];
        StringView name = token->value;
        int line = token->line;
        int col = token->col;
        
        // Check if this is a function call
        if (parser_match(parser, TOKEN_LPAREN)) {
            Expr** args = (Expr**)darray_new(Expr*);
            
            // Parse arguments
            if (!parser_check(parser, TOKEN_RPAREN)) {
                do {
                    Expr* arg = parse_expr(parser);
                    darray_push(args, arg);
                } while (parser_match(parser, TOKEN_COMMA));
            }
            
            parser_expect(parser, TOKEN_RPAREN, "Expected ')' after function arguments");
            return expr_new_call(name, args, line, col);
        }
        
        // Otherwise, it's a variable
        return expr_new_variable(name, line, col);
    }
    
    // Parse grouped expressions
    if (parser_match(parser, TOKEN_LPAREN)) {
        Expr* expr = parse_expr(parser);
        parser_expect(parser, TOKEN_RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    // It is okay to crash program here instead of sending diagnostic info
    // because if program reaches here something is wrong with this code.
    diags_new_diag(DIAG_PARSER, DIAG_ERROR, parser->filename, parser->c->line, parser->c->col, "Unexpected token in expression");
    longjmp(parser->loop_jmp, 1);
    
    return NULL;
}

// ----------------------------------
// parse_expr ends
// ----------------------------------

Stmt** parse_helper_compound(Parser* parser) {
    Stmt** stmts = (Stmt**)darray_new(Stmt*);
    parser_expect(parser, TOKEN_LBRACE, "Expected '{' at the beginning of compound statement");
    while (parser->c && parser->c->type != TOKEN_RBRACE) {
        Stmt* stmt = parse_stmt(parser);
        if (stmt) {
            darray_push(stmts, stmt);
        } else {
            parser_expect(parser, TOKEN_SEMI, "Expected ';' after statement");
        }
    }

    parser_expect(parser, TOKEN_RBRACE, "Expected '}' at the end of compound statement");
    return stmts;
}

Stmt* parse_compound(Parser* parser) {
    Stmt** stmts = parse_helper_compound(parser);
    return stmt_new_compound(stmts, parser->c->line, parser->c->col);
}

Stmt* parse_decl(Parser* parser) {
    TokenType type = parser->c->type;
    parser_advance(parser);
    
    StringView name = parser->c->value;
    parser_expect(parser, TOKEN_IDENTIFIER, "Expected identifier after 'let' or 'const'"); 

    if (parser_match(parser, TOKEN_SEMI))
        return stmt_new_decl(name, NULL, type == TOKEN_KW_CONST, parser->c->line, parser->c->col);

    parser_expect(parser, TOKEN_ASSIGN, "Expected '=' after identifier");
    Expr* initializer = parse_expr(parser);

    parser_expect(parser, TOKEN_SEMI, "Expected ';' after declaration");
    return stmt_new_decl(name, initializer, type == TOKEN_KW_CONST, parser->c->line, parser->c->col);
}

Stmt* parse_while(Parser* parser) {
    parser_advance(parser);
    parser_expect(parser, TOKEN_LPAREN, "Expected '(' after 'while'");

    Expr* condition = parse_expr(parser);
    parser_expect(parser, TOKEN_RPAREN, "Expected ')' after condition");

    Stmt** body = parse_helper_compound(parser);
    return stmt_new_while(condition, body, parser->c->line, parser->c->col);
}

Stmt* parse_if(Parser* parser) {
    parser_advance(parser);
    parser_expect(parser, TOKEN_LPAREN, "Expected '(' after 'if'");
    
    Expr* condition = parse_expr(parser);
    parser_expect(parser, TOKEN_RPAREN, "Expected ')' after condition");

    Stmt** then_branch = parse_helper_compound(parser);
    Stmt* else_branch = NULL;

    if (parser_match(parser, TOKEN_KW_ELSE))
        else_branch = parse_stmt(parser);
    else if (parser_match(parser, TOKEN_KW_IF))
        else_branch = parse_if(parser);

    return stmt_new_if(condition, then_branch, else_branch, parser->c->line, parser->c->col);
}

Stmt* parse_method_decl(Parser* parser) {
    parser_advance(parser);

    StringView name = parser->c->value;
    parser_expect(parser, TOKEN_IDENTIFIER, "Expected identifier after 'method'");

    parser_expect(parser, TOKEN_LPAREN, "Expected '(' after method name");
    Expr** params = (Expr**)darray_new(Expr*);
    while (parser->c && parser->c->type != TOKEN_EOF && parser->c->type != TOKEN_RPAREN) {
        if (parser->c->type == TOKEN_IDENTIFIER) {
            Expr* param = expr_new_variable(parser->c->value, parser->c->line, parser->c->col);
            darray_push(params, param);
            parser_advance(parser);
        } else {
            parser_expect(parser, TOKEN_COMMA, "Expected ',' or ')' after parameter");
        }
    }

    parser_expect(parser, TOKEN_RPAREN, "Expected ')' after parameter list");
    Stmt** body = parse_helper_compound(parser);

    return stmt_new_method_decl(name, params, body, parser->c->line, parser->c->col);
}

Stmt* parse_return(Parser* parser) {
    parser_advance(parser);
    Expr* value = NULL;

    if (!parser_check(parser, TOKEN_SEMI))
        value = parse_expr(parser);

    parser_expect(parser, TOKEN_SEMI, "Expected ';' after return statement");
    return stmt_new_return(value, parser->c->line, parser->c->col);
}

Stmt* parse_break(Parser* parser) {
    parser_advance(parser);
    parser_expect(parser, TOKEN_SEMI, "Expected ';' after break statement");
    return stmt_new_break(parser->c->line, parser->c->col);
}

Stmt* parse_continue(Parser* parser) {
    parser_advance(parser);
    parser_expect(parser, TOKEN_SEMI, "Expected ';' after continue statement");
    return stmt_new_continue(parser->c->line, parser->c->col);
}

Stmt* parse_stmt(Parser* parser) {
    switch (parser->c->type) {
        case TOKEN_KW_LET:
        case TOKEN_KW_CONST:
            return parse_decl(parser);
        case TOKEN_LBRACE:
            return parse_compound(parser);
        case TOKEN_KW_WHILE:
            return parse_while(parser);
        case TOKEN_KW_IF:
            return parse_if(parser);
        case TOKEN_KW_METHOD:
            return parse_method_decl(parser);
        case TOKEN_KW_RETURN:
            return parse_return(parser);
        case TOKEN_KW_BREAK:
            return parse_break(parser);
        case TOKEN_KW_CONTINUE:
            return parse_continue(parser);
        case TOKEN_SEMI:
            parser_advance(parser);
            return NULL;
        default:
        {
            Expr* expr = parse_expr(parser);
            if (expr) {
                parser_expect(parser, TOKEN_SEMI, "Expected ';' after expression");
                return stmt_new_expr(expr, parser->c->line, parser->c->col);
            }
        }


    }

    return NULL;
}

Stmt** parse(const char* filename, Token** tokens) {
    Parser parser;
    parser_init(&parser, filename, tokens);

    Stmt** stmts = (Stmt**)darray_new(Stmt*);

    setjmp(parser.loop_jmp);
    while(parser.c && parser.c->type != TOKEN_EOF) {
        Stmt* stmt = parse_stmt(&parser);
        if (stmt)
            darray_push(stmts, stmt); 
    }

    return stmts;
}