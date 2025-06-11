#ifndef __ANALYSER_H
#define __ANALYSER_H

#include "stmt.h"

#include "../utils/hashmap.h"

typedef struct Symbol {
    StringView name;
    int is_const;
    int is_initialized;
    int nargs;
    int line;
    int col;
} Symbol;

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

void analyse(const char* filename, Stmt** stmts);

Symbol* symbol_new(StringView name, int is_const, int line, int col);
void symbol_free(Symbol* symbol);
int scope_define(Scope* scope, Symbol* symbol);

#endif