#ifndef __FUNCTIONOBJECT_H
#define __FUNCTIONOBJECT_H

#include "../object.h"
#include "../co.h"

#include "strobject.h"


extern METypeObject me_type_function;

typedef struct {
    ME_OBJHEAD
    MECodeObject* co;
    size_t nargs;
} MEFunctionObject;

static inline int me_function_check(MEObject* obj) {
    return ME_TYPE_CHECK(obj, &me_type_function);
}

MEObject* me_function_new(MECodeObject* co, size_t nargs);

#endif