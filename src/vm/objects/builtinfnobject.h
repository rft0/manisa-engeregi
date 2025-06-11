#ifndef __BUILTINFNOBJECT_H
#define __BUILTINFNOBJECT_H

#include "../object.h"

extern METypeObject me_type_builtinfn;

typedef MEObject* (*MEBuiltinFunction)(MEObject* self, MEObject** args);

typedef struct {
    ME_OBJHEAD
    const char* ob_name;
    MEBuiltinFunction fn;
} MEBuiltinFnObject;

static inline int me_builtinfn_check(MEObject* obj) {
    return ME_TYPE_CHECK(obj, &me_type_builtinfn);
}

MEObject* me_builtinfn_new(const char* name, MEBuiltinFunction fn);

#endif