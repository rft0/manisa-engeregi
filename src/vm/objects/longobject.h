#ifndef __LONGOBJECT_H
#define __LONGOBJECT_H

#include "../object.h"

extern METypeObject me_type_long;

typedef struct {
    ME_OBJHEAD
    long ob_value;
} MELongObject;

inline int me_long_check(MEObject* obj) {
    return ME_TYPE_CHECK(obj, &me_type_long);
}

MEObject* me_long_from_long(long value);
MEObject* me_long_from_ulong(unsigned long value);
MEObject* me_long_from_str(const char* str);

#endif