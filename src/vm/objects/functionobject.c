#include "functionobject.h"

#include "noneobject.h"
#include "boolobject.h"
#include "errorobject.h"

MEObject* me_function_new(MECodeObject* co, size_t nargs) {
    MEFunctionObject* obj = malloc(sizeof(MEFunctionObject));
    if (!obj) {
        me_set_error(me_error_outofmemory, "Failed to allocate memory for function object");
        return NULL;
    }

    obj->ob_type = &me_type_function;
    obj->ob_refcount = 1;

    obj->co = co;
    obj->nargs = nargs;

    return (MEObject*)obj;
}

static MEObject* function_str(MEFunctionObject* obj) {
    return me_str_from_str(obj->co->co_name);
}

static MEObject* function_bool(MEFunctionObject* obj) {
    return me_true;
}

static MEObject* function_call(MEFunctionObject* obj, MEObject** args, size_t nargs) {
    return me_none;
}

static MEObject* function_cmp(MEFunctionObject* v, MEFunctionObject* w, MECmpOp op) {
    switch (op) {
        case ME_CMP_EQ: return v == w ? me_true : me_false;
        case ME_CMP_NEQ: return v != w ? me_true : me_false;
        case ME_CMP_LT: return me_error_notimplemented;
        case ME_CMP_LTE: return me_error_notimplemented;
        case ME_CMP_GT: return me_error_notimplemented;
        case ME_CMP_GTE: return me_error_notimplemented;
        default:
            me_set_error(me_error_notimplemented, "Comparison operation not implemented for function");
            return NULL;
    }
}

METypeObject me_type_function = {
    .tp_name = "function",
    .tp_base = NULL,
    .tp_sizeof = sizeof(MEFunctionObject),
    .tp_dealloc = NULL,
    .tp_str = (fn_str)function_str,
    .tp_bool = (fn_bool)function_bool,
    .tp_call = (fn_call)function_call,

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

    .tp_cmp = (fn_cmp)function_cmp,
};