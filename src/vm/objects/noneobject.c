#include "noneobject.h"

#include "errorobject.h"
#include "strobject.h"
#include "boolobject.h"

static MENoneObject me_none_instance = {
    .ob_type = &me_type_none,
    .ob_refcount = 1,
};

MEObject* me_none = (MEObject*)&me_none_instance;

static MEObject* none_str(MENoneObject* obj) {
    return me_str_from_str("None");
}

static MEObject* none_bool(MENoneObject* obj) {
    return me_false;
}

static MEObject* none_cmp(MENoneObject* v, MENoneObject* w, MECmpOp op) {
    switch (op) {
        case ME_CMP_EQ: return v == w ? me_true : me_false;
        case ME_CMP_NEQ: return v != w ? me_true : me_false;
        default:
            return me_error_notimplemented;
    }
}

METypeObject me_type_none = {
    .tp_name = "none",
    .tp_base = NULL,
    .tp_sizeof = sizeof(MENoneObject),
    .tp_dealloc = NULL,
    .tp_str = (fn_str)none_str,
    .tp_bool = (fn_bool)none_bool,
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

    .tp_unary_negative = NULL,
    .tp_unary_positive = NULL,
    .tp_unary_bit_not = NULL,

    .tp_cmp = (fn_cmp)none_cmp,
};