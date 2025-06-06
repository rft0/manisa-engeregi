#include "noneobject.h"

#include "errorobject.h"
#include "longobject.h"
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

static MEObject* none_cmp(MELongObject* v, MELongObject* w, MECmpOp op) {
    switch (op) {
        case ME_CMP_EQ: return me_false;
        case ME_CMP_NEQ: return me_true;
        case ME_CMP_LT: return me_error_notimplemented;
        case ME_CMP_LTE: return me_error_notimplemented;
        case ME_CMP_GT: return me_error_notimplemented;
        case ME_CMP_GTE: return me_error_notimplemented;
    }
}

METypeObject me_type_none = {
    .tp_name = "none",
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
    .tp_nb_bit_not = NULL,
    .tp_nb_lshift = NULL,
    .tp_nb_rshift = NULL,

    .tp_cmp = NULL,
};