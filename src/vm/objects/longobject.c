#include "longobject.h"

#include <stdlib.h>

#include "strobject.h"
#include "boolobject.h"
#include "errorobject.h"

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

MEObject* long_cmp(MELongObject* v, MELongObject* w, MECmpOp op) {
    switch (op) {
        case ME_CMP_EQ: return v->ob_value == w->ob_value ? me_true : me_false;
        case ME_CMP_NEQ: return v->ob_value != w->ob_value ? me_true : me_false;
        case ME_CMP_LT: return v->ob_value < w->ob_value ? me_true : me_false;
        case ME_CMP_LTE: return v->ob_value <= w->ob_value ? me_true : me_false;
        case ME_CMP_GT: return v->ob_value > w->ob_value ? me_true : me_false;
        case ME_CMP_GTE: return v->ob_value >= w->ob_value ? me_true : me_false;
    }
}

MEObject* long_nb_add(MEObject* v, MEObject* w) {
    if (!me_long_check(v) || !me_long_check(w))
        return me_error_notimplemented;

    MELongObject* lv = (MELongObject*)v;
    MELongObject* lw = (MELongObject*)w;

    return me_long_from_long(lv->ob_value + lw->ob_value);
}

MEObject* long_nb_sub(MEObject* v, MEObject* w) {
    if (!me_long_check(v) || !me_long_check(w))
        return me_error_notimplemented;

    MELongObject* lv = (MELongObject*)v;
    MELongObject* lw = (MELongObject*)w;

    return me_long_from_long(lv->ob_value - lw->ob_value);
}

MEObject* long_nb_mul(MEObject* v, MEObject* w) {
    if (!me_long_check(v) || !me_long_check(w))
        return me_error_notimplemented;

    MELongObject* lv = (MELongObject*)v;
    MELongObject* lw = (MELongObject*)w;

    return me_long_from_long(lv->ob_value * lw->ob_value);
}

MEObject* long_nb_div(MEObject* v, MEObject* w) {
    if (!me_long_check(v) || !me_long_check(w))
        return me_error_notimplemented;

    MELongObject* lv = (MELongObject*)v;
    MELongObject* lw = (MELongObject*)w;

    if (lw->ob_value == 0)
        return me_error_divisionbyzero;

    return me_long_from_long(lv->ob_value / lw->ob_value);
}

MEObject* long_nb_mod(MEObject* v, MEObject* w) {
    if (!me_long_check(v) || !me_long_check(w))
        return me_error_notimplemented;

    MELongObject* lv = (MELongObject*)v;
    MELongObject* lw = (MELongObject*)w;

    if (lw->ob_value == 0)
        return me_error_divisionbyzero;

    return me_long_from_long(lv->ob_value % lw->ob_value);
}

MEObject* long_nb_bit_and(MEObject* v, MEObject* w) {
    if (!me_long_check(v) || !me_long_check(w))
        return me_error_notimplemented;

    MELongObject* lv = (MELongObject*)v;
    MELongObject* lw = (MELongObject*)w;

    return me_long_from_long(lv->ob_value & lw->ob_value);
}

MEObject* long_nb_bit_or(MEObject* v, MEObject* w) {
    if (!me_long_check(v) || !me_long_check(w))
        return me_error_notimplemented;

    MELongObject* lv = (MELongObject*)v;
    MELongObject* lw = (MELongObject*)w;

    return me_long_from_long(lv->ob_value | lw->ob_value);
}

MEObject* long_nb_bit_xor(MEObject* v, MEObject* w) {
    if (!me_long_check(v) || !me_long_check(w))
        return me_error_notimplemented;

    MELongObject* lv = (MELongObject*)v;
    MELongObject* lw = (MELongObject*)w;

    return me_long_from_long(lv->ob_value ^ lw->ob_value);
}

MEObject* long_nb_bit_not(MEObject* v) {
    if (!me_long_check(v))
        return me_error_notimplemented;

    MELongObject* lv = (MELongObject*)v;

    return me_long_from_long(~lv->ob_value);
}

MEObject* long_nb_lshift(MEObject* v, MEObject* w) {
    if (!me_long_check(v) || !me_long_check(w))
        return me_error_notimplemented;

    MELongObject* lv = (MELongObject*)v;
    MELongObject* lw = (MELongObject*)w;

    return me_long_from_long(lv->ob_value << lw->ob_value);
}

MEObject* long_nb_rshift(MEObject* v, MEObject* w) {
    if (!me_long_check(v) || !me_long_check(w))
        return me_error_notimplemented;

    MELongObject* lv = (MELongObject*)v;
    MELongObject* lw = (MELongObject*)w;

    return me_long_from_long(lv->ob_value >> lw->ob_value);
}

METypeObject me_type_long = {
    .tp_name = "long",
    .tp_base = NULL,
    .tp_sizeof = sizeof(MELongObject),
    .tp_dealloc = (fn_destructor)long_dealloc,
    .tp_str = (fn_str)long_str,
    .tp_bool = (fn_bool)long_bool,
    .tp_call = NULL,

    .tp_nb_add = (fn_nb_add)long_nb_add,
    .tp_nb_sub = (fn_nb_sub)long_nb_sub,
    .tp_nb_mul = (fn_nb_mul)long_nb_mul,
    .tp_nb_div = (fn_nb_div)long_nb_div,
    .tp_nb_mod = (fn_nb_mod)long_nb_mod,
    .tp_nb_bit_and = (fn_nb_bit_and)long_nb_bit_and,
    .tp_nb_bit_or = (fn_nb_bit_or)long_nb_bit_or,
    .tp_nb_bit_xor = (fn_nb_bit_xor)long_nb_bit_xor,
    .tp_nb_bit_not = (fn_nb_bit_not)long_nb_bit_not,
    .tp_nb_lshift = (fn_nb_lshift)long_nb_lshift,
    .tp_nb_rshift = (fn_nb_rshift)long_nb_rshift,

    .tp_cmp = (fn_cmp)long_cmp,
};