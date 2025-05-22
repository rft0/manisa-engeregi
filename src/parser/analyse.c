#include "analyse.h"

#include "../utils/darray.h"
#include "../utils/str.h"

#include "../diag/diag.h"

#include "node.h"

/*
-- Things to check:
    - Check for undeclared variables
    - Check for variable shadowing
    - Check for function argument arity
    - Check for function overloading
    - Check for function calls with wrong number of arguments
*/

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
    Symbol** symbols;
    int depth;
} Scope;

typedef struct Analyser {
    const char* filename;
    Stmt** stmts;
    int index;

    Scope* current_scope;
    int inside_loop;
    int inside_method;
} Analyser;

// Forward decls
Scope* scope_new(Scope* parent);
void scope_free(Scope* scope);
void scope_enter(Analyser* ctx);
void scope_exit(Analyser* ctx);

Symbol* symbol_new(StringView name, int is_const, int line, int col);
void symbol_free(Symbol* symbol);
Symbol* scope_lookup(Scope* scope, StringView name);
Symbol* scope_lookup_current(Scope* scope, StringView name);
int scope_define(Scope* scope, Symbol* symbol);

// to me, nested functions will not be allowed

void analyser_init(Analyser* analyser, const char* filename, Stmt** stmts) {
    analyser->filename = filename;
    analyser->stmts = stmts;
    analyser->index = 0;

    analyser->current_scope = NULL;
    analyser->inside_loop = 0;
    analyser->inside_method = 0;
}

void analyse(const char* filename, Stmt** stmts) {


}