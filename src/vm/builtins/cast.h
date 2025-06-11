#ifndef __CAST
#define __CAST

#include "../object.h"

MEObject* me_typecast_int(MEObject* self, MEObject** args);
MEObject* me_typecast_float(MEObject* self, MEObject** args);
MEObject* me_typecast_str(MEObject* self, MEObject** args);
MEObject* me_typecast_bool(MEObject* self, MEObject** args);

#endif