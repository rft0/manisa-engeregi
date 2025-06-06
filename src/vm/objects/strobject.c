#include "strobject.h"

#include <stdlib.h>
#include <stdio.h>

MEObject* me_str_from_long(long value) {
    MEStrObject* obj = (MEStrObject*)malloc(sizeof(MEStrObject));
    if (!obj)
        return NULL;

    obj->ob_type = &me_type_str;
    obj->ob_refcount = 1;

    obj->ob_value = (char*)malloc(32);
    if (!obj->ob_value) {
        free(obj);
        return NULL;
    }

    int written = snprintf(obj->ob_value, 32, "%ld", value);
    if (written < 0 || written >= 31) {
        free(obj->ob_value);
        free(obj);
        return NULL;
    }

    obj->ob_length = written;
    obj->ob_bytelength = written;
    obj->ob_value[written] = '\0';

    return (MEObject*)obj;
}

MEObject* me_str_from_ulong(unsigned long value) {
    MEStrObject* obj = (MEStrObject*)malloc(sizeof(MEStrObject));
    if (!obj)
        return NULL;

    obj->ob_type = &me_type_str;
    obj->ob_refcount = 1;

    obj->ob_value = (char*)malloc(32);
    if (!obj->ob_value) {
        free(obj);
        return NULL;
    }

    int written = snprintf(obj->ob_value, 32, "%lu", value);
    if (written < 0 || written >= 31) {
        free(obj->ob_value);
        free(obj);
        return NULL;
    }

    obj->ob_length = written;
    obj->ob_bytelength = written;
    obj->ob_value[written] = '\0';

    return (MEObject*)obj;
}

MEObject* me_str_from_double(double value) {
    MEStrObject* obj = (MEStrObject*)malloc(sizeof(MEStrObject));
    if (!obj)
        return NULL;

    obj->ob_type = &me_type_str;
    obj->ob_refcount = 1;

    obj->ob_value = (char*)malloc(32);
    if (!obj->ob_value) {
        free(obj);
        return NULL;
    }

    int written = snprintf(obj->ob_value, 32, "%.2f", value);
    if (written < 0 || written >= 31) {
        free(obj->ob_value);
        free(obj);
        return NULL;
    }

    obj->ob_length = written;
    obj->ob_bytelength = written;
    obj->ob_value[written] = '\0';

    return (MEObject*)obj;
}

static void str_dealloc(MEStrObject* obj) {
    free(obj->ob_value);
    free(obj);
}

static MEObject* str_str(MEStrObject* obj) {
    return (MEObject*)obj;
}

METypeObject me_type_str = {
    .tp_name = "str",
    .tp_sizeof = sizeof(MEObject),
    .tp_dealloc = (fn_destructor)str_dealloc,
    .tp_str = (fn_str)str_str,
};