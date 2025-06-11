#include "vm.h"

#include <stdio.h>

#include "../utils/darray.h"
#include "../lut.h"

#include "co.h"

#include "objects/builtinfnobject.h"
#include "objects/functionobject.h"
#include "objects/errorobject.h"
#include "objects/boolobject.h"
#include "object.h"

#define TOP(vm) ((vm)->stack[(vm)->sp - 1])
// #define POP(vm) (darray_pop((vm)->stack), (vm)->sp--, TOP(vm))
#define POP(vm) ({ MEObject* obj = TOP(vm); darray_pop((vm)->stack); (vm)->sp--; obj; })
#define PUSH(vm, obj) (darray_push((vm)->stack, (obj)), (vm)->sp++)

MEObject* me_binary_op(MEObject* lhs, MEObject* rhs, BinaryOp op);
MEObject* me_unary_op(MEObject* obj, UnaryOp op);
MEVMExitCode me_function_call(MEVM* vm, MEObject* func_obj, MEObject** args, uint8_t arg_count);

MEObject* me_binary_add(MEObject* lhs, MEObject* rhs);
MEObject* me_binary_sub(MEObject* lhs, MEObject* rhs);
MEObject* me_binary_mul(MEObject* lhs, MEObject* rhs);
MEObject* me_binary_div(MEObject* lhs, MEObject* rhs);
MEObject* me_binary_mod(MEObject* lhs, MEObject* rhs);
MEObject* me_binary_bit_and(MEObject* lhs, MEObject* rhs);
MEObject* me_binary_bit_or(MEObject* lhs, MEObject* rhs);
MEObject* me_binary_bit_xor(MEObject* lhs, MEObject* rhs);
MEObject* me_binary_lshift(MEObject* lhs, MEObject* rhs);
MEObject* me_binary_rshift(MEObject* lhs, MEObject* rhs);
MEObject* me_binary_cmp(MEObject* lhs, MEObject* rhs, BinaryOp op);

MEVM* me_vm_new(MECodeObject* co) {
    MEVM* vm = (MEVM*)malloc(sizeof(MEVM));
    vm->parent = NULL;
    vm->co = co;
    vm->stack = darray_new(MEObject*);
    vm->ip = 0;
    vm->sp = 0;

    return vm;
}

MEVMExitCode me_vm_run(MEVM* vm) {
    while (vm->ip < vm->co->co_size) {
        // printf("%04u\n", vm->ip);
    
        MECodeOp op = vm->co->co_bytecode[vm->ip++];
        switch (op) {
            case CO_OP_NOP:
                break;
            case CO_OP_POP: {
                if (vm->sp == 0) {
                    me_set_error(me_error_generic, "Stack underflow.");
                    return MEVM_EXIT_ERROR;
                }

                MEObject* o = POP(vm);
                break;
            }
            case CO_OP_DUP: {
                if (vm->sp == 0) {
                    me_set_error(me_error_generic, "Stack underflow.");
                    return MEVM_EXIT_ERROR;
                }

                MEObject* top = TOP(vm);
                PUSH(vm, top);
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

                if (vm->sp == 0) {
                    me_set_error(me_error_generic, "Stack underflow.");
                    return MEVM_EXIT_ERROR;
                }

                vm->co->co_globals[idx] = POP(vm);
                break;
            }
            case CO_OP_STORE_VARIABLE: {
                uint16_t idx = *(uint16_t*)(vm->co->co_bytecode + vm->ip);
                vm->ip += 2;

                if (vm->sp == 0) {
                    me_set_error(me_error_generic, "Stack underflow.");
                    return MEVM_EXIT_ERROR;
                }

                vm->co->co_locals[idx] = POP(vm);
                break;
            }
            case CO_OP_BINARY_OP: {
                if (vm->sp < 2) {
                    me_set_error(me_error_generic, "Stack underflow.");
                    return MEVM_EXIT_ERROR;
                }

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
                if (vm->sp == 0) {
                    me_set_error(me_error_generic, "Stack underflow.");
                    return MEVM_EXIT_ERROR;
                }

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
                if (vm->sp < arg_count) {
                    me_set_error(me_error_generic, "Stack underflow.");
                    return MEVM_EXIT_ERROR;
                }

                MEObject** args = darray_new(MEObject*);
                for (int i = 0; i < arg_count; i++)
                    darray_pushd(args, POP(vm));

                MEObject* func_obj = POP(vm);
                MEVMExitCode result = me_function_call(vm, func_obj, args, arg_count);
                darray_free(args);

                if (result != MEVM_EXIT_OK)
                    return result;

                // Result of the function is pushed to stack by child
                break;
            }
            case CO_OP_RETURN: {
                if (vm->sp == 0) {
                    me_set_error(me_error_generic, "Stack underflow.");
                    return MEVM_EXIT_ERROR;
                }

                MEObject* return_value = POP(vm);
                if (vm->parent)
                    PUSH(vm->parent, return_value);

                break;
            }
            case CO_OP_JUMP_IF_FALSE: {
                uint16_t jump_if_false_offset = *(uint16_t*)(vm->co->co_bytecode + vm->ip);
                vm->ip += 2;

                if (vm->sp == 0) {
                    me_set_error(me_error_generic, "Stack underflow.");
                    return MEVM_EXIT_ERROR;
                }

                MEObject* condition = POP(vm);
                if (!me_is_true(condition)) {
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
                me_set_error(me_error_generic, "Unknown opcode.");
                return MEVM_EXIT_ERROR;
        }
    }

    return MEVM_EXIT_OK;
}

void me_vm_free(MEVM* vm) {
    darray_free(vm->stack);
    co_free(vm->co);
    free(vm);
}

MEObject* me_binary_op(MEObject* lhs, MEObject* rhs, BinaryOp op) {
    switch (op) {
        case BIN_ASSIGN:
            return rhs;
        case BIN_ADD:
            return me_binary_add(lhs, rhs);
        case BIN_SUB:
            return me_binary_sub(lhs, rhs);
        case BIN_MUL:
            return me_binary_mul(lhs, rhs);
        case BIN_DIV:
            return me_binary_div(lhs, rhs);
        case BIN_MOD:
            return me_binary_mod(lhs, rhs);
        case BIN_BIT_AND:
            return me_binary_bit_and(lhs, rhs);
        case BIN_BIT_OR:
            return me_binary_bit_or(lhs, rhs);
        case BIN_BIT_XOR:
            return me_binary_bit_xor(lhs, rhs);
        case BIN_BIT_LSHIFT:
            return me_binary_lshift(lhs, rhs);
        case BIN_BIT_RSHIFT:
            return me_binary_rshift(lhs, rhs);
        case BIN_EQ:
        case BIN_NEQ:
        case BIN_LTE:
        case BIN_LT:
        case BIN_GTE:
        case BIN_GT:
            return me_binary_cmp(lhs, rhs, op);
        default:
            break;
    }

    me_set_error(me_error_notimplemented, "Something went wrong between \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
    return NULL;
}

MEObject* me_binary_add(MEObject* lhs, MEObject* rhs) {
    if (!ME_TYPE(lhs)->tp_nb_add || !ME_TYPE(rhs)->tp_nb_add) {
        me_set_error(me_error_notimplemented, "Binary addition not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    MEObject* result = ME_TYPE(lhs)->tp_nb_add(lhs, rhs);
    if (result == me_error_notimplemented)
        result = ME_TYPE(rhs)->tp_nb_add(rhs, lhs);

    if (result == me_error_notimplemented) {
        me_set_error(me_error_notimplemented, "Binary addition not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    return result;
}

MEObject* me_binary_sub(MEObject* lhs, MEObject* rhs) {
    if (!ME_TYPE(lhs)->tp_nb_sub || !ME_TYPE(rhs)->tp_nb_sub) {
        me_set_error(me_error_notimplemented, "Binary substraction not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    MEObject* result = ME_TYPE(lhs)->tp_nb_sub(lhs, rhs);
    if (result == me_error_notimplemented)
        result = ME_TYPE(rhs)->tp_nb_sub(rhs, lhs);

    if (result == me_error_notimplemented) {
        me_set_error(me_error_notimplemented, "Binary substraction not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    return result;
}

MEObject* me_binary_mul(MEObject* lhs, MEObject* rhs) {
    if (!ME_TYPE(lhs)->tp_nb_mul || !ME_TYPE(rhs)->tp_nb_mul) {
        me_set_error(me_error_notimplemented, "Binary multiplication not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    MEObject* result = ME_TYPE(lhs)->tp_nb_mul(lhs, rhs);
    if (result == me_error_notimplemented)
        result = ME_TYPE(rhs)->tp_nb_mul(rhs, lhs);

    if (result == me_error_notimplemented) {
        me_set_error(me_error_notimplemented, "Binary multiplication not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    return result;
}

MEObject* me_binary_div(MEObject* lhs, MEObject* rhs) {
    if (!ME_TYPE(lhs)->tp_nb_div || !ME_TYPE(rhs)->tp_nb_div) {
        me_set_error(me_error_notimplemented, "Binary division not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    MEObject* result = ME_TYPE(lhs)->tp_nb_div(lhs, rhs);
    if (result == me_error_notimplemented)
        result = ME_TYPE(rhs)->tp_nb_div(rhs, lhs);

    if (result == me_error_notimplemented) {
        me_set_error(me_error_notimplemented, "Binary division not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    return result;
}

MEObject* me_binary_mod(MEObject* lhs, MEObject* rhs) {
    if (!ME_TYPE(lhs)->tp_nb_mod || !ME_TYPE(rhs)->tp_nb_mod) {
        me_set_error(me_error_notimplemented, "Binary modulo not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    MEObject* result = ME_TYPE(lhs)->tp_nb_mod(lhs, rhs);
    if (result == me_error_notimplemented)
        result = ME_TYPE(rhs)->tp_nb_mod(rhs, lhs);

    if (result == me_error_notimplemented) {
        me_set_error(me_error_notimplemented, "Binary modulo not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    return result;
}

MEObject* me_binary_bit_and(MEObject* lhs, MEObject* rhs) {
    if (!ME_TYPE(lhs)->tp_nb_bit_and || !ME_TYPE(rhs)->tp_nb_bit_and) {
        me_set_error(me_error_notimplemented, "Binary bit and not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    MEObject* result = ME_TYPE(lhs)->tp_nb_bit_and(lhs, rhs);
    if (result == me_error_notimplemented)
        result = ME_TYPE(rhs)->tp_nb_bit_and(rhs, lhs);

    if (result == me_error_notimplemented) {
        me_set_error(me_error_notimplemented, "Binary bit and not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    return result;
}

MEObject* me_binary_bit_or(MEObject* lhs, MEObject* rhs) {
    if (!ME_TYPE(lhs)->tp_nb_bit_or || !ME_TYPE(rhs)->tp_nb_bit_or) {
        me_set_error(me_error_notimplemented, "Binary bit or not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    MEObject* result = ME_TYPE(lhs)->tp_nb_bit_or(lhs, rhs);
    if (result == me_error_notimplemented)
        result = ME_TYPE(rhs)->tp_nb_bit_or(rhs, lhs);

    if (result == me_error_notimplemented) {
        me_set_error(me_error_notimplemented, "Binary bit or not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    return result;
}

MEObject* me_binary_bit_xor(MEObject* lhs, MEObject* rhs) {
    if (!ME_TYPE(lhs)->tp_nb_bit_xor || !ME_TYPE(rhs)->tp_nb_bit_xor) {
        me_set_error(me_error_notimplemented, "Binary bit xor not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    MEObject* result = ME_TYPE(lhs)->tp_nb_bit_xor(lhs, rhs);
    if (result == me_error_notimplemented)
        result = ME_TYPE(rhs)->tp_nb_bit_xor(rhs, lhs);

    if (result == me_error_notimplemented) {
        me_set_error(me_error_notimplemented, "Binary bit xor not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    return result;
}

MEObject* me_binary_lshift(MEObject* lhs, MEObject* rhs) {
    if (!ME_TYPE(lhs)->tp_nb_lshift || !ME_TYPE(rhs)->tp_nb_lshift) {
        me_set_error(me_error_notimplemented, "Binary left shift not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    MEObject* result = ME_TYPE(lhs)->tp_nb_lshift(lhs, rhs);
    if (result == me_error_notimplemented)
        result = ME_TYPE(rhs)->tp_nb_lshift(rhs, lhs);

    if (result == me_error_notimplemented) {
        me_set_error(me_error_notimplemented, "Binary left shift not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    return result;
}

MEObject* me_binary_rshift(MEObject* lhs, MEObject* rhs) {
    if (!ME_TYPE(lhs)->tp_nb_rshift || !ME_TYPE(rhs)->tp_nb_rshift) {
        me_set_error(me_error_notimplemented, "Binary right shift not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    MEObject* result = ME_TYPE(lhs)->tp_nb_rshift(lhs, rhs);
    if (result == me_error_notimplemented)
        result = ME_TYPE(rhs)->tp_nb_rshift(rhs, lhs);

    if (result == me_error_notimplemented) {
        me_set_error(me_error_notimplemented, "Binary right shift not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    return result;
}

MEObject* me_binary_cmp(MEObject* lhs, MEObject* rhs, BinaryOp op) {
    if (!ME_TYPE(lhs)->tp_cmp || !ME_TYPE(rhs)->tp_cmp) {
        me_set_error(me_error_notimplemented, "Binary comparison not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    MEObject* result = ME_TYPE(lhs)->tp_cmp(lhs, rhs, lut_binop_to_cmpop[op]);
    if (result == me_error_notimplemented)
        result = ME_TYPE(rhs)->tp_cmp(rhs, lhs, lut_binop_to_cmpop[op]);

    if (result == me_error_notimplemented) {
        me_set_error(me_error_notimplemented, "Binary comparison not implemented for \"%s\" and \"%s\".", ME_TYPE_NAME(lhs), ME_TYPE_NAME(rhs));
        return NULL;
    }

    return result;
}

MEObject* me_unary_op(MEObject* obj, UnaryOp op) {
    switch (op) {
        case UNARY_BIT_NOT:
            if (!ME_TYPE(obj)->tp_unary_bit_not) {
                me_set_error(me_error_notimplemented, "Unary bit not not implemented for operand type: \"%s\".", ME_TYPE_NAME(obj));
                return NULL;
            }

            return ME_TYPE(obj)->tp_unary_bit_not(obj);
        case UNARY_POSITIVE:
            if (!ME_TYPE(obj)->tp_unary_positive) {
                me_set_error(me_error_notimplemented, "Unary positive not implemented for operand type: \"%s\".", ME_TYPE_NAME(obj));
                return NULL;
            }

            return ME_TYPE(obj)->tp_unary_positive(obj);
        case UNARY_NEGATIVE:
            if (!ME_TYPE(obj)->tp_unary_negative) {
                me_set_error(me_error_notimplemented, "Unary negative not implemented for operand type: \"%s\".", ME_TYPE_NAME(obj));
                return NULL;
            }

            return ME_TYPE(obj)->tp_unary_negative(obj);
        case UNARY_LOGICAL_NOT: {
            if (!ME_TYPE(obj)->tp_bool) {
                me_set_error(me_error_notimplemented, "Unary logical not not implemented for \"%s\".", ME_TYPE_NAME(obj));
                return NULL;
            }

            MEObject* result = ME_TYPE(obj)->tp_bool(obj);
            if (result == me_error_notimplemented) {
                me_set_error(me_error_notimplemented, "Unary logical not not implemented for \"%s\".", ME_TYPE_NAME(obj));
                return NULL;
            }

            return ME_TYPE(obj)->tp_bool(obj);
        }
        case UNARY_POST_INC:
        case UNARY_POST_DEC:
        case UNARY_PRE_INC:
        case UNARY_PRE_DEC:
            me_set_error(me_error_notimplemented, "Unknown unary operation for operand \"%s\".", ME_TYPE_NAME(obj));
            return NULL;

    }
}

MEVMExitCode me_function_call(MEVM* vm, MEObject* func_obj, MEObject** args, uint8_t arg_count) {
    if (me_function_check(func_obj)) {
        MEFunctionObject* func = (MEFunctionObject*)func_obj;
        if (arg_count != func->nargs) {
            me_set_error(me_error_generic, "Function \"%s\" expects %u arguments, got %u.", func->co->co_name, func->nargs, arg_count);
            return MEVM_EXIT_ERROR;
        }
    
        MEVM* func_vm = me_vm_new(func->co);
        vm->parent = vm;
    
        MEVMExitCode exit = me_vm_run(vm);
        me_vm_free(func_vm);
        return exit;
    } else if (me_builtinfn_check(func_obj)) {
        MEObject* result = ((MEBuiltinFnObject*)func_obj)->fn(func_obj, args);
        if (!result) // In case of NULL error must be set by the function itself
            return MEVM_EXIT_ERROR;

        PUSH(vm, result);
        return MEVM_EXIT_OK;
    }


    me_set_error(me_error_typemismatch, "Object is not callable: \"%s\".", ME_TYPE_NAME(func_obj));
    return MEVM_EXIT_ERROR;
}

