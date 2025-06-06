#include "floatobject.h"

#include <stdlib.h>

#include "strobject.h"

MEObject* me_float_from_double(double value) {
    MEFloatObject* obj = (MEFloatObject*)malloc(sizeof(MEFloatObject));
    if (!obj)
        return NULL;

    obj->ob_type = &me_type_float;
    obj->ob_refcount = 1;

    obj->ob_value = value;

    return (MEObject*)obj;
}

MEObject* me_float_from_long(long value) {
    MEFloatObject* obj = (MEFloatObject*)malloc(sizeof(MEFloatObject));
    if (!obj)
        return NULL;

    obj->ob_type = &me_type_float;
    obj->ob_refcount = 1;

    obj->ob_value = value;

    return (MEObject*)obj;
}

MEObject* me_float_from_ulong(unsigned long value) {
    MEFloatObject* obj = (MEFloatObject*)malloc(sizeof(MEFloatObject));
    if (!obj)
        return NULL;

    obj->ob_type = &me_type_float;
    obj->ob_refcount = 1;

    obj->ob_value = value;

    return (MEObject*)obj;
}

MEObject* me_float_from_str(const char* str) {
    MEFloatObject* obj = (MEFloatObject*)malloc(sizeof(MEFloatObject));
    if (!obj)
        return NULL;

    obj->ob_type = &me_type_float;
    obj->ob_refcount = 1;

    char* endptr;
    double value = strtod(str, &endptr);
    if (*endptr != '\0') {
        free(obj);
        return NULL;
    }

    obj->ob_value = value;

    return (MEObject*)obj;
}

static void float_dealloc(MEFloatObject* obj) {
    free(obj);
}

static MEObject* float_str(MEFloatObject* obj) {
    return me_str_from_double(obj->ob_value);
}

METypeObject me_type_float = {
    .tp_name = "float",
    .tp_sizeof = sizeof(MEFloatObject),
    .tp_dealloc = (fn_destructor)float_dealloc,
    .tp_str = (fn_str)float_str,
};