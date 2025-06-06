#include "co.h"

#include <stdlib.h>

#include "../utils/hashmap.h"
#include "../utils/darray.h"

MECodeObject* co_new(Stmt** stmts) {
    MECodeObject* co = malloc(sizeof(MECodeObject));
    
    HashMap* map = hashmap_new();
    if (!map) {
        free(co);
        return NULL; // Memory allocation failed
    }

    for (size_t i = 0; stmts[i]; i++) {
        Stmt* stmt = stmts[i];
        if (stmt->type == STMT_FUNC) {
            hashmap_set(map, stmt->name, strlen(stmt->name), (uintptr_t)stmt);
        }
    }

    co->bytecode = (uint8_t*)map;
    co->size = hashmap_size(map);

    return co;
}