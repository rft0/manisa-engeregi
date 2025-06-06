#include "floatobject.h"

#include <stdlib.h>

#include "strobject.h"
#include "boolobject.h"

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

static MEObject* float_bool(MEFloatObject* obj) {
    return obj->ob_value != 0.0 ? me_true : me_false;
}

static MEObject* float_cmp(MEFloatObject* v, MEFloatObject* w, MECmpOp op) {
    switch (op) {
        case ME_CMP_EQ: return v->ob_value == w->ob_value ? me_true : me_false;
        case ME_CMP_NEQ: return v->ob_value != w->ob_value ? me_true : me_false;
        case ME_CMP_LT: return v->ob_value < w->ob_value ? me_true : me_false;
        case ME_CMP_LTE: return v->ob_value <= w->ob_value ? me_true : me_false;
        case ME_CMP_GT: return v->ob_value > w->ob_value ? me_true : me_false;
        case ME_CMP_GTE: return v->ob_value >= w->ob_value ? me_true : me_false;
    }
}

METypeObject me_type_float = {
    .tp_name = "float",
    .tp_sizeof = sizeof(MEFloatObject),
    .tp_dealloc = (fn_destructor)float_dealloc,
    .tp_str = (fn_str)float_str,
    .tp_bool = (fn_bool)float_bool,

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

    .tp_cmp = (fn_cmp)float_cmp,
};