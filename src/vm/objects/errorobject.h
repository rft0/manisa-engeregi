#ifndef __ERROROBJECT_H
#define __ERROROBJECT_H

#include "../object.h"

void me_set_error(MEObject* error, const char* fmt, ...);
MEObject* me_get_error_obj();
const char* me_get_error_msg();

extern MEObject* me_error_divisionbyzero;
extern MEObject* me_error_typemismatch;
extern MEObject* me_error_notimplemented;
extern MEObject* me_error_outofmemory;

#endif