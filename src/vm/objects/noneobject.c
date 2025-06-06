#include "noneobject.h"

#include "strobject.h"

static MENoneObject me_none_instance = {
    .ob_type = &me_type_none,
    .ob_refcount = 1,
};

MEObject* me_none = (MEObject*)&me_none_instance;

static MEObject* none_str(MENoneObject* obj) {
    return me_str_from_str("None");
}

METypeObject me_type_none = {
    .tp_name = "None",
    .tp_sizeof = sizeof(MENoneObject),
    .tp_dealloc = NULL,
    .tp_str = (fn_str)none_str,
};