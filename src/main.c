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

int main() {
    const char* filename = "deneme.txt";
    char* src = read_file_binary(filename, NULL);

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
    darray_for(tokens) token_dump(tokens[__i]);

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

    diags_free();

    // Byte code generation here after bytecode generation we can free the tokens and stmts

    darray_for(tokens) free(tokens[__i]);
    darray_free(tokens);

    darray_for(stmts) free(stmts[__i]);
    darray_free(stmts);

    // VM execution here

    diags_dump();
    diags_free();
    lut_free();

    return 0;
}