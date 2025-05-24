#ifndef __STR_H
#define __STR_H

#include <stddef.h>
#include <string.h>

#define EMPTY_STRV (StringView){NULL, 0}

typedef struct {
    const char* data;
    size_t len;
    size_t byte_len;
} StringView;

inline int strv_eq(StringView a, StringView b) {
    if (a.len != b.len) return 0;
    return a.byte_len == b.byte_len && memcmp(a.data, b.data, a.byte_len) == 0;
}

#endif