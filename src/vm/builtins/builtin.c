#include "builtin.h"

#include "io.h"

#include "../objects/builtinfnobject.h"

#include "../../utils/hashmap.h"
#include "../../utils/darray.h"
#include "../../utils/str.h"

#define ME_BUILTIN_IO_PRINT     "çıktı"
#define ME_BUILTIN_IO_INPUT     "girdi"
#define ME_BUILTIN_IO_OPEN      "aç"
#define ME_BUILTIN_IO_CLOSE     "kapat"
#define ME_BUILTIN_IO_READ      "oku"
#define ME_BUILTIN_IO_WRITE     "yaz"
#define ME_BUILTIN_IO_FLUSH     "temizle"

StringView strv_from_cstr(const char* str) {
    StringView view;
    view.data = str;
    view.len = strlen(str);
    view.byte_len = strlen(str);
    return view;
}

void me_register_builtins_co(MECodeObject* co) {
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

void me_register_builtins_analyser(Analyser* analyser) {
    Symbol* print_sym = symbol_new(strv_from_cstr(ME_BUILTIN_IO_PRINT), 0, 0, 0);
    print_sym->nargs = 1;
    print_sym->is_initialized = 1;
    scope_define(analyser->current_scope, print_sym);
    
    Symbol* input_sym = symbol_new(strv_from_cstr(ME_BUILTIN_IO_INPUT), 0, 0, 0);
    input_sym->nargs = 0;
    input_sym->is_initialized = 1;
    scope_define(analyser->current_scope, input_sym);
    
    Symbol* open_sym = symbol_new(strv_from_cstr(ME_BUILTIN_IO_OPEN), 0, 0, 0);
    open_sym->nargs = 2;
    open_sym->is_initialized = 1;
    scope_define(analyser->current_scope, open_sym);
    
    Symbol* close_sym = symbol_new(strv_from_cstr(ME_BUILTIN_IO_CLOSE), 0, 0, 0);
    close_sym->nargs = 1;
    close_sym->is_initialized = 1;
    scope_define(analyser->current_scope, close_sym);
    
    Symbol* read_sym = symbol_new(strv_from_cstr(ME_BUILTIN_IO_READ), 0, 0, 0);
    read_sym->nargs = 2;
    read_sym->is_initialized = 1;
    scope_define(analyser->current_scope, read_sym);
    
    Symbol* write_sym = symbol_new(strv_from_cstr(ME_BUILTIN_IO_WRITE), 0, 0, 0);
    write_sym->nargs = 2;
    write_sym->is_initialized = 1;
    scope_define(analyser->current_scope, write_sym);
    
    Symbol* flush_sym = symbol_new(strv_from_cstr(ME_BUILTIN_IO_FLUSH), 0, 0, 0);
    flush_sym->nargs = 1;
    flush_sym->is_initialized = 1;
    scope_define(analyser->current_scope, flush_sym);
}