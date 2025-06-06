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

struct MEObject {
    ME_OBJHEAD
};

typedef void (*fn_destructor)(MEObject*);
typedef MEObject* (*fn_str)(MEObject*);

struct METypeObject {
    const char* tp_name;
    size_t tp_sizeof;
    fn_destructor tp_dealloc;
    fn_str tp_str;
    // Add more function pointers as needed (tp_add, tp_call, etc.)
};

#endif