#include "diag.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "../utils/darray.h"
#include "../utils/utf8.h"

struct {
    Diagnostic** errs;
    Diagnostic** warns;
    Diagnostic** infos;
} g_diags = {0};

void diags_init() {
    g_diags.errs = darray_new(Diagnostic**);
    g_diags.warns = darray_new(Diagnostic**);
    g_diags.infos = darray_new(Diagnostic**);
}

void diags_new_diag(DiagnosticType type, DiagnosticLevel level, const char* filename, int line, int column, const char* fmt, ...) {
    Diagnostic* diag = (Diagnostic*)malloc(sizeof(Diagnostic));
    diag->filename = filename;
    diag->type = type;
    diag->level = level;
    diag->line = line;
    diag->col = column;

    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);

    if (size < 0) {
        va_end(args);
        free(diag);
        return;
    }

    diag->msg = (char*)malloc(size + 1);
    vsnprintf(diag->msg, size + 1, fmt, args);

    va_end(args);

    if (level == DIAG_ERROR)
        darray_push(g_diags.errs, diag);
    else if (level == DIAG_WARN)
        darray_push(g_diags.warns, diag);
    else
        darray_push(g_diags.infos, diag);
}

void diags_free_diag(Diagnostic* diag) {
    if (diag == NULL)
        return;
    
    free(diag->msg);
    free(diag);
}

size_t diags_errs_size() {
    return darray_size(g_diags.errs);
}

size_t diags_warns_size() {
    return darray_size(g_diags.warns);
}

size_t diags_infos_size() {
    return darray_size(g_diags.infos);
}

void diags_free() {
    darray_for(g_diags.errs) diags_free_diag(g_diags.errs[__i]);
    darray_for(g_diags.warns) diags_free_diag(g_diags.warns[__i]);
    darray_for(g_diags.infos) diags_free_diag(g_diags.infos[__i]);

    darray_free(g_diags.errs);
    darray_free(g_diags.warns);
    darray_free(g_diags.infos);

    g_diags.errs = NULL;
    g_diags.warns = NULL;
    g_diags.infos = NULL;
}

void diags_dump() {
    darray_for(g_diags.errs) {
        Diagnostic* diag = g_diags.errs[__i];
        printf("%.*s:%d:%d: error: %s\n", (int)utf8_strsize(diag->filename), diag->filename, diag->line, diag->col, diag->msg);
    }

    darray_for(g_diags.warns) {
        Diagnostic* diag = g_diags.warns[__i];
        printf("%.*s:%d:%d: warn: %s\n", (int)utf8_strsize(diag->filename), diag->filename, diag->line, diag->col, diag->msg);
    }

    darray_for(g_diags.infos) {
        Diagnostic* diag = g_diags.infos[__i];
        printf("%.*s:%d:%d: info: %s\n", (int)utf8_strsize(diag->filename), diag->filename, diag->line, diag->col, diag->msg);
    }
}