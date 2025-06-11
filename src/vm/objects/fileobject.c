#include "fileobject.h"

#include <stdlib.h>
#include <string.h>

#include "errorobject.h"
#include "boolobject.h"
#include "strobject.h"


MEObject* me_file_new(FILE* file, const char* filename, const char* mode) {
    MEFileObject* obj = malloc(sizeof(MEFileObject));
    obj->ob_type = &me_file_type;
    obj->ob_file = file;
    obj->ob_filename = strdup(filename);
    obj->ob_mode = strdup(mode);
    obj->ob_closed = 0;
    return (MEObject*)obj;
}

void file_dealloc(MEObject* self) {
    MEFileObject* file_obj = (MEFileObject*)self;
    if (file_obj->ob_file && !file_obj->ob_closed)
        fclose(file_obj->ob_file);

    free(file_obj->ob_filename);
    free(file_obj->ob_mode);
    free(self);
}

static MEObject* file_str(MEObject* obj) {
    return me_str_from_str(((MEFileObject*)obj)->ob_filename);
}

static MEObject* file_bool(MEObject* obj) {
    return ((MEFileObject*)obj)->ob_file ? me_true : me_false;
}

static MEObject* file_cmp(MEObject* v, MEObject* w, MECmpOp op) {
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

METypeObject me_file_type = {
    .tp_name = "file",
    .tp_base = NULL,
    .tp_sizeof = sizeof(MEFileObject),
    .tp_dealloc = (fn_destructor)file_dealloc,
    .tp_str = (fn_str)file_str,
    .tp_bool = (fn_bool)file_bool,
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

    .tp_cmp = (fn_cmp)file_cmp,
};