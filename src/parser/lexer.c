#include "lexer.h"

#include <string.h>
#include <ctype.h>

#include "../utils/darray.h"
#include "../utils/utf8.h"

#include "../diag/diag.h"
#include "../lut.h"

typedef struct _Lexer {
    const char* src;
    const char* filename;
    const char* c;
    size_t c_len; // length of char as bytes
    int line;
    int col;
} Lexer;

static void lexer_init(Lexer* lexer, const char* filename, const char* src) {
    lexer->filename = filename;
    lexer->src = src;
    lexer->line = 1;
    lexer->col = 1;
    lexer->c = src;
    lexer->c_len = utf8_csize(src);
}

static void advance(Lexer* lexer) {
    if (*lexer->c == '\n') {
        lexer->line++;
        lexer->col = 0;
    }

    lexer->col++;
    lexer->c += lexer->c_len;
    lexer->c_len = utf8_csize(lexer->c);

}

static void skipwhitespace(Lexer* lexer) {
    while(*lexer->c != '\0' && isspace(*lexer->c) && *lexer->c != '\n')
        advance(lexer);
}

static void skipcomment(Lexer* lexer) {
    while (*lexer->c && *lexer->c != '\n')
        advance(lexer);
}

static Token* get_identifier(Lexer* lexer) {
    StringView sv;
    sv.data = lexer->c;
    sv.byte_len = 0;
    sv.len = 0;

    while (utf8_isalnum(lexer->c) || *lexer->c == '_') {
        sv.byte_len += lexer->c_len;
        sv.len++;
        advance(lexer);
    }

    // Check keywords here
    uintptr_t idx = 0;
    hashmap_get(lut_kw_to_token, sv.data, sv.byte_len, &idx);
    if (idx)
        return token_new((TokenType)idx, sv, lexer->line, lexer->col - sv.len);

    return token_new(TOKEN_IDENTIFIER, sv, lexer->line, lexer->col - sv.len);
}

static Token* get_number(Lexer* lexer) {
    StringView sv;
    sv.data = lexer->c;
    sv.byte_len = 0;
    sv.len = 0;

    while (utf8_isdigit(lexer->c)) {
        sv.byte_len += lexer->c_len;
        sv.len++;
        advance(lexer);
    }

    if (*lexer->c == '.') {
        sv.byte_len++;
        sv.len++;
        advance(lexer);

        while (utf8_isdigit(lexer->c)) {
            sv.byte_len++;
            sv.len++;
            advance(lexer);
        }

        return token_new(TOKEN_LIT_FLOAT, sv, lexer->line, lexer->col - sv.len);
    }

    return token_new(TOKEN_LIT_INTEGER, sv, lexer->line, lexer->col - sv.len);
}

static Token* get_string(Lexer* lexer) {
    advance(lexer);

    StringView sv;
    sv.data = lexer->c;
    sv.byte_len = 0;
    sv.len = 0;

    //! TODO: Handle escape sequences in strings
    while (*lexer->c != '"' && *lexer->c != '\0' && *lexer->c != '\n' && *lexer->c != '\r') {
        sv.byte_len += lexer->c_len;
        sv.len++;
        advance(lexer);
    }

    if (*lexer->c != '"') {
        diags_new_diag(DIAG_LEXER, DIAG_ERROR, lexer->filename, lexer->line, lexer->col, "Unterminated string");
        return token_new(TOKEN_EOF, EMPTY_STRV, lexer->line, lexer->col);
    }

    advance(lexer);
    return token_new(TOKEN_LIT_STRING, sv, lexer->line, lexer->col - sv.len - 1);
}

static Token* get_token(Lexer* lexer) {
    skipwhitespace(lexer);
    if (*lexer->c == '\n') {
        int last_col = lexer->col;
        advance(lexer);
        // return token_new(TOKEN_LF, EMPTY_STRV, lexer->line - 1, last_col);
        return NULL;
    }

    if (!*lexer->c)
        return token_new(TOKEN_EOF, EMPTY_STRV, lexer->line, lexer->col);

    if (utf8_isalpha(lexer->c) || *lexer->c == '_')
        return get_identifier(lexer);
    else if (utf8_isdigit(lexer->c))
        return get_number(lexer);

    switch (*lexer->c) {
        case '\n':
            advance(lexer);
            // return token_new(TOKEN_LF, EMPTY_STRV, lexer->line - 1, lexer->col);
            return NULL;
        case '#':
            skipcomment(lexer);
            if (*lexer->c == '\n') {
                int last_col = lexer->col;
                advance(lexer);
                // return token_new(TOKEN_LF, EMPTY_STRV, lexer->line - 1, last_col);
                return NULL;
            }

            advance(lexer);
            return get_token(lexer);
        case ';':
            advance(lexer);
            return token_new(TOKEN_SEMI, EMPTY_STRV, lexer->line, lexer->col);
        case ':':
            advance(lexer);
            return token_new(TOKEN_COLON, EMPTY_STRV, lexer->line, lexer->col);
        case '.':
            advance(lexer);
            return token_new(TOKEN_DOT, EMPTY_STRV, lexer->line, lexer->col);
        case ',':
            advance(lexer);
            return token_new(TOKEN_COMMA, EMPTY_STRV, lexer->line, lexer->col);
        case '(':
            advance(lexer);
            return token_new(TOKEN_LPAREN, EMPTY_STRV, lexer->line, lexer->col);
        case ')':
            advance(lexer);
            return token_new(TOKEN_RPAREN, EMPTY_STRV, lexer->line, lexer->col);
        case '{':
            advance(lexer);
            return token_new(TOKEN_LBRACE, EMPTY_STRV, lexer->line, lexer->col);
        case '}':
            advance(lexer);
            return token_new(TOKEN_RBRACE, EMPTY_STRV, lexer->line, lexer->col);
        case '[':
            advance(lexer);
            return token_new(TOKEN_LBRACKET, EMPTY_STRV, lexer->line, lexer->col);
        case ']':
            advance(lexer);
            return token_new(TOKEN_RBRACKET, EMPTY_STRV, lexer->line, lexer->col);
        case '+':
            advance(lexer);
            if (*lexer->c == '+') {
                advance(lexer);
                return token_new(TOKEN_UNARY_INC, EMPTY_STRV, lexer->line, lexer->col);
            }

            if (*lexer->c == '=') {
                advance(lexer);
                return token_new(TOKEN_ASSIGN_ADD, EMPTY_STRV, lexer->line, lexer->col);
            }

            return token_new(TOKEN_OP_ADD, EMPTY_STRV, lexer->line, lexer->col);
        case '-':
            advance(lexer);
            if (*lexer->c == '-') {
                advance(lexer);
                return token_new(TOKEN_UNARY_DEC, EMPTY_STRV, lexer->line, lexer->col);
            }

            if (*lexer->c == '=') {
                advance(lexer);
                return token_new(TOKEN_ASSIGN_SUB, EMPTY_STRV, lexer->line, lexer->col);
            }

            return token_new(TOKEN_OP_SUB, EMPTY_STRV, lexer->line, lexer->col);
        case '*':
            advance(lexer);
            if (*lexer->c == '=') {
                advance(lexer);
                return token_new(TOKEN_ASSIGN_MUL, EMPTY_STRV, lexer->line, lexer->col);
            }

            return token_new(TOKEN_OP_MUL, EMPTY_STRV, lexer->line, lexer->col);
        case '/':
            advance(lexer);
            if (*lexer->c == '=') {
                advance(lexer);
                return token_new(TOKEN_ASSIGN_DIV, EMPTY_STRV, lexer->line, lexer->col);
            }

            return token_new(TOKEN_OP_DIV, EMPTY_STRV, lexer->line, lexer->col);
        case '%':
            advance(lexer);
            if (*lexer->c == '=') {
                advance(lexer);
                return token_new(TOKEN_ASSIGN_MOD, EMPTY_STRV, lexer->line, lexer->col);
            }

            return token_new(TOKEN_OP_MOD, EMPTY_STRV, lexer->line, lexer->col);
        case '=':
            advance(lexer);
            if (*lexer->c == '=') {
                advance(lexer);
                return token_new(TOKEN_COMP_EQ, EMPTY_STRV, lexer->line, lexer->col);
            }

            return token_new(TOKEN_ASSIGN, EMPTY_STRV, lexer->line, lexer->col);
        case '!':
            advance(lexer);
            if (*lexer->c == '=') {
                advance(lexer);
                return token_new(TOKEN_COMP_NEQ, EMPTY_STRV, lexer->line, lexer->col);
            }

            return token_new(TOKEN_UNARY_NOT, EMPTY_STRV, lexer->line, lexer->col);
        case '<':
            advance(lexer);
            if (*lexer->c == '=') {
                advance(lexer);
                return token_new(TOKEN_COMP_LTE, EMPTY_STRV, lexer->line, lexer->col);
            }

            return token_new(TOKEN_COMP_LT, EMPTY_STRV, lexer->line, lexer->col);
        case '>':
            advance(lexer);
            if (*lexer->c == '=') {
                advance(lexer);
                return token_new(TOKEN_COMP_GTE, EMPTY_STRV, lexer->line, lexer->col);
            }

            return token_new(TOKEN_COMP_GT, EMPTY_STRV, lexer->line, lexer->col);
        case '&':
            advance(lexer);
            if (*lexer->c == '&') {
                advance(lexer);
                return token_new(TOKEN_LOGICAL_AND, EMPTY_STRV, lexer->line, lexer->col);
            }

            if (*lexer->c == '=') {
                advance(lexer);
                return token_new(TOKEN_ASSIGN_BIT_AND, EMPTY_STRV, lexer->line, lexer->col);
            }

            return token_new(TOKEN_BIT_AND, EMPTY_STRV, lexer->line, lexer->col);
        case '|':
            advance(lexer);
            if (*lexer->c == '|') {
                advance(lexer);
                return token_new(TOKEN_LOGICAL_OR, EMPTY_STRV, lexer->line, lexer->col);
            }

            if (*lexer->c == '=') {
                advance(lexer);
                return token_new(TOKEN_ASSIGN_BIT_OR, EMPTY_STRV, lexer->line, lexer->col);
            }

            return token_new(TOKEN_BIT_OR, EMPTY_STRV, lexer->line, lexer->col);
        case '^':
            advance(lexer);
            if (*lexer->c == '=') {
                advance(lexer);
                return token_new(TOKEN_ASSIGN_BIT_XOR, EMPTY_STRV, lexer->line, lexer->col);
            }

            return token_new(TOKEN_BIT_XOR, EMPTY_STRV, lexer->line, lexer->col);
        case '~':
            advance(lexer);
            if (*lexer->c == '=') {
                advance(lexer);
                return token_new(TOKEN_ASSIGN_BIT_NOT, EMPTY_STRV, lexer->line, lexer->col);
            }

            return token_new(TOKEN_BIT_NOT, EMPTY_STRV, lexer->line, lexer->col);
        case '"':
            return get_string(lexer);
        default:
            diags_new_diag(DIAG_LEXER, DIAG_ERROR, lexer->filename, lexer->line, lexer->col, "Unexpected character '%.*s'", lexer->c_len, lexer->c);
            advance(lexer);
            return get_token(lexer);
    }
}

Token** lex(const char* filename, const char* src) {
    Lexer lexer;
    lexer_init(&lexer, filename, src);

    Token** tokens = (Token**)darray_new(Token*);
    while (1) {
        Token* token = get_token(&lexer);
        if (!token)
            continue;

        if (token->type == TOKEN_EOF) {
            darray_push(tokens, token);
            break;
        }

        darray_push(tokens, token);
    }

    return tokens;
}