#ifndef __FUNCTIONOBJECT_H
#define __FUNCTIONOBJECT_H

#include "../object.h"
#include "strobject.h"

extern METypeObject me_type_function;

typedef struct {
    ME_OBJHEAD
    MEStrObject* ob_name;
} MEFunctionObject;

static inline int me_function_check(MEObject* obj) {
    return ME_TYPE_CHECK(obj, &me_type_function);
}

#endif