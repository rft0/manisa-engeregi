#include "floatobject.h"

#include <stdlib.h>
#include <math.h>

#include "errorobject.h"
#include "longobject.h"
#include "boolobject.h"
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

static MEObject* float_bool(MEFloatObject* obj) {
    return obj->ob_value != 0.0 ? me_true : me_false;
}

static MEObject* float_cmp(MEObject* v, MEObject* w, MECmpOp op) {
    double lhs, rhs;

    if (me_float_check(v))
        lhs = ((MEFloatObject*)v)->ob_value;
    else
        return me_error_notimplemented;

    if (me_float_check(w))
        rhs = ((MEFloatObject*)w)->ob_value;
    else if (me_long_check(w))
        rhs = (double)((MELongObject*)w)->ob_value;
    else
        return me_error_notimplemented;

    double lhv = ((MEFloatObject*)v)->ob_value;
    double rhv = ((MEFloatObject*)w)->ob_value;

    switch (op) {
        case ME_CMP_EQ: return lhv == rhv ? me_true : me_false;
        case ME_CMP_NEQ: return lhv != rhv ? me_true : me_false;
        case ME_CMP_LT: return lhv < rhv ? me_true : me_false;
        case ME_CMP_LTE: return lhv <= rhv ? me_true : me_false;
        case ME_CMP_GT: return lhv > rhv ? me_true : me_false;
        case ME_CMP_GTE: return lhv >= rhv ? me_true : me_false;
    }
}

static MEObject* float_nb_add(MEObject* v, MEObject* w) {
    if (me_float_check(w))
        return me_float_from_double(((MEFloatObject*)v)->ob_value + ((MEFloatObject*)w)->ob_value);
    else if (me_long_check(w))
        return me_float_from_double(((MEFloatObject*)v)->ob_value + (double)((MELongObject*)w)->ob_value);
    else
        return me_error_notimplemented;
}

static MEObject* float_nb_sub(MEObject* v, MEObject* w) {
    if (me_float_check(w))
        return me_float_from_double(((MEFloatObject*)v)->ob_value - ((MEFloatObject*)w)->ob_value);
    else if (me_long_check(w))
        return me_float_from_double(((MEFloatObject*)v)->ob_value - (double)((MELongObject*)w)->ob_value);
    else
        return me_error_notimplemented;
}

static MEObject* float_nb_mul(MEObject* v, MEObject* w) {
    if (me_float_check(w))
        return me_float_from_double(((MEFloatObject*)v)->ob_value * ((MEFloatObject*)w)->ob_value);
    else if (me_long_check(w))
        return me_float_from_double(((MEFloatObject*)v)->ob_value * (double)((MELongObject*)w)->ob_value);
    else
        return me_error_notimplemented;
}

static MEObject* float_nb_div(MEObject* v, MEObject* w) {
    if (me_float_check(w)) {
        if (((MEFloatObject*)w)->ob_value == 0.0) {
            me_set_error(me_error_divisionbyzero, "Division by zero in float division");
            return NULL;
        }
        return me_float_from_double(((MEFloatObject*)v)->ob_value / ((MEFloatObject*)w)->ob_value);
    } else if (me_long_check(w)) {
        if (((MELongObject*)w)->ob_value == 0) {
            me_set_error(me_error_divisionbyzero, "Division by zero in float division");
            return NULL;
        }
        return me_float_from_double(((MEFloatObject*)v)->ob_value / (double)((MELongObject*)w)->ob_value);
    } else {
        return me_error_notimplemented;
    }
}

static MEObject* float_nb_mod(MEObject* v, MEObject* w) {
    if (me_float_check(w)) {
        if (((MEFloatObject*)w)->ob_value == 0.0) {
            me_set_error(me_error_divisionbyzero, "Division by zero in modulo operation");
            return NULL;
        }
        return me_float_from_double(fmod(((MEFloatObject*)v)->ob_value, ((MEFloatObject*)w)->ob_value));
    } else if (me_long_check(w)) {
        if (((MELongObject*)w)->ob_value == 0) {
            me_set_error(me_error_divisionbyzero, "Division by zero in modulo operation");
            return NULL;
        }
        return me_float_from_double(fmod(((MEFloatObject*)v)->ob_value, (double)((MELongObject*)w)->ob_value));
    } else {
        return me_error_notimplemented;
    }
}

METypeObject me_type_float = {
    .tp_name = "float",
    .tp_base = NULL,
    .tp_sizeof = sizeof(MEFloatObject),
    .tp_dealloc = (fn_destructor)float_dealloc,
    .tp_str = (fn_str)float_str,
    .tp_bool = (fn_bool)float_bool,

    .tp_nb_add = (fn_nb_add)float_nb_add,
    .tp_nb_sub = (fn_nb_sub)float_nb_sub,
    .tp_nb_mul = (fn_nb_mul)float_nb_mul,
    .tp_nb_div = (fn_nb_div)float_nb_div,
    .tp_nb_mod = (fn_nb_mod)float_nb_mod,
    .tp_nb_bit_and = NULL,
    .tp_nb_bit_or = NULL,
    .tp_nb_bit_xor = NULL,
    .tp_nb_bit_not = NULL,
    .tp_nb_lshift = NULL,
    .tp_nb_rshift = NULL,

    .tp_cmp = (fn_cmp)float_cmp,
};