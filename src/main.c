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

#include "vm/co.h"
#include "vm/vm.h"

int main(int argc, char *argv[]) {
#ifndef ME_DEBUG
    // COMMAND LINE HANDLING WILL BE DONE IN SEPERATE FILE LASTLY DO NOT ADD THINGS LIKE THAT HERE.
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
#else
    const char* filename = "deneme.me"; // For now let's assume "filename" is both utf-8 and null terminated string
#endif

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
    // darray_for(tokens) token_dump(tokens[__i]);

    Stmt** stmts = parse(filename, tokens);
    darray_for(stmts) stmt_dump(stmts[__i]);
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

    // Byte code generation here after bytecode generation we can free the tokens and stmts
    MECodeObject* co = co_new(filename, stmts);
    co_disasm(co);
    
    darray_for(tokens) free(tokens[__i]);
    darray_free(tokens);
    
    darray_for(stmts) free(stmts[__i]);
    darray_free(stmts);

    diags_dump();
    diags_free();
    free(src);

    // Different structure than diag will be used for runtime errors
    // VM execution here

    lut_free();

    return 0;
}
