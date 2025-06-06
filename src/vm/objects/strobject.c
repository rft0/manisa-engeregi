#include "strobject.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../utils/utf8.h"

#include "boolobject.h"

MEObject* me_str_from_str(const char* str) {
    MEStrObject* obj = (MEStrObject*)malloc(sizeof(MEStrObject));
    if (!obj)
        return NULL;

    obj->ob_type = &me_type_str;
    obj->ob_refcount = 1;

    obj->ob_length = utf8_strlen(str);
    obj->ob_bytelength = utf8_strsize(str);

    obj->ob_value = (char*)malloc(obj->ob_bytelength + 1);
    if (!obj->ob_value) {
        free(obj);
        return NULL;
    }

    memcpy(obj->ob_value, str, obj->ob_bytelength);
    obj->ob_value[obj->ob_bytelength] = '\0';

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

static MEObject* str_bool(MEStrObject* obj) {
    return obj->ob_bytelength > 0 ? me_true : me_false;
}

static MEObject* str_cmp(MEStrObject* v, MEStrObject* w, MECmpOp op) {
    int cmp = strcmp(v->ob_value, w->ob_value);
    switch (op) {
        case ME_CMP_EQ: return cmp == 0 ? me_true : me_false;
        case ME_CMP_NEQ: return cmp != 0 ? me_true : me_false;
        case ME_CMP_LT: return cmp < 0 ? me_true : me_false;
        case ME_CMP_LTE: return cmp <= 0 ? me_true : me_false;
        case ME_CMP_GT: return cmp > 0 ? me_true : me_false;
        case ME_CMP_GTE: return cmp >= 0 ? me_true : me_false;
    }
}

METypeObject me_type_str = {
    .tp_name = "str",
    .tp_sizeof = sizeof(MEStrObject),
    .tp_dealloc = (fn_destructor)str_dealloc,
    .tp_str = (fn_str)str_str,
    .tp_bool = (fn_bool)str_bool,
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

    .tp_cmp = (fn_cmp)str_cmp,
};