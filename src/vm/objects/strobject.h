#ifndef __STROBJECT_H
#define __STROBJECT_H

#include "../object.h"

extern METypeObject me_type_str;

typedef struct {
    ME_OBJHEAD
    char* ob_value;
    size_t ob_length;
    size_t ob_bytelength;
} MEStrObject;

inline int me_str_check(MEObject* obj) {
    return ME_TYPE_CHECK(obj, &me_type_str);
}

MEObject* me_str_from_str(const char* str);
MEObject* me_str_from_long(long value);
MEObject* me_str_from_ulong(unsigned long value);
MEObject* me_str_from_double(double value);

#endif