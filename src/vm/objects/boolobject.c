#include "boolobject.h"

#include <stdlib.h>

#include "strobject.h"
#include "longobject.h"

MEObject me_true_instance = {
    .ob_type = &me_type_bool,
    .ob_refcount = 1,
};

MEObject me_false_instance = {
    .ob_type = &me_type_bool,
    .ob_refcount = 1,
};

MEObject* me_true = &me_true_instance;
MEObject* me_false = &me_false_instance;

MEObject* me_bool_from_long(long value) {
    return value ? me_true : me_false;
}

MEObject* me_bool_from_ulong(unsigned long value) {
    return value ? me_true : me_false;
}

MEObject* me_bool_from_str(MEObject* str) {
    return (ME_TYPE_CHECK(str, &me_type_str) && ((MEStrObject*)str)->ob_length > 0) ? me_true : me_false;
}

static void bool_dealloc(MEBoolObject* obj) {
    free(obj);
}

static MEObject* bool_str(MEBoolObject* obj) {
    return me_str_from_long(obj->ob_value);
}

static MEObject* bool_bool(MEBoolObject* obj) {
    return obj->ob_value ? me_true : me_false;
}

METypeObject me_type_bool = {
    .tp_name = "bool",
    .tp_base = &me_type_long,
    .tp_sizeof = sizeof(MEBoolObject),
    .tp_dealloc = (fn_destructor)bool_dealloc,
    .tp_str = (fn_str)bool_str,
    .tp_bool = (fn_bool)bool_bool,
    .tp_call = NULL,

    .tp_nb_add = NULL,
    .tp_nb_sub = NULL,
    .tp_nb_mul = NULL,
    .tp_nb_div = NULL,
    .tp_nb_mod = NULL,
    .tp_nb_bit_and = NULL,
    .tp_nb_bit_or = NULL,
    .tp_nb_bit_xor = NULL,
    .tp_nb_lshift = NULL,
    .tp_nb_rshift = NULL,

    .tp_unary_bit_not = NULL,

    .tp_cmp = NULL,
};

void me_bool_init() {
    me_type_bool.tp_nb_add = me_type_long.tp_nb_add;
    me_type_bool.tp_nb_sub = me_type_long.tp_nb_sub;
    me_type_bool.tp_nb_mul = me_type_long.tp_nb_mul;
    me_type_bool.tp_nb_div = me_type_long.tp_nb_div;
    me_type_bool.tp_nb_mod = me_type_long.tp_nb_mod;
    me_type_bool.tp_nb_bit_and = me_type_long.tp_nb_bit_and;
    me_type_bool.tp_nb_bit_or = me_type_long.tp_nb_bit_or;
    me_type_bool.tp_nb_bit_xor = me_type_long.tp_nb_bit_xor;
    me_type_bool.tp_nb_lshift = me_type_long.tp_nb_lshift;
    me_type_bool.tp_nb_rshift = me_type_long.tp_nb_rshift;

    me_type_bool.tp_unary_bit_not = me_type_long.tp_unary_bit_not;

    me_type_bool.tp_cmp = me_type_long.tp_cmp;
}