#ifndef __VM_H
#define __VM_H

#include "object.h"
#include "co.h"

typedef struct _MEVM {
    struct _MEVM* parent;
    MECodeObject* co;
    MEObject** stack;

    uint32_t ip;
    uint32_t sp;
    uint32_t depth;
} MEVM;

typedef enum {
    MEVM_EXIT_OK,
    MEVM_EXIT_ERROR,
} MEVMExitCode;

MEVM* me_vm_new(MECodeObject* co);
MEVMExitCode me_vm_run(MEVM* vm);
void me_vm_free(MEVM* vm);

#endif