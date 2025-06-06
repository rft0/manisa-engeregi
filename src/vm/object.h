#ifndef __OBJ_H
#define __OBJ_H

#include <stdint.h>
#include <stddef.h>

typedef struct METypeObject METypeObject;
typedef struct MEObject MEObject;

#define ME_INCREF(obj) do { if ((obj) != NULL) ++(obj)->ob_refcount; } while (0)
#define ME_DECREF(obj) do { if (--(obj)->ob_refcount == 0) (obj)->type->tp_dealloc(obj); } while (0)

#define ME_OBJHEAD size_t ob_refcount; METypeObject* ob_type;

#define ME_TYPE(obj) ((obj)->ob_type)
#define ME_TYPE_CHECK(obj, typeobject) ((obj)->ob_type == typeobject || (obj)->ob_type->tp_base == typeobject)

struct MEObject {
    ME_OBJHEAD
};

typedef enum {
    ME_CMP_EQ = 0,
    ME_CMP_NEQ,
    ME_CMP_LT,
    ME_CMP_LTE,
    ME_CMP_GT,
    ME_CMP_GTE,
} MECmpOp;

typedef void (*fn_destructor)(MEObject*);
typedef MEObject* (*fn_str)(MEObject*);
typedef MEObject* (*fn_bool)(MEObject*);
typedef MEObject* (*fn_call)(MEObject*, MEObject** args, size_t nargs);
typedef MEObject* (*fn_nb_add)(MEObject*, MEObject*);
typedef MEObject* (*fn_nb_sub)(MEObject*, MEObject*);
typedef MEObject* (*fn_nb_mul)(MEObject*, MEObject*);
typedef MEObject* (*fn_nb_div)(MEObject*, MEObject*);
typedef MEObject* (*fn_nb_mod)(MEObject*, MEObject*);
typedef MEObject* (*fn_nb_bit_and)(MEObject*, MEObject*);
typedef MEObject* (*fn_nb_bit_or)(MEObject*, MEObject*);
typedef MEObject* (*fn_nb_bit_xor)(MEObject*, MEObject*);
typedef MEObject* (*fn_nb_bit_not)(MEObject*);
typedef MEObject* (*fn_nb_lshift)(MEObject*, MEObject*);
typedef MEObject* (*fn_nb_rshift)(MEObject*, MEObject*);
typedef MEObject* (*fn_cmp)(MEObject*, MEObject*, MECmpOp);

struct METypeObject {
    const char* tp_name;
    METypeObject* tp_base;
    size_t tp_sizeof;
    fn_destructor tp_dealloc;
    fn_str tp_str;
    fn_bool tp_bool;
    fn_call tp_call;

    fn_nb_add tp_nb_add;
    fn_nb_sub tp_nb_sub;
    fn_nb_mul tp_nb_mul;
    fn_nb_div tp_nb_div;
    fn_nb_mod tp_nb_mod;
    fn_nb_bit_and tp_nb_bit_and;
    fn_nb_bit_or tp_nb_bit_or;
    fn_nb_bit_xor tp_nb_bit_xor;
    fn_nb_bit_not tp_nb_bit_not;
    fn_nb_lshift tp_nb_lshift;
    fn_nb_rshift tp_nb_rshift;

    fn_cmp tp_cmp;
};

void me_objects_init();

#endif