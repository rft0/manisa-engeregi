#include "io.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../utils/darray.h"

#include "../objects/errorobject.h"
#include "../objects/strobject.h"
#include "../objects/noneobject.h"
#include "../objects/fileobject.h"
#include "../objects/longobject.h"

MEObject* me_io_print(MEObject* self, MEObject** args) {
    if (!args || darray_size(args) != 1) {
        me_set_error(me_error_typemismatch, "print() expects one argument");
        return NULL;
    }

    if (!me_str_check(args[0])) {
        if (ME_TYPE(args[0])->tp_str) {
            MEStrObject* str_obj = (MEStrObject*)ME_TYPE(args[0])->tp_str(args[0]);
            printf("%.*s\n", (int)str_obj->ob_bytelength, str_obj->ob_value);
            return me_none;
        } else {
            me_set_error(me_error_typemismatch, "print() expects a string argument");
            return NULL;
        }
    }

    MEStrObject* str_obj = (MEStrObject*)args[0];
    printf("%.*s\n", (int)str_obj->ob_bytelength, str_obj->ob_value);
    
    return me_none;
}

MEObject* me_io_input(MEObject* self, MEObject** args) {
    if (args && darray_size(args) > 1) {
        me_set_error(me_error_typemismatch, "input() does take zero or one argument");
        return NULL;
    }

    if (args && darray_size(args) == 1 && !me_str_check(args[0])) {
        me_set_error(me_error_typemismatch, "input() expects a string argument");
        return NULL;
    }
    
    if (args && darray_size(args) == 1) {
        MEStrObject* prompt_obj = (MEStrObject*)args[0];
        printf("%.*s", (int)prompt_obj->ob_bytelength, prompt_obj->ob_value);
    }

    size_t capacity = 256;
    size_t size = 0;
    int c;

    char* buffer = malloc(capacity);

    while ((c = getc(stdin)) != EOF && c != '\n') {
        buffer[size++] = c;
        if (size >= capacity - 1) {
            capacity *= 2;
            char* new_buffer = realloc(buffer, capacity);
            if (!new_buffer) {
                free(buffer);
                me_set_error(me_error_generic, "Failed to allocate memory for input");
                return NULL;
            }
            buffer = new_buffer;
        }
    }

    buffer[size] = '\0';

    MEObject* result = me_str_from_str(buffer);
    free(buffer);
    
    return result;
}

MEObject* me_io_open(MEObject* self, MEObject** args) {
    if (!args || darray_size(args) != 2) {
        me_set_error(me_error_typemismatch, "open() expects a filename and mode");
        return NULL;
    }

    if (!me_str_check(args[0]) || !me_str_check(args[1])) {
        me_set_error(me_error_typemismatch, "open() expects string arguments");
        return NULL;
    }

    MEStrObject* filename_obj = (MEStrObject*)args[0];
    MEStrObject* mode_obj = (MEStrObject*)args[1];
    
    char* filename = malloc(filename_obj->ob_bytelength + 1);
    char* mode = malloc(mode_obj->ob_bytelength + 1);
    
    memcpy(filename, filename_obj->ob_value, filename_obj->ob_bytelength);
    memcpy(mode, mode_obj->ob_value, mode_obj->ob_bytelength);
    
    filename[filename_obj->ob_bytelength] = '\0';
    mode[mode_obj->ob_bytelength] = '\0';
    
    FILE* file = fopen(filename, mode);
    if (!file) {
        free(filename);
        free(mode);
        me_set_error(me_error_generic, "Could not open file: %s", filename);
        return NULL;
    }
    
    MEObject* file_obj = me_file_new(file, filename, mode);
    free(filename);
    free(mode);
    
    return file_obj;
}

MEObject* me_io_close(MEObject* self, MEObject** args) {
    if (!args || darray_size(args) != 1) {
        me_set_error(me_error_typemismatch, "close() expects a file object");
        return NULL;
    }

    if (!me_file_check(args[0])) {
        me_set_error(me_error_typemismatch, "close() expects a file object");
        return NULL;
    }

    MEFileObject* file_obj = (MEFileObject*)args[0];
    
    if (file_obj->ob_closed) {
        me_set_error(me_error_generic, "File is already closed");
        return NULL;
    }

    int result = fclose(file_obj->ob_file);
    if (result != 0) {
        me_set_error(me_error_generic, "Error closing file");
        return NULL;
    }
    
    file_obj->ob_closed = 1;
    file_obj->ob_file = NULL;
    
    return me_none;
}

MEObject* me_io_read(MEObject* self, MEObject** args) {
    if (!args || darray_size(args) != 2) {
        me_set_error(me_error_typemismatch, "read() expects a file object and size");
        return NULL;
    }

    if (!me_file_check(args[0]) || !me_long_check(args[1])) {
        me_set_error(me_error_typemismatch, "read() expects a file object and integer size");
        return NULL;
    }

    MEFileObject* file_obj = (MEFileObject*)args[0];
    long size = ((MELongObject*)args[1])->ob_value;
    
    if (file_obj->ob_closed || !file_obj->ob_file) {
        me_set_error(me_error_generic, "File is closed");
        return NULL;
    }
    
    if (size == -1) {
        long current_pos = ftell(file_obj->ob_file);
        fseek(file_obj->ob_file, 0, SEEK_END);
        long file_size = ftell(file_obj->ob_file);
        
        fseek(file_obj->ob_file, current_pos, SEEK_SET);
        
        size = file_size - current_pos;
    } else if (size <= 0) {
        me_set_error(me_error_generic, "Invalid read size");
        return NULL;
    }
    
    char* buffer = malloc(size + 1);
    if (!buffer) {
        me_set_error(me_error_generic, "Failed to allocate memory for read");
        return NULL;
    }
    
    size_t bytes_read = fread(buffer, 1, size, file_obj->ob_file);
    buffer[bytes_read] = '\0';
    
    MEObject* result = me_str_from_str(buffer);
    free(buffer);
    
    return result;
}

MEObject* me_io_write(MEObject* self, MEObject** args) {
    if (!args || darray_size(args) != 2) {
        me_set_error(me_error_typemismatch, "write() expects a file object and string");
        return NULL;
    }

    if (!me_file_check(args[0]) || !me_str_check(args[1])) {
        me_set_error(me_error_typemismatch, "write() expects a file object and string");
        return NULL;
    }

    MEFileObject* file_obj = (MEFileObject*)args[0];
    MEStrObject* str_obj = (MEStrObject*)args[1];
    
    if (file_obj->ob_closed || !file_obj->ob_file) {
        me_set_error(me_error_generic, "File is closed");
        return NULL;
    }
    
    size_t bytes_written = fwrite(str_obj->ob_value, 1, str_obj->ob_bytelength, file_obj->ob_file);
    
    return me_long_from_long(bytes_written);
}

MEObject* me_io_flush(MEObject* self, MEObject** args) {
    if (!args || darray_size(args) != 1) {
        me_set_error(me_error_typemismatch, "flush() expects a file object");
        return NULL;
    }

    if (!me_file_check(args[0])) {
        me_set_error(me_error_typemismatch, "flush() expects a file object");
        return NULL;
    }

    MEFileObject* file_obj = (MEFileObject*)args[0];
    
    if (file_obj->ob_closed || !file_obj->ob_file) {
        me_set_error(me_error_generic, "File is closed");
        return NULL;
    }
    
    int result = fflush(file_obj->ob_file);
    if (result != 0) {
        me_set_error(me_error_generic, "Error flushing file");
        return NULL;
    }
    
    return me_none;
}