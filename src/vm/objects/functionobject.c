#include "functionobject.h"

static MEObject* function_str(MEFunctionObject* obj) {
    return (MEObject*)obj->ob_name;
}

METypeObject me_type_function = {
    .tp_name = "Function",
    .tp_sizeof = sizeof(MEFunctionObject),
    .tp_dealloc = NULL,
    .tp_str = (fn_str)function_str,
};