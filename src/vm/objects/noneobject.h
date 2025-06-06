#ifndef __NONEOBJECT_H
#define __NONEOBJECT_H

#include "../object.h"

extern METypeObject me_type_none;
extern MEObject* me_none;

typedef struct {
    ME_OBJHEAD
} MENoneObject;

#endif