#ifndef __PARSER_H
#define __PARSER_H

#include "token.h"
#include "node.h"

Stmt** parse(const char* filename, Token** tokens);

#endif