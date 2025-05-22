#ifndef __UTF_8_H
#define __UTF_8_H

#include <stdint.h>
#include <stddef.h>

size_t utf8_csize(const char* c);
size_t utf8_strsize(const char* str);
size_t utf8_strlen(const char* str);

int utf8_isvalid(const char* str);

int utf8_isalpha(const char* c);
int utf8_isdigit(const char* c);
int utf8_isalnum(const char* c);

#endif