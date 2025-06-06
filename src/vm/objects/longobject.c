#include "longobject.h"

#include <stdlib.h>

#include "strobject.h"

MEObject* me_long_from_long(long value) {
    MELongObject* obj = (MELongObject*)malloc(sizeof(MELongObject));
    if (!obj)
        return NULL;

    obj->ob_type = &me_type_long;
    obj->ob_refcount = 1;

    obj->ob_value = value;

    return (MEObject*)obj;
}

MEObject* me_long_from_ulong(unsigned long value) {
    MELongObject* obj = (MELongObject*)malloc(sizeof(MELongObject));
    if (!obj)
        return NULL;

    obj->ob_type = &me_type_long;
    obj->ob_refcount = 1;

    obj->ob_value = value;

    return (MEObject*)obj;
}

MEObject* me_long_from_str(const char* str) {
    MELongObject* obj = (MELongObject*)malloc(sizeof(MELongObject));
    if (!obj)
        return NULL;

    obj->ob_type = &me_type_long;
    obj->ob_refcount = 1;

    char* endptr;
    long value = strtol(str, &endptr, 10);
    if (*endptr != '\0') {
        free(obj);
        return NULL;
    }

    obj->ob_value = value;

    return (MEObject*)obj;
}

static void long_dealloc(MELongObject* obj) {
    free(obj);
}

static MEObject* long_str(MELongObject* obj) {
    return me_str_from_long(obj->ob_value);
}

METypeObject me_type_long = {
    .tp_name = "long",
    .tp_sizeof = sizeof(MELongObject),
    .tp_dealloc = (fn_destructor)long_dealloc,
    .tp_str = (fn_str)long_str,
};