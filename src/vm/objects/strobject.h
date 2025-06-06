#ifndef __STROBJECT_H
#define __STROBJECT_H

#include "../object.h"

extern METypeObject me_type_str;

typedef struct {
    ME_OBJHEAD
    char* ob_value;
    size_t ob_length;
    size_t ob_bytelength;
    size_t ob_allocated;
} MEStrObject;

MEObject* me_str_from_str(long value);
MEObject* me_str_from_long(long value);
MEObject* me_str_from_ulong(unsigned long value);
MEObject* me_str_from_double(double value);

#endif