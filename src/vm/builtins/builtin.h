#ifndef __BUILTIN_H
#define __BUILTIN_H

#include "../co.h"
#include "../../parser/analyser.h"

void me_register_builtins_co(MECodeObject* co);
void me_register_builtins_analyser(Analyser* analyser);

#endif