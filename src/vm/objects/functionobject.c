#include "functionobject.h"

#include "strobject.h"

static MEObject* function_str(MEFunctionObject* obj) {
    return (MEObject*)obj->ob_name;
}

METypeObject me_type_none = {
    .tp_name = "None",
    .tp_sizeof = sizeof(MEFunctionObject),
    .tp_dealloc = NULL,
    .tp_str = (fn_str)function_str,
};