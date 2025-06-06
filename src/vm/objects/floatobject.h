#ifndef __FLOATOBJECT_H
#define __FLOATOBJECT_H

#include "../object.h"

extern METypeObject me_type_float;

typedef struct {
    ME_OBJHEAD
    double ob_value;
} MEFloatObject;

MEObject* me_float_from_double(double value);
MEObject* me_float_from_long(long value);
MEObject* me_float_from_ulong(unsigned long value);
MEObject* me_float_from_str(const char* str);

#endif