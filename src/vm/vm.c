#include "vm.h"

#include "../utils/darray.h"
#include "co.h"
#include "object.h"
#include "objects/functionobject.h"

#define TOP(vm) ((vm)->stack[(vm)->sp - 1])
#define POP(vm) (darray_pop((vm)->stack), (vm)->sp--, TOP(vm))
#define PUSH(vm, obj) (darray_push((vm)->stack, (obj)), (vm)->sp++)

MEObject* me_binary_op(MEObject* lhs, MEObject* rhs, BinaryOp op);
MEObject* me_unary_op(MEObject* obj, UnaryOp op);
MEObject* me_function_call(MEObject* func_obj, MEObject** args, uint8_t arg_count);
MEObject* me_is_falsey(MEObject* obj);

MEVM* me_vm_new(MECodeObject* co) {
    MEVM* vm = (MEVM*)malloc(sizeof(MEVM));
    vm->co = co;
    vm->stack = darray_new(MEObject*);
    vm->ip = 0;
    vm->sp = 0;

    return vm;
}

int me_vm_run(MEVM* vm) {
    while (vm->ip < vm->co->co_size) {
        MECodeOp op = vm->co->co_bytecode[vm->ip++];
        switch (op) {
            case CO_OP_NOP:
                break;
            case CO_OP_POP: {
                if (vm->sp == 0)
                    return MEVM_EXIT_STACK_UNDERFLOW;

                POP(vm);
                break;
            }
            case CO_OP_LOAD_CONST: {
                uint16_t idx = *(uint16_t*)(vm->co->co_bytecode + vm->ip);
                vm->ip += 2;

                PUSH(vm, vm->co->co_consts[idx]);
                break;
            }
            case CO_OP_LOAD_GLOBAL: {
                uint16_t idx = *(uint16_t*)(vm->co->co_bytecode + vm->ip);
                vm->ip += 2;
                
                PUSH(vm, vm->co->co_globals[idx]);
                break;
            }
            case CO_OP_LOAD_VARIABLE: {
                uint16_t idx = *(uint16_t*)(vm->co->co_bytecode + vm->ip);
                vm->ip += 2;

                PUSH(vm, vm->co->co_locals[idx]);
                break;
            }
            case CO_OP_STORE_GLOBAL: {
                uint16_t idx = *(uint16_t*)(vm->co->co_bytecode + vm->ip);
                vm->ip += 2;

                if (vm->sp == 0)
                    return MEVM_EXIT_STACK_UNDERFLOW;

                vm->co->co_globals[idx] = POP(vm);
                break;
            }
            case CO_OP_STORE_VARIABLE: {
                uint16_t idx = *(uint16_t*)(vm->co->co_bytecode + vm->ip);
                vm->ip += 2;

                if (vm->sp == 0)
                    return MEVM_EXIT_STACK_UNDERFLOW;

                vm->co->co_locals[idx] = POP(vm);
                break;
            }
            case CO_OP_BINARY_OP: {
                if (vm->sp < 2)
                    return MEVM_EXIT_STACK_UNDERFLOW;

                MEObject* rhs = POP(vm);
                MEObject* lhs = POP(vm);

                uint8_t op = vm->co->co_bytecode[vm->ip++];
                MEObject* result = me_binary_op(lhs, rhs, op);
                if (!result)
                    return MEVM_EXIT_ERROR;

                PUSH(vm, result);
                break;
            }
            case CO_OP_UNARY_OP: {
                if (vm->sp == 0)
                    return MEVM_EXIT_STACK_UNDERFLOW;

                MEObject* obj = POP(vm);
                uint8_t op = vm->co->co_bytecode[vm->ip++];
                MEObject* result = me_unary_op(obj, op);
                if (!result)
                    return MEVM_EXIT_ERROR;

                PUSH(vm, result);
                break;
            }
            case CO_OP_CALL_FUNCTION: {
                uint8_t arg_count = vm->co->co_bytecode[vm->ip++];
                if (vm->sp < arg_count)
                    return MEVM_EXIT_STACK_UNDERFLOW;

                MEObject** args = malloc(arg_count * sizeof(MEObject*));
                for (uint8_t i = 0; i < arg_count; i++) {
                    args[arg_count - 1 - i] = POP(vm);
                }

                MEObject* func_obj = POP(vm);
                if (!me_function_check(func_obj)) {
                    // NON CALLABLE OBJECT
                    free(args);
                    return MEVM_EXIT_ERROR;
                }

                MEObject* result = me_function_call(func_obj, args, arg_count);
                free(args);

                if (!result)
                    return MEVM_EXIT_ERROR;

                PUSH(vm, result);
                break;
            }
            case CO_OP_RETURN: {
                if (vm->sp == 0)
                    return MEVM_EXIT_STACK_UNDERFLOW;

                MEObject* return_value = POP(vm);
                if (vm->co->in_function) {
                    vm->co->in_function = 0;
                    PUSH(vm, return_value);
                } else {
                    // For functions mainly
                    return MEVM_EXIT_OK; // Exit the VM with the return value on the stack
                }
                break;
            }
            case CO_OP_JUMP_IF_FALSE: {
                uint16_t jump_if_false_offset = *(uint16_t*)(vm->co->co_bytecode + vm->ip);
                vm->ip += 2;

                if (vm->sp == 0)
                    return MEVM_EXIT_STACK_UNDERFLOW;

                MEObject* condition = POP(vm);
                if (me_is_falsey(condition)) {
                    vm->ip += jump_if_false_offset;
                }
                break;
            }
            case CO_OP_JUMP_REL: {
                int16_t jump_offset = *(int16_t*)(vm->co->co_bytecode + vm->ip);
                vm->ip += 2;

                vm->ip += jump_offset;
                break;
            }
            default:
                return MEVM_EXIT_INVALID_OPCODE;
        }
    }

    return MEVM_EXIT_OK;
}


void me_vm_free(MEVM* vm) {
    darray_free(vm->stack);
    co_free(vm->co);
    free(vm);
}