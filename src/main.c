#define ME_DEBUG

#include <stdlib.h>
#include <stdio.h>

#include "diag/diag.h"
#include "helpers.h"
#include "lut.h"

#include "utils/darray.h"
#include "utils/utf8.h"

#include "parser/lexer.h"
#include "parser/parser.h"
#include "parser/analyser.h"

#include "vm/objects/errorobject.h"
#include "vm/co.h"
#include "vm/vm.h"

int main(int argc, char *argv[]) {
// #ifndef ME_DEBUG
//     // COMMAND LINE HANDLING WILL BE DONE IN SEPERATE FILE LASTLY DO NOT ADD THINGS LIKE THAT HERE.
//     if (argc < 2) {
//         fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
//         return 1;
//     }

//     const char* filename = argv[1];
// #else
//     const char* filename = "deneme.me"; // For now let's assume "filename" is both utf-8 and null terminated string
// #endif

    // NEVERMIND I AM TIRED, NO COMPLEX COMMAND LINE HANDLING
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];

    char* src = read_file_binary(filename, NULL);
    if (!src) {
        fprintf(stderr, "Failed to read source file: %s\n", filename);
        return 1;
    }
    
    // Some editors add BOM at the beginning of the files with UTF-8 encoding
    if (src[0] == '\xEF' && src[1] == '\xBB' && src[2] == '\xBF')
        src += 3; // Skip BOM

    if (!utf8_isvalid(src)) {
        fprintf(stderr, "Invalid UTF-8 encoding\n");
        free(src);
        return 1;
    }

    diags_init();
    lut_init();

    Token** tokens = lex(filename, src);
#ifdef ME_DEBUG
    printf("Tokens:\n");
    darray_for(tokens) token_dump(tokens[__i]);
    printf("--------------------\n");
#endif

    Stmt** stmts = parse(filename, tokens);
#ifdef ME_DEBUG
    printf("Statements:\n");
    darray_for(stmts) stmt_dump(stmts[__i]);
    printf("--------------------\n");
#endif
    analyse(filename, stmts);

    if (diags_errs_size() > 0) {
        
        diags_dump();
        diags_free();
        lut_free();
        
        darray_for(tokens) free(tokens[__i]);
        darray_free(tokens);

        darray_for(stmts) free(stmts[__i]);
        darray_free(stmts);

        free(src);

        fprintf(stderr, "Compilation failed due to errors.\n");

        return 1;
    }

    MECodeObject* co = co_new(filename, stmts);
#ifdef ME_DEBUG
    co_disasm(co);
#endif
    
    darray_for(tokens) free(tokens[__i]);
    darray_free(tokens);
    
    darray_for(stmts) free(stmts[__i]);
    darray_free(stmts);

    diags_dump();
    diags_free();
    free(src);

    MEVM* vm = me_vm_new(co);
    MEVMExitCode res = me_vm_run(vm);
    if (res != MEVM_EXIT_OK) {
        const char* msg = me_get_error_msg();
        fprintf(stderr, "Runtime error: %s\n", msg);
        me_vm_free(vm);
        lut_free();
        return 1;
    }

#ifdef ME_DEBUG
    printf("Execution fin.\n");
#endif


    me_vm_free(vm);
    lut_free();

    return 0;
}
