#include "longobject.h"

#include <stdlib.h>

#include "strobject.h"
#include "boolobject.h"

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

static MEObject* long_bool(MELongObject* obj) {
    return obj->ob_value ? me_true : me_false;
}

static MEObject* long_cmp(MELongObject* v, MELongObject* w, MECmpOp op) {
    switch (op) {
        case ME_CMP_EQ: return v->ob_value == w->ob_value ? me_true : me_false;
        case ME_CMP_NEQ: return v->ob_value != w->ob_value ? me_true : me_false;
        case ME_CMP_LT: return v->ob_value < w->ob_value ? me_true : me_false;
        case ME_CMP_LTE: return v->ob_value <= w->ob_value ? me_true : me_false;
        case ME_CMP_GT: return v->ob_value > w->ob_value ? me_true : me_false;
        case ME_CMP_GTE: return v->ob_value >= w->ob_value ? me_true : me_false;
    }
}

METypeObject me_type_long = {
    .tp_name = "long",
    .tp_sizeof = sizeof(MELongObject),
    .tp_dealloc = (fn_destructor)long_dealloc,
    .tp_str = (fn_str)long_str,
    .tp_bool = (fn_bool)long_bool,
    .tp_call = NULL,

    .tp_nb_add = NULL,
    .tp_nb_sub = NULL,
    .tp_nb_mul = NULL,
    .tp_nb_div = NULL,
    .tp_nb_mod = NULL,
    .tp_nb_bit_and = NULL,
    .tp_nb_bit_or = NULL,
    .tp_nb_bit_xor = NULL,
    .tp_nb_bit_not = NULL,
    .tp_nb_lshift = NULL,
    .tp_nb_rshift = NULL,

    .tp_cmp = (fn_cmp)long_cmp,
};