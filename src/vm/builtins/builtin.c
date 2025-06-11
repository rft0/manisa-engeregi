#include "builtin.h"

#include "io.h"

#include "../objects/builtinfnobject.h"

#include "../../utils/hashmap.h"
#include "../../utils/darray.h"

#define ME_BUILTIN_IO_PRINT     "print"
#define ME_BUILTIN_IO_INPUT     "input"
#define ME_BUILTIN_IO_OPEN      "open"
#define ME_BUILTIN_IO_CLOSE     "close"
#define ME_BUILTIN_IO_READ      "read"
#define ME_BUILTIN_IO_WRITE     "write"
#define ME_BUILTIN_IO_FLUSH     "flush"

void me_register_builtins(MECodeObject* co) {
    uintptr_t idx;
    
    idx = darray_size(co->co_globals);
    hashmap_set(co->co_h_globals, hashmap_lit_str(ME_BUILTIN_IO_PRINT), (uintptr_t)idx);
    darray_pushd(co->co_globals, me_builtinfn_new(ME_BUILTIN_IO_PRINT, me_io_print));
    
    idx = darray_size(co->co_globals);
    hashmap_set(co->co_h_globals, hashmap_lit_str(ME_BUILTIN_IO_INPUT), (uintptr_t)idx);
    darray_pushd(co->co_globals, me_builtinfn_new(ME_BUILTIN_IO_INPUT, me_io_input));
    
    idx = darray_size(co->co_globals);
    hashmap_set(co->co_h_globals, hashmap_lit_str(ME_BUILTIN_IO_OPEN), (uintptr_t)idx);
    darray_pushd(co->co_globals, me_builtinfn_new(ME_BUILTIN_IO_OPEN, me_io_open));
    
    idx = darray_size(co->co_globals);
    hashmap_set(co->co_h_globals, hashmap_lit_str(ME_BUILTIN_IO_CLOSE), (uintptr_t)idx);
    darray_pushd(co->co_globals, me_builtinfn_new(ME_BUILTIN_IO_CLOSE, me_io_close));
    
    idx = darray_size(co->co_globals);
    hashmap_set(co->co_h_globals, hashmap_lit_str(ME_BUILTIN_IO_READ), (uintptr_t)idx);
    darray_pushd(co->co_globals, me_builtinfn_new(ME_BUILTIN_IO_READ, me_io_read));
    
    idx = darray_size(co->co_globals);
    hashmap_set(co->co_h_globals, hashmap_lit_str(ME_BUILTIN_IO_WRITE), (uintptr_t)idx);
    darray_pushd(co->co_globals, me_builtinfn_new(ME_BUILTIN_IO_WRITE, me_io_write));
    
    idx = darray_size(co->co_globals);
    hashmap_set(co->co_h_globals, hashmap_lit_str(ME_BUILTIN_IO_FLUSH), (uintptr_t)idx);
    darray_pushd(co->co_globals, me_builtinfn_new(ME_BUILTIN_IO_FLUSH, me_io_flush));
}