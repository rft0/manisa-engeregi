#include "lut.h"
#include "parser/node.h"

#define KW_CONST        "sabit"
#define KW_LET          "değişken"
#define KW_AND          "ve"
#define KW_OR           "veya"
#define KW_IF           "şayet"
#define KW_ELSE         "değilse"
#define KW_WHILE        "madem"
#define KW_METHOD       "marifet"
#define KW_RETURN       "tebliğ"
#define KW_BREAK        "yeter"
#define KW_CONTINUE     "devam"
#define KW_NONE         "none"

HashMap* lut_kw_to_token = NULL;

void lut_init() {
    lut_kw_to_token = hashmap_new();

    hashmap_set(lut_kw_to_token, hashmap_lit_str(KW_CONST), TOKEN_KW_CONST);
    hashmap_set(lut_kw_to_token, hashmap_lit_str(KW_LET), TOKEN_KW_LET);
    hashmap_set(lut_kw_to_token, hashmap_lit_str(KW_AND), TOKEN_LOGICAL_AND);
    hashmap_set(lut_kw_to_token, hashmap_lit_str(KW_OR), TOKEN_LOGICAL_OR);
    hashmap_set(lut_kw_to_token, hashmap_lit_str(KW_IF), TOKEN_KW_IF);
    hashmap_set(lut_kw_to_token, hashmap_lit_str(KW_ELSE), TOKEN_KW_ELSE);
    hashmap_set(lut_kw_to_token, hashmap_lit_str(KW_WHILE), TOKEN_KW_WHILE);
    hashmap_set(lut_kw_to_token, hashmap_lit_str(KW_METHOD), TOKEN_KW_METHOD);
    hashmap_set(lut_kw_to_token, hashmap_lit_str(KW_RETURN), TOKEN_KW_RETURN);
    hashmap_set(lut_kw_to_token, hashmap_lit_str(KW_BREAK), TOKEN_KW_BREAK);
    hashmap_set(lut_kw_to_token, hashmap_lit_str(KW_CONTINUE), TOKEN_KW_CONTINUE);
    hashmap_set(lut_kw_to_token, hashmap_lit_str(KW_NONE), TOKEN_LIT_NONE);
}

void lut_free() {
    hashmap_free(lut_kw_to_token);
    
    lut_kw_to_token = NULL;
}

// For debug purposes
const char* lut_token_to_str[] = {
    [TOKEN_EOF] = "EOF",
    [TOKEN_SEMI] = "SEMICOLON",
    [TOKEN_LF] = "LF",

    [TOKEN_COLON] = "COLON",
    [TOKEN_DOT] = "DOT",
    [TOKEN_COMMA] = "COMMA",
    [TOKEN_LPAREN] = "LPAREN",
    [TOKEN_RPAREN] = "RPAREN",
    [TOKEN_LBRACE] = "LBRACE",
    [TOKEN_RBRACE] = "RBRACE",
    [TOKEN_LBRACKET] = "LBRACKET",
    [TOKEN_RBRACKET] = "RBRACKET",
    [TOKEN_IDENTIFIER] = "IDENTIFIER",

    // Literals
    [TOKEN_LIT_STRING] = "LIT_STRING",
    [TOKEN_LIT_INTEGER] = "LIT_INTEGER",
    [TOKEN_LIT_FLOAT] = "LIT_FLOAT",
    [TOKEN_LIT_NONE] = "LIT_NONE",

    // Arithmetic Operators
    [TOKEN_OP_ADD] = "OP_ADD",
    [TOKEN_OP_SUB] = "OP_SUB",
    [TOKEN_OP_MUL] = "OP_MUL",
    [TOKEN_OP_DIV] = "OP_DIV",
    [TOKEN_OP_MOD] = "OP_MOD",

    // Compound Assignment Operators
    [TOKEN_ASSIGN] = "ASSIGN",
    [TOKEN_ASSIGN_ADD] = "ASSIGN_ADD",
    [TOKEN_ASSIGN_SUB] = "ASSIGN_SUB",
    [TOKEN_ASSIGN_MUL] = "ASSIGN_MUL",
    [TOKEN_ASSIGN_DIV] = "ASSIGN_DIV",
    [TOKEN_ASSIGN_MOD] = "ASSIGN_MOD",
    [TOKEN_ASSIGN_BIT_AND] = "ASSIGN_BIT_AND",
    [TOKEN_ASSIGN_BIT_OR] = "ASSIGN_BIT_OR",
    [TOKEN_ASSIGN_BIT_XOR] = "ASSIGN_BIT_XOR",
    [TOKEN_ASSIGN_BIT_NOT] = "ASSIGN_BIT_NOT",

    // Unary Operators
    [TOKEN_UNARY_NOT] = "UNARY_NOT",
    [TOKEN_UNARY_INC] = "UNARY_INC",
    [TOKEN_UNARY_DEC] = "UNARY_DEC",

    // Logical Operators
    [TOKEN_LOGICAL_AND] = "LOGICAL_AND",
    [TOKEN_LOGICAL_OR] = "LOGICAL_OR",
    [TOKEN_LOGICAL_NOT] = "LOGICAL_NOT",

    // Comparison Operators
    [TOKEN_COMP_EQ] = "COMP_EQ",
    [TOKEN_COMP_NEQ] = "COMP_NEQ",
    [TOKEN_COMP_LT] = "COMP_LT",
    [TOKEN_COMP_LTE] = "COMP_LTE",
    [TOKEN_COMP_GT] = "COMP_GT",
    [TOKEN_COMP_GTE] = "COMP_GTE",

    // Bitwise Operators
    [TOKEN_BIT_AND] = "BIT_AND",
    [TOKEN_BIT_OR] = "BIT_OR",
    [TOKEN_BIT_XOR] = "BIT_XOR",
    [TOKEN_BIT_NOT] = "BIT_NOT",
    [TOKEN_BIT_LSHIFT] = "BIT_LSHIFT",
    [TOKEN_BIT_RSHIFT] = "BIT_RSHIFT",

    // Keywords
    [TOKEN_KW_CONST] = "KW_CONST",
    [TOKEN_KW_LET] = "KW_LET",
    [TOKEN_KW_AND] = "KW_AND",
    [TOKEN_KW_OR] = "KW_OR",
    [TOKEN_KW_IF] = "KW_IF",
    [TOKEN_KW_ELSE] = "KW_ELSE",
    [TOKEN_KW_WHILE] = "KW_WHILE",
    [TOKEN_KW_METHOD] = "KW_METHOD",
    [TOKEN_KW_RETURN] = "KW_RETURN",
    [TOKEN_KW_BREAK] = "KW_BREAK",
    [TOKEN_KW_CONTINUE] = "KW_CONTINUE",
};

BinaryOp lut_token_to_binop[] = {
    [TOKEN_OP_ADD] = BIN_ADD,
    [TOKEN_OP_SUB] = BIN_SUB,
    [TOKEN_OP_MUL] = BIN_MUL,
    [TOKEN_OP_DIV] = BIN_DIV,
    [TOKEN_OP_MOD] = BIN_MOD,

    [TOKEN_COMP_EQ] = BIN_EQ,
    [TOKEN_COMP_NEQ] = BIN_NEQ,
    [TOKEN_COMP_LT] = BIN_LT,
    [TOKEN_COMP_LTE] = BIN_LTE,
    [TOKEN_COMP_GT] = BIN_GT,
    [TOKEN_COMP_GTE] = BIN_GTE,

    [TOKEN_LOGICAL_AND] = BIN_AND,
    [TOKEN_LOGICAL_OR] = BIN_OR,

    [TOKEN_BIT_AND] = BIN_BIT_AND,
    [TOKEN_BIT_OR] = BIN_BIT_OR,
    [TOKEN_BIT_XOR] = BIN_BIT_XOR,
};

// Create lut for assignment operators
int lut_assignment_ops[] = {
    [TOKEN_ASSIGN] = BIN_EQ,
    [TOKEN_ASSIGN_ADD] = BIN_ADD,
    [TOKEN_ASSIGN_SUB] = BIN_SUB,
    [TOKEN_ASSIGN_MUL] = BIN_MUL,
    [TOKEN_ASSIGN_DIV] = BIN_DIV,
    [TOKEN_ASSIGN_MOD] = BIN_MOD,
    [TOKEN_ASSIGN_BIT_AND] = BIN_BIT_AND,
    [TOKEN_ASSIGN_BIT_OR] = BIN_BIT_OR,
    [TOKEN_ASSIGN_BIT_XOR] = BIN_BIT_XOR,
};

BinaryOp lut_compound_to_binop[] = {
    [TOKEN_ASSIGN_ADD] = BIN_ADD,
    [TOKEN_ASSIGN_SUB] = BIN_SUB,
    [TOKEN_ASSIGN_MUL] = BIN_MUL,
    [TOKEN_ASSIGN_DIV] = BIN_DIV,
    [TOKEN_ASSIGN_MOD] = BIN_MOD,
    [TOKEN_ASSIGN_BIT_AND] = BIN_BIT_AND,
    [TOKEN_ASSIGN_BIT_OR] = BIN_BIT_OR,
    [TOKEN_ASSIGN_BIT_XOR] = BIN_BIT_XOR,
    [TOKEN_ASSIGN_BIT_NOT] = BIN_BIT_NOT,
};