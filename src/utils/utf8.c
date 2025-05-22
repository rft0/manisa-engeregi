#include "utf8.h"

#include <stdint.h>

#define UTF8_ERROR 0

typedef struct {
    uint32_t start;
    uint32_t end;
} utf8_range;

static const utf8_range alpha_ranges[] = {
    {0x0041, 0x005A},   // A-Z
    {0x0061, 0x007A},   // a-z
    {0x00C0, 0x00D6},   // Latin-1 uppercase accented
    {0x00D8, 0x00F6},   // Latin-1 letters
    {0x00F8, 0x00FF},   // Latin-1 letters
    {0x0100, 0x017F},   // Latin Extended-A
    {0x0370, 0x03FF},   // Greek and Coptic
    {0x0400, 0x04FF},   // Cyrillic
    {0x0531, 0x058F},   // Armenian
    {0x0590, 0x05FF},   // Hebrew
    {0x0600, 0x06FF},   // Arabic
    {0x0700, 0x074F},   // Syriac
    {0x0780, 0x07BF},   // Thaana
    {0x0900, 0x097F},   // Devanagari
    {0x0981, 0x09FF},   // Bengali
    {0x0A00, 0x0A7F},   // Gurmukhi
    {0x0A80, 0x0AFF},   // Gujarati
    {0x0B00, 0x0B7F},   // Oriya
    {0x0B81, 0x0BFF},   // Tamil
    {0x0C00, 0x0C7F},   // Telugu
    {0x0C80, 0x0CFF},   // Kannada
};

static const utf8_range digit_ranges[] = {
    {0x0030, 0x0039},   // 0-9
    {0x0660, 0x0669},   // Arabic-Indic digits
    {0x06F0, 0x06F9},   // Extended Arabic-Indic digits
    {0x0966, 0x096F},   // Devanagari digits
    {0x09E6, 0x09EF},   // Bengali digits
    {0x0A66, 0x0A6F},   // Gurmukhi digits
    {0x0AE6, 0x0AEF},   // Gujarati digits
    {0x0B66, 0x0B6F},   // Oriya digits
    {0x0BE6, 0x0BEF},   // Tamil digits
    {0x0C66, 0x0C6F},   // Telugu digits
    {0xFF10, 0xFF19}    // Fullwidth digits
};

size_t utf8_csize(const char* c) {
    if ((c[0] & 0x80) == 0)
        return 1;
    else if ((c[0] & 0xE0) == 0xC0)
        return 2;
    else if ((c[0] & 0xF0) == 0xE0)
        return 3;
    else if ((c[0] & 0xF8) == 0xF0)
        return 4;

    return 1;
}

size_t utf8_strsize(const char* str) {
    size_t size = 0;
    while (*str) {
        size_t cs = utf8_csize(str);
        size += cs;
        str += cs;
    }

    return size;
}

size_t utf8_strlen(const char* str) {
    size_t len = 0;
    while (*str) {
        size_t cs = utf8_csize(str);
        len++;
        str += cs;
    }

    return len;
}

uint32_t utf8_codepoint(const char* str) {
    unsigned char c = (unsigned char) *str;
    if ((c & 0x80) == 0) {
        return c;
    } else if ((c & 0xE0) == 0xC0) {
        return ((c & 0x1F) << 6) | (str[1] & 0x3F); // 2-byte character
    } else if ((c & 0xF0) == 0xE0) {
        return ((c & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F); // 3-byte character
    } else if ((c & 0xF8) == 0xF0) {
        return ((c & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F); // 4-byte character
    }

    return UTF8_ERROR;
}

typedef void (*utf8_callback_t)(uint32_t codepoint, size_t index, void* user_data);
void utf8_iterate(const char* str, utf8_callback_t callback, void* user_data) {
    size_t index = 0;
    while (*str) {
        uint32_t codepoint = utf8_codepoint(str);
        if (codepoint == UTF8_ERROR)
            break;

        callback(codepoint, index, user_data);
        str += utf8_csize(str);
        index++;
    }
}

int utf8_isvalid(const char* str) {
    while (*str) {
        uint32_t codepoint = utf8_codepoint(str);
        if (codepoint == UTF8_ERROR)
            return 0;

        str += utf8_csize(str);
    }

    return 1; // Valid UTF-8
}

static int utf8_in_range(uint32_t cp, const utf8_range *ranges, size_t count) {
    size_t left = 0, right = count;

    while (left < right) {
        size_t mid = left + (right - left) / 2;
        if (cp < ranges[mid].start) {
            right = mid;
        } else if (cp > ranges[mid].end) {
            left = mid + 1;
        } else {
            return 1;
        }
    }
    return 0;
}

int utf8_isalpha(const char* c) {
    return utf8_in_range(utf8_codepoint(c), alpha_ranges, sizeof(alpha_ranges) / sizeof(utf8_range));
}

int utf8_isdigit(const char* c) {
    return utf8_in_range(utf8_codepoint(c), digit_ranges, sizeof(digit_ranges) / sizeof(utf8_range));
}

int utf8_isalnum(const char* c) {
    return utf8_isalpha(c) || utf8_isdigit(c);
}

