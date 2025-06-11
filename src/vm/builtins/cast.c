#include "cast.h"

#include <stdlib.h>
#include <string.h>

#include "../objects/floatobject.h"
#include "../objects/errorobject.h"
#include "../objects/longobject.h"
#include "../objects/boolobject.h"
#include "../objects/noneobject.h"
#include "../objects/strobject.h"

#include "../../utils/darray.h"

MEObject* me_typecast_int(MEObject* self, MEObject** args) {
    if (!args || darray_size(args) != 1) {
        me_set_error(me_error_typemismatch, "int() expects one argument");
        return NULL;
    }

    MEObject* obj = args[0];

    if (me_long_check(obj)) {
        return obj;
    }
    
    if (me_float_check(obj)) {
        MEFloatObject* float_obj = (MEFloatObject*)obj;
        return me_long_from_long((long)float_obj->ob_value);
    }
    
    if (me_str_check(obj)) {
        MEStrObject* str_obj = (MEStrObject*)obj;
        char* null_terminated_str = (char*)malloc(str_obj->ob_bytelength + 1);
        if (!null_terminated_str) {
            me_set_error(me_error_outofmemory, "Memory allocation failed for string conversion");
            return NULL;
        }

        memcpy(null_terminated_str, str_obj->ob_value, str_obj->ob_bytelength);
        null_terminated_str[str_obj->ob_bytelength] = '\0';

        char* endptr;
        long value = strtol(null_terminated_str, &endptr, 10);
        if (*endptr != '\0') {
            me_set_error(me_error_typemismatch, "Cannot convert string \"%.*s\" to integer", str_obj->ob_bytelength, str_obj->ob_value);
            return NULL;
        }
        
        free(null_terminated_str);
    
        return me_long_from_long(value);
    }
    
    if (me_bool_check(obj)) {
        return me_long_from_long(((MEBoolObject*)obj)->ob_value);
    }
    
    me_set_error(me_error_typemismatch, "Cannot cast \"%s\" to integer", ME_TYPE_NAME(obj));
    return NULL;
}

MEObject* me_typecast_float(MEObject* self, MEObject** args) {
    if (!args || darray_size(args) != 1) {
        me_set_error(me_error_typemismatch, "float() expects one argument");
        return NULL;
    }

    MEObject* obj = args[0];

    if (me_float_check(obj)) {
        return obj;
    }
    
    if (me_long_check(obj)) {
        MELongObject* long_obj = (MELongObject*)obj;
        return me_float_from_double((double)long_obj->ob_value);
    }
    
    if (me_str_check(obj)) {
        MEStrObject* str_obj = (MEStrObject*)obj;
        char* null_terminated_str = (char*)malloc(str_obj->ob_bytelength + 1);
        if (!null_terminated_str) {
            me_set_error(me_error_outofmemory, "Memory allocation failed for string conversion");
            return NULL;
        }

        memcpy(null_terminated_str, str_obj->ob_value, str_obj->ob_bytelength);
        null_terminated_str[str_obj->ob_bytelength] = '\0';

        char* endptr;
        double value = strtod(null_terminated_str, &endptr);
        if (*endptr != '\0') {
            me_set_error(me_error_typemismatch, "Cannot convert string \"%.*s\" to float", str_obj->ob_bytelength, str_obj->ob_value);
            return NULL;
        }

        free(null_terminated_str);
        return me_float_from_double(value);
    }
    
    if (me_bool_check(obj)) {
        return me_float_from_double((double)((MEBoolObject*)obj)->ob_value);
    }
    
    me_set_error(me_error_typemismatch, "Cannot cast \"%s\" to float", ME_TYPE_NAME(obj));
    return NULL;
}

MEObject* me_typecast_str(MEObject* self, MEObject** args) {
    if (!args || darray_size(args) != 1) {
        me_set_error(me_error_typemismatch, "str() expects one argument");
        return NULL;
    }

    MEObject* obj = args[0];

    if (me_str_check(obj))
        return obj;
    
    if (ME_TYPE(obj)->tp_str)
        return ME_TYPE(obj)->tp_str(obj);
    
    me_set_error(me_error_typemismatch, "Cannot cast \"%s\" to string", ME_TYPE_NAME(obj));
    return NULL;
}

MEObject* me_typecast_bool(MEObject* self, MEObject** args) {
    if (!args || darray_size(args) != 1) {
        me_set_error(me_error_typemismatch, "bool() expects one argument");
        return NULL;
    }

    MEObject* obj = args[0];

    if (me_bool_check(obj))
        return obj;
    
    if (ME_TYPE(obj)->tp_bool)
        return ME_TYPE(obj)->tp_bool(obj);


    if (obj == me_none || obj == me_false)
        return me_false;

    return  me_true;
}