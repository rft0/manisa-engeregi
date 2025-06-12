#include "builtinfnobject.h"

#include <stdlib.h>

#include "errorobject.h"
#include "boolobject.h"
#include "strobject.h"

static MEObject* builtinfn_dealloc(MEObject* obj) {
    free(obj);
    return NULL;
}

static MEObject* builtinfn_str(MEObject* obj) {
    return me_str_from_str(((MEBuiltinFnObject*)obj)->ob_name);
}

static MEObject* builtinfn_bool(MEObject* obj) {
    return me_true;
}

static MEObject* builtinfn_cmp(MEObject* v, MEObject* w, MECmpOp op) {
    switch (op) {
        case ME_CMP_EQ: return v == w ? me_true : me_false;
        case ME_CMP_NEQ: return v != w ? me_true : me_false;
        case ME_CMP_LT: return me_error_notimplemented;
        case ME_CMP_LTE: return me_error_notimplemented;
        case ME_CMP_GT: return me_error_notimplemented;
        case ME_CMP_GTE: return me_error_notimplemented;
        default:
            return me_error_notimplemented;
    }
}


MEObject* me_builtinfn_new(const char* name, MEBuiltinFunction fn) {
    MEBuiltinFnObject* obj = malloc(sizeof(MEBuiltinFnObject));
    if (!obj) {
        me_set_error(me_error_generic, "Failed to allocate memory for builtin function object");
        return NULL;
    }
    
    obj->ob_type = &me_type_builtinfn;
    obj->ob_refcount = 1; // Initial reference count

    obj->ob_name = name;
    obj->fn = fn;

    ME_INCREF((MEObject*)obj);
    return (MEObject*)obj;
}

METypeObject me_type_builtinfn = {
    .tp_name = "builtin function",
    .tp_base = NULL,
    .tp_sizeof = sizeof(MEBuiltinFnObject),
    .tp_dealloc = (fn_destructor)builtinfn_dealloc,
    .tp_str = (fn_str)builtinfn_str,
    .tp_bool = (fn_bool)builtinfn_bool,
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

    .tp_cmp = (fn_cmp)builtinfn_cmp,
};