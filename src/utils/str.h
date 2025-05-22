#ifndef __STR_H
#define __STR_H

#include <stddef.h>

#define EMPTY_STRV (StringView){NULL, 0}

typedef struct {
    const char* data;
    size_t len;
    size_t byte_len;
} StringView;

#endif