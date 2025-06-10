#include "strobject.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../utils/utf8.h"

#include "errorobject.h"
#include "boolobject.h"
#include "longobject.h"

MEObject* me_str_from_str(const char* str) {
    MEStrObject* obj = (MEStrObject*)malloc(sizeof(MEStrObject));
    if (!obj)
        return NULL;

    obj->ob_type = &me_type_str;
    obj->ob_refcount = 1;

    obj->ob_length = utf8_strlen(str);
    obj->ob_bytelength = utf8_strsize(str);

    obj->ob_value = (char*)malloc(obj->ob_bytelength);
    if (!obj->ob_value) {
        free(obj);
        return NULL;
    }

    memcpy(obj->ob_value, str, obj->ob_bytelength);

    return (MEObject*)obj;
}

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
    if (written < 0 || written >= 32) {
        free(obj->ob_value);
        free(obj);
        return NULL;
    }

    obj->ob_length = written;
    obj->ob_bytelength = written;

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
    if (written < 0 || written >= 32) {
        free(obj->ob_value);
        free(obj);
        return NULL;
    }

    obj->ob_length = written;
    obj->ob_bytelength = written;

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
    if (written < 0 || written >= 32) {
        free(obj->ob_value);
        free(obj);
        return NULL;
    }

    obj->ob_length = written;
    obj->ob_bytelength = written;

    return (MEObject*)obj;
}

static void str_dealloc(MEObject* obj) {
    free(((MEStrObject*)obj)->ob_value);
    free(obj);
}

static MEObject* str_str(MEObject* obj) {
    return obj;
}

static MEObject* str_bool(MEObject* obj) {
    return ((MEStrObject*)obj)->ob_bytelength > 0 ? me_true : me_false;
}

static MEObject* str_cmp(MEObject* v, MEObject* w, MECmpOp op) {
    if (!me_str_check(v) || !me_str_check(w)) {
        me_set_error(me_error_typemismatch, "Type mismatch in string comparison");
        return NULL;
    }

    int cmp = strcmp(((MEStrObject*)v)->ob_value, ((MEStrObject*)w)->ob_value);
    switch (op) {
        case ME_CMP_EQ: return cmp == 0 ? me_true : me_false;
        case ME_CMP_NEQ: return cmp != 0 ? me_true : me_false;
        case ME_CMP_LT: return cmp < 0 ? me_true : me_false;
        case ME_CMP_LTE: return cmp <= 0 ? me_true : me_false;
        case ME_CMP_GT: return cmp > 0 ? me_true : me_false;
        case ME_CMP_GTE: return cmp >= 0 ? me_true : me_false;
        default:
            return me_error_notimplemented;
    }
}

static MEObject* str_nb_add(MEObject* v, MEObject* w) {
    if (!me_str_check(v) || !me_str_check(w)) {
        me_set_error(me_error_typemismatch, "Type mismatch in string addition");
        return NULL;
    }

    MEStrObject* str_v = (MEStrObject*)v;
    MEStrObject* str_w = (MEStrObject*)w;

    MEStrObject* new_str = (MEStrObject*)malloc(sizeof(MEStrObject));
    if (!new_str) {
        me_set_error(me_error_outofmemory, "Out of memory while adding strings");
        return NULL;
    }

    new_str->ob_type = &me_type_str;
    new_str->ob_refcount = 1;
    new_str->ob_length = str_v->ob_length + str_w->ob_length;
    new_str->ob_bytelength = str_v->ob_bytelength + str_w->ob_bytelength;
    new_str->ob_value = (char*)malloc(new_str->ob_bytelength);
    if (!new_str->ob_value) {
        free(new_str);
        me_set_error(me_error_outofmemory, "Out of memory while adding strings");
        return NULL;
    }

    memcpy(new_str->ob_value, str_v->ob_value, str_v->ob_bytelength);
    memcpy(new_str->ob_value + str_v->ob_bytelength, str_w->ob_value, str_w->ob_bytelength);

    return (MEObject*)new_str;
}

static MEObject* str_nb_mul(MEObject* v, MEObject* w) {
    if (!me_str_check(v) || !me_long_check(w))
        return me_error_notimplemented;

    MEStrObject* str_v = (MEStrObject*)v;
    MELongObject* long_w = (MELongObject*)w;

    if (long_w->ob_value < 0)
        return me_str_from_str(""); 

    MEStrObject* new_str = (MEStrObject*)malloc(sizeof(MEStrObject));
    if (!new_str) {
        me_set_error(me_error_outofmemory, "Out of memory while multiplying string");
        return NULL;
    }

    new_str->ob_type = &me_type_str;
    new_str->ob_refcount = 1;
    new_str->ob_length = str_v->ob_length * long_w->ob_value;
    new_str->ob_bytelength = str_v->ob_bytelength * long_w->ob_value;
    new_str->ob_value = (char*)malloc(new_str->ob_bytelength);
    if (!new_str->ob_value) {
        free(new_str);
        me_set_error(me_error_outofmemory, "Out of memory while multiplying string");
        return NULL;
    }

    for (size_t i = 0; i < long_w->ob_value; i++)
        memcpy(new_str->ob_value + i * str_v->ob_bytelength, str_v->ob_value, str_v->ob_bytelength);

    return (MEObject*)new_str;
}

METypeObject me_type_str = {
    .tp_name = "str",
    .tp_base = NULL,
    .tp_sizeof = sizeof(MEStrObject),
    .tp_dealloc = (fn_destructor)str_dealloc,
    .tp_str = (fn_str)str_str,
    .tp_bool = (fn_bool)str_bool,
    .tp_call = NULL,

    .tp_nb_add = (fn_nb_add)str_nb_add,
    .tp_nb_sub = NULL,
    .tp_nb_mul = (fn_nb_mul)str_nb_mul,
    .tp_nb_div = NULL,
    .tp_nb_mod = NULL,
    .tp_nb_bit_and = NULL,
    .tp_nb_bit_or = NULL,
    .tp_nb_bit_xor = NULL,
    .tp_nb_lshift = NULL,
    .tp_nb_rshift = NULL,

    .tp_unary_negative = NULL,
    .tp_unary_positive = NULL,
    .tp_unary_bit_not = NULL,

    .tp_cmp = (fn_cmp)str_cmp,
};