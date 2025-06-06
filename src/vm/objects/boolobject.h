#ifndef __BOOLOBJECT_H
#define __BOOLOBJECT_H

#include "../object.h"

extern METypeObject me_type_bool;

extern MEObject* me_true;
extern MEObject* me_false;

typedef struct {
    ME_OBJHEAD
    int ob_value;
} MEBoolObject;

MEObject* me_bool_from_double(double value);
MEObject* me_bool_from_long(long value);
MEObject* me_bool_from_ulong(unsigned long value);
MEObject* me_bool_from_str(MEObject* str);

#endif