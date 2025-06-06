#include "co.h"

#include <stdlib.h>

#include "../utils/hashmap.h"
#include "../utils/darray.h"

MECodeObject* co_new(Stmt** stmts) {
    MECodeObject* co = malloc(sizeof(MECodeObject));
    
    HashMap* map = hashmap_new();
    if (!map) {
        free(co);
        return NULL;
    }

    return co;
}