#ifndef __BOOLOBJECT_H
#define __BOOLOBJECT_H

#include "../object.h"

extern METypeObject me_type_bool;

extern MEObject* me_true;
extern MEObject* me_false;

typedef struct {
    ME_OBJHEAD
    long ob_value;
} MEBoolObject;

static inline int me_bool_check(MEObject* obj) {
    return ME_TYPE_CHECK(obj, &me_type_bool);
}

void me_bool_init();

MEObject* me_bool_from_double(double value);
MEObject* me_bool_from_long(long value);
MEObject* me_bool_from_ulong(unsigned long value);
MEObject* me_bool_from_str(MEObject* str);

#endif