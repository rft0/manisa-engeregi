#ifndef __NONEOBJECT_H
#define __NONEOBJECT_H

#include "../object.h"

extern METypeObject me_type_none;
extern MEObject* me_none;

typedef struct {
    ME_OBJHEAD
} MENoneObject;

inline int me_none_check(MEObject* obj) {
    return ME_TYPE_CHECK(obj, &me_type_none);
}

#endif