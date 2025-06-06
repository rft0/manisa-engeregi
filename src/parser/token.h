#ifndef __TOKEN_H
#define __TOKEN_H

#include "../utils/str.h"

typedef enum {
    TOKEN_EOF,
    TOKEN_LF,
    TOKEN_SEMI,
    TOKEN_COLON,
    TOKEN_DOT,
    TOKEN_COMMA,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_IDENTIFIER,

    // Literals
    TOKEN_LIT_STRING,
    TOKEN_LIT_INTEGER,
    TOKEN_LIT_FLOAT,
    TOKEN_LIT_NONE,

    // Arithmetic Operators
    TOKEN_OP_ADD,
    TOKEN_OP_SUB,
    TOKEN_OP_MUL,
    TOKEN_OP_DIV,
    TOKEN_OP_MOD,

    // Compound Assignment Operatos
    TOKEN_ASSIGN,
    TOKEN_ASSIGN_ADD,
    TOKEN_ASSIGN_SUB,
    TOKEN_ASSIGN_MUL,
    TOKEN_ASSIGN_DIV,
    TOKEN_ASSIGN_MOD,

    TOKEN_ASSIGN_BIT_AND,
    TOKEN_ASSIGN_BIT_OR,
    TOKEN_ASSIGN_BIT_XOR,
    TOKEN_ASSIGN_BIT_NOT,

    // Unary Operators
    TOKEN_UNARY_NOT,
    TOKEN_UNARY_INC,
    TOKEN_UNARY_DEC,

    // Logical Operators
    TOKEN_LOGICAL_AND,
    TOKEN_LOGICAL_OR,
    TOKEN_LOGICAL_NOT,

    // Comparasion Operators
    TOKEN_COMP_EQ,
    TOKEN_COMP_NEQ,
    TOKEN_COMP_LT,
    TOKEN_COMP_LTE,
    TOKEN_COMP_GT,
    TOKEN_COMP_GTE,

    // Bitwise Operators
    TOKEN_BIT_AND,
    TOKEN_BIT_OR,
    TOKEN_BIT_XOR,
    TOKEN_BIT_NOT,
    TOKEN_BIT_LSHIFT,
    TOKEN_BIT_RSHIFT, //! ONLY ARITHMETIC RIGHT SHIFT NOT LOGICAL

    // Keywords
    TOKEN_KW_CONST,
    TOKEN_KW_LET,
    TOKEN_KW_AND,
    TOKEN_KW_OR,
    TOKEN_KW_IF,
    TOKEN_KW_ELSE,
    TOKEN_KW_WHILE,
    TOKEN_KW_FUNCTION,
    TOKEN_KW_RETURN,
    TOKEN_KW_BREAK,
    TOKEN_KW_CONTINUE,
} TokenType;

typedef struct {
    TokenType type;
    StringView value;
    int line;
    int col;
} Token;

Token* token_new(TokenType type, StringView value, int line, int col);
void token_dump(Token* token);

#endif