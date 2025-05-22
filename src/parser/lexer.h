#ifndef __LEXER_H
#define __LEXER_H

#include "token.h"

Token** lex(const char* filename, const char* src);

#endif