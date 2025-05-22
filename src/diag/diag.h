#ifndef __DIAG_H
#define __DIAG_H

#include <stddef.h>

typedef enum {
    DIAG_LEXER,
    DIAG_PARSER,
    DIAG_SEMANTIC,
    DIAG_RUNTIME,
} DiagnosticType;

typedef enum {
    DIAG_WARN,
    DIAG_ERROR,
    DIAG_INFO,
} DiagnosticLevel;

typedef struct {
    DiagnosticType type;
    DiagnosticLevel level;
    const char* filename;
    char* msg;
    int line;
    int col;
} Diagnostic;

void diags_init();

void diags_new_diag(DiagnosticType type, DiagnosticLevel level, const char* filename, int line, int column, const char* fmt, ...);
void diags_free_diag(Diagnostic* error);

size_t diags_errs_size();
size_t diags_warns_size();

void diags_free();
void diags_dump();

#endif
