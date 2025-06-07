#include "errorobject.h"

#include "strobject.h"

struct {
    const char* error_msg;
    MEObject* error_obj;
} me_global_error = {NULL, NULL}; 

METypeObject me_type_error_divisionbyzero;
METypeObject me_type_error_typemismatch;
METypeObject me_type_error_notimplemented;
METypeObject me_type_error_outofmemory;

void me_set_error(MEObject* error, const char* msg) {
    me_global_error.error_msg = msg;
    me_global_error.error_obj = error;
}

MEObject* me_get_error() {
    return me_global_error.error_obj;
}


struct {
    ME_OBJHEAD
} me_error_divisionbyzero_instance = {
    .ob_type = &me_type_error_typemismatch,
    .ob_refcount = 1,
};

struct {
    ME_OBJHEAD
} me_error_typemismatch_instance = {
    .ob_type = &me_type_error_typemismatch,
    .ob_refcount = 1,
};

struct {
    ME_OBJHEAD
} me_error_notimplemented_instance = {
    .ob_type = &me_type_error_notimplemented,
    .ob_refcount = 1,
};

struct {
    ME_OBJHEAD
} me_error_outofmemory_instance = {
    .ob_type = &me_type_error_notimplemented,
    .ob_refcount = 1,
};

MEObject* me_error_divisionbyzero = (MEObject*)&me_error_divisionbyzero_instance;
MEObject* me_error_typemismatch = (MEObject*)&me_error_typemismatch_instance;
MEObject* me_error_notimplemented = (MEObject*)&me_error_notimplemented_instance;

static MEObject* error_divisionbyzero_str(MEObject* obj) {
    return me_str_from_str("Division by zero error");
}

static MEObject* error_typemismatch_str(MEObject* obj) {
    return me_str_from_str("Type mismatch error");
}

static MEObject* error_notimplemented_str(MEObject* obj) {
    return me_str_from_str("Not implemented error");
}

static MEObject* error_outofmemory_str(MEObject* obj) {
    return me_str_from_str("Out of memory error");
}


METypeObject me_type_error_divisionbyzero = {
    .tp_name = "DivisonByZeroError",
    .tp_sizeof = sizeof(me_error_divisionbyzero_instance),
    .tp_dealloc = NULL,
    .tp_str = error_divisionbyzero_str,
    .tp_bool = NULL,
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
    .tp_cmp = NULL
};

METypeObject me_type_error_typemismatch = {
    .tp_name = "TypeMismatchError",
    .tp_sizeof = sizeof(me_error_typemismatch_instance),
    .tp_dealloc = NULL,
    .tp_str = error_typemismatch_str,
    .tp_bool = NULL,
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
    .tp_cmp = NULL
};

METypeObject me_type_error_notimplemented = {
    .tp_name = "NotImplementedError",
    .tp_sizeof = sizeof(me_error_notimplemented_instance),
    .tp_dealloc = NULL,
    .tp_str = error_notimplemented_str,
    .tp_bool = NULL,
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
    .tp_cmp = NULL
};

METypeObject me_type_error_outofmemory = {
    .tp_name = "OutOfMemoryError",
    .tp_base = NULL,
    .tp_sizeof = sizeof(me_error_outofmemory_instance),
    .tp_dealloc = NULL,
    .tp_str = error_outofmemory_str,
    .tp_bool = NULL,
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
    .tp_cmp = NULL
};