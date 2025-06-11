#ifndef __FILEOBJECT_H
#define __FILEOBJECT_H

#include <stdio.h>
#include "../object.h"

extern METypeObject me_file_type;

typedef struct {
    ME_OBJHEAD
    FILE* ob_file;
    char* ob_mode;
    char* ob_filename;
    int ob_closed;
} MEFileObject;

static inline int me_file_check(MEObject* obj) {
    return ME_TYPE_CHECK(obj, &me_file_type);
}

MEObject* me_file_new(FILE* file, const char* filename, const char* mode);
void me_file_dealloc(MEObject* self);

#endif