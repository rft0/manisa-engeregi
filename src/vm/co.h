#ifndef __CO_H
#define __CO_H

#include "../parser/stmt.h"

#include <stdint.h>

#include "../utils/hashmap.h"

typedef struct MECodeObject {
    uint8_t* bytecode;
    size_t size;
    HashMap* globals;
} MECodeObject;

MECodeObject* co_new(Stmt** stmts);

#endif