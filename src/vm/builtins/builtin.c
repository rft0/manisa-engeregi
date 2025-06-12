#include "builtin.h"

#include "io.h"
#include "cast.h"

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

#define ME_BUILTIN_CAST_INT      "tamsayı"
#define ME_BUILTIN_CAST_FLOAT    "ondalık"
#define ME_BUILTIN_CAST_STR      "cümle"
#define ME_BUILTIN_CAST_BOOL     "doğruluk"

#define REGISTER_BUILTIN_CO(co, name, func) do { \
    uintptr_t idx = darray_size((co)->co_globals); \
    hashmap_set((co)->co_h_globals, hashmap_lit_str(name), (uintptr_t)idx); \
    darray_pushd((co)->co_globals, me_builtinfn_new(name, func)); \
} while(0)

#define REGISTER_BUILTIN_ANALYZER(analyser, name, num_args) do { \
    Symbol* sym = symbol_new(strv_from_cstr(name), 0, 0, 0); \
    sym->nargs = num_args; \
    sym->is_initialized = 1; \
    scope_define((analyser)->current_scope, sym); \
} while(0)

StringView strv_from_cstr(const char* str) {
    StringView view;
    view.data = str;
    view.len = strlen(str);
    view.byte_len = strlen(str);
    return view;
}

void me_register_builtins_co(MECodeObject* co) {
    REGISTER_BUILTIN_CO(co, ME_BUILTIN_IO_PRINT, me_io_print);
    REGISTER_BUILTIN_CO(co, ME_BUILTIN_IO_INPUT, me_io_input);
    REGISTER_BUILTIN_CO(co, ME_BUILTIN_IO_OPEN, me_io_open);
    REGISTER_BUILTIN_CO(co, ME_BUILTIN_IO_CLOSE, me_io_close);
    REGISTER_BUILTIN_CO(co, ME_BUILTIN_IO_READ, me_io_read);
    REGISTER_BUILTIN_CO(co, ME_BUILTIN_IO_WRITE, me_io_write);
    REGISTER_BUILTIN_CO(co, ME_BUILTIN_IO_FLUSH, me_io_flush);

    REGISTER_BUILTIN_CO(co, ME_BUILTIN_CAST_INT, me_typecast_int);
    REGISTER_BUILTIN_CO(co, ME_BUILTIN_CAST_FLOAT, me_typecast_float);
    REGISTER_BUILTIN_CO(co, ME_BUILTIN_CAST_STR, me_typecast_str);
    REGISTER_BUILTIN_CO(co, ME_BUILTIN_CAST_BOOL, me_typecast_bool);
}

void me_register_builtins_analyser(Analyser* analyser) {
    REGISTER_BUILTIN_ANALYZER(analyser, ME_BUILTIN_IO_PRINT, 1);
    REGISTER_BUILTIN_ANALYZER(analyser, ME_BUILTIN_IO_INPUT, -1);
    REGISTER_BUILTIN_ANALYZER(analyser, ME_BUILTIN_IO_OPEN, 2);
    REGISTER_BUILTIN_ANALYZER(analyser, ME_BUILTIN_IO_CLOSE, 1);
    REGISTER_BUILTIN_ANALYZER(analyser, ME_BUILTIN_IO_READ, 2);
    REGISTER_BUILTIN_ANALYZER(analyser, ME_BUILTIN_IO_WRITE, 2);
    REGISTER_BUILTIN_ANALYZER(analyser, ME_BUILTIN_IO_FLUSH, 1);

    REGISTER_BUILTIN_ANALYZER(analyser, ME_BUILTIN_CAST_INT, 1);
    REGISTER_BUILTIN_ANALYZER(analyser, ME_BUILTIN_CAST_FLOAT, 1);
    REGISTER_BUILTIN_ANALYZER(analyser, ME_BUILTIN_CAST_STR, 1);
    REGISTER_BUILTIN_ANALYZER(analyser, ME_BUILTIN_CAST_BOOL, 1);
}