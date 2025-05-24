#ifndef __LUT_H
#define __LUT_H

#include "utils/hashmap.h"
#include "parser/token.h"
#include "parser/node.h"

extern HashMap* lut_kw_to_token;

extern const char* lut_token_to_str[];
extern BinaryOp lut_token_to_binop[];
extern BinaryOp lut_compound_to_binop[];
// extern const TokenType lut_inst_to_tok[];
// extern const ASTNodeOp lut_token_to_op[];
// extern const char* lut_op_to_str[];

void lut_init();
void lut_free();

#endif