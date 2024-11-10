#include "svmi.h"

SVMI_image::SVMI_image()
{
}

SVMI_image::~SVMI_image()
{
}

SVMI_STATUS SVMI_image::load_from_disk(const char* p_filename)
{
	return SVMI_STATUS();
}

SVMI_STATUS SVMI_image::load_from_memory(uint8_t* p_image_data, size_t image_size)
{
	return SVMI_STATUS();
}

/**
* executing byte-code here
*/
SVMI_STATUS SVMI::exec(SVMI_context* p_ctx)
{
	assert(m_pimage_info && "image ptr was nullptr!");
	SVM_instruction instr;
	uint8_t  *p_pcode = m_pimage_info->get_code();
	uint8_t  *p_data = m_pimage_info->get_data();
	cell_t   *p_stack = p_ctx->get_stack();
	SVMI_VCPU_registers* p_regs = p_ctx->get_regs();
	union {
		struct { float  fa, fb, fc; };
		struct { cell_t ia, ib, ic; };
	};
	/* parse bytecode */
	while (1) {
		/* decode instruction 
		 structure: [opcode][mode][rdst][rsrc][reserved] */
		SVM_instr_fetch(instr, *((int *)&p_pcode[p_regs->IP]));
		/* handle opcodes */
		p_regs->SR_reset(); //reset state register
		p_regs->IP_add(VM_I_SIZE); //skip instruction size, move next to args
		switch (instr.opcode) {
			/* NOP */
		case SVM_OP_NOP:
			break;

		/* MOVE */
		case SVM_OP_MOV:
			/* register to register */
			if (instr.mode == SVMI_ARG_REG) {
				p_regs->regs[instr.rdst] = p_regs->regs[instr.rsrc];
				break; /* no args */
			}
			// move value from register to memory
			// s: SVM_OP_MOV SVMI_ARG_REG rdst rsrc
			if (instr.mode == SVMI_ARG_REG) {
				p_data[p_regs->regs[instr.rdst]] = p_regs->regs[instr.rsrc] & 0xff;
				break; /* no args */
			}
			// move imm32 value to register
			// s: SVM_OP_MOV SVMI_ARG_IMM rdst rsrc imm32
			if (instr.mode == SVMI_ARG_IMM) {
				p_regs->regs[instr.rdst] = *((cell_t*)&p_pcode[p_regs->IP]);
				p_regs->IP_add(SVM_CELL_SIZE); //move next from imm32 instr arg
				break; /* 1 arg - 4 bytes */
			}
			/* instr.mode is not correc t */
			return SVMI_STATUS_INVALID_INSTRUCTION;

		/* ADD/SUBTRACT */
		// A = A + B
			// A = A - B
		case SVM_OP_ADD:
		case SVM_OP_SUB:
			/* register with register */
			ia = (instr.opcode == SVM_OP_ADD) ? 1 : -1; /* is ADD? */
			if (instr.mode == SVMI_ARG_REG) {
				/* floating point addition */
				if (instr.flags & VMI_F_FP_OP) {				
					fc = SVM_ctof(p_regs->A) + SVM_ctof(p_regs->B) * (float)ia;		
					p_regs->SR_set_flags(fc);
					p_regs->A = SVM_ftoc(fc);
				}
				else {
					/* integer */
					p_regs->A = p_regs->A + p_regs->B * ia;
					p_regs->SR_set_flags(p_regs->A);
				}
				break; /* no args */
			}
			// add imm32 to register
			// s: SVM_OP_ADD|SVM_OP_SUB SVMI_ARG_IMM rdst rsrc imm32
			if (instr.mode == SVMI_ARG_IMM) {
				if (instr.flags & VMI_F_FP_OP) {
					fc = SVM_ctof(p_regs->A) + SVM_ctof(p_pcode[p_regs->IP]) * (float)ia;
					p_regs->SR_set_flags(fc);
					p_regs->A = SVM_ftoc(fc);
				}
				else {
					/* integer add/sub operation*/
					p_regs->A = p_regs->A + *((cell_t*)&p_pcode[p_regs->IP]) * ia;
					p_regs->SR_set_flags(p_regs->A);
				}
				p_regs->IP_add(SVM_CELL_SIZE); //move next from imm32 instr arg
				break; /* 1 arg - 4 bytes */
			}
			break;

			/* INCREMENT/DECREMENT */
			// R++;R--
		case SVM_OP_INC:
		case SVM_OP_DEC:
			/* increment only register value */
			ia = (instr.opcode == SVM_OP_INC) ? 1 : -1;
			if (instr.mode == SVMI_ARG_REG) {
				if (instr.flags & VMI_F_FP_OP) {
					SVM_ctof(p_regs->regs[instr.rdst]) += (float)ia;
					p_regs->SR_set_flags(SVM_ctof(p_regs->regs[instr.rdst]));
				}
				else {
					p_regs->regs[instr.rdst] += ia;
					p_regs->SR_set_flags(p_regs->regs[instr.rdst]);
				}
				break;
			}
			break;

			/* MUL/DIV */
		case SVM_OP_MUL:
		case SVM_OP_DIV:
			ia = (int)(instr.opcode == SVM_OP_MUL);			
			/* register with register */
			if (instr.mode == SVMI_ARG_REG) {
				/* floating point op */
				if (instr.flags & VMI_F_FP_OP) {				
					if (ia) {
						/*SVM_OP_MUL*/
						fc = SVM_ctof(p_regs->A) * SVM_ctof(p_regs->B);	
					}
					else {
						/*SVM_OP_DIV*/
						fb = SVM_ctof(p_regs->B);
						if (fabsf(fb) < FLT_EPSILON) {
							/* register B was contains zero value */
							return SVMI_STATUS_FLOATING_POINT_DIVISION_BY_ZERO;
						}
						fc = SVM_ctof(p_regs->A) / fb;
					}
					p_regs->SR_set_flags(fc);
					p_regs->A = SVM_ftoc(fc);
				}
				else {
					/* integer */
					if (ia) {
						/*SVM_OP_MUL*/
						p_regs->A = p_regs->A * p_regs->B;
						p_regs->SR_set_flags(p_regs->A);
					}
					else {
						/*SVM_OP_DIV*/
						if (!p_regs->B) {
							/* register B was contains zero value */
							return SVMI_STATUS_FLOATING_POINT_DIVISION_BY_ZERO;
						}
						p_regs->A = p_regs->A / p_regs->B;
						p_regs->SR_set_flags(p_regs->A);
					}
				}
				break; /* no args */
			}
			// mul/div register by imm32 value
			// s: SVM_OP_MOV SVMI_ARG_IMM rdst rsrc imm32
			if (instr.mode == SVMI_ARG_IMM) {
				/* is floatign poinnt? */
				if (instr.flags & VMI_F_FP_OP) {
					if (ia) {
						/*SVM_OP_MUL*/
						fc = SVM_ctof(p_regs->A) * SVM_ctof(p_pcode[p_regs->IP]);
					}
					else {
						/*SVM_OP_DIV*/
						fb = SVM_ctof(p_pcode[p_regs->IP]);
						if (fabsf(fb) < FLT_EPSILON) {
							/* prevent division by zero */
							return SVMI_STATUS_FLOATING_POINT_DIVISION_BY_ZERO;
						}
						fc = SVM_ctof(p_regs->A) / fb;
					}
					p_regs->SR_set_flags(fc);
					p_regs->A = SVM_ftoc(fc);
				}
				else {
					/* integer */
					if (ia) {
						/*SVM_OP_MUL*/
						p_regs->A = p_regs->A * *((cell_t*)&p_pcode[p_regs->IP]);
					}
					else {
						/*SVM_OP_DIV*/
						ib = *((cell_t*)&p_pcode[p_regs->IP]);
						if (!ib) {
							/* imm32 is zero for division */
							return SVMI_STATUS_INT_DIVISION_BY_ZERO;
						}
						p_regs->A = p_regs->A / ib;
					}
					p_regs->SR_set_flags(p_regs->A); //update SR
				}
				p_regs->IP_add(SVM_CELL_SIZE); //move next from imm32 instr arg
				break; /* 1 arg - 4 bytes */
			}
			break;

		case SVM_OP_MOD:
			/* register with register */
			if (instr.mode == SVMI_ARG_REG) {
				if (!p_regs->B) {
					/* imm32 is zero for division */
					return SVMI_STATUS_INT_DIVISION_BY_ZERO;
				}
				p_regs->A = p_regs->A % p_regs->B;
				p_regs->SR_set_flags(p_regs->A);
				break; /* no args */
			}
			// imm32
			// s: SVM_OP_MOD SVMI_ARG_IMM rdst rsrc imm32
			if (instr.mode == SVMI_ARG_IMM) {
				ia = *((cell_t*)&p_pcode[p_regs->IP]);
				if (!ia) {
					/* imm32 is zero for division */
					return SVMI_STATUS_INT_DIVISION_BY_ZERO;
				}
				p_regs->A = p_regs->A % ia;
				p_regs->SR_set_flags(p_regs->A);
				p_regs->IP_add(SVM_CELL_SIZE); //move next from imm32 instr arg
				break; /* 1 arg - 4 bytes */
			}
			break;

			/* SHIFT LEFT */
		case SVM_OP_SHL:
			//if (instr.mode == SVMI_ARG_REG) {
				p_regs->regs[instr.rdst] = p_regs->regs[instr.rsrc] << p_regs->regs[instr.rdst];
				p_regs->SR_set_flags(p_regs->regs[instr.rdst]);
				break; /* no args */
			//}
			break;

		case SVM_OP_SHR:
			//if (instr.mode == SVMI_ARG_REG) {
			p_regs->regs[instr.rdst] = p_regs->regs[instr.rsrc] >> p_regs->regs[instr.rdst];
			p_regs->SR_set_flags(p_regs->regs[instr.rdst]);
			break; /* no args */
			//}
			break;

		case SVM_OP_AND:
			/* register with register */
			if (instr.mode == SVMI_ARG_REG) {
				p_regs->regs[instr.rdst] &= p_regs->regs[instr.rsrc];
				p_regs->SR_set_flags(p_regs->regs[instr.rdst]);
				break; /* no args */
			}
			/* register with imm32 */
			if (instr.mode == SVMI_ARG_IMM) {
				p_regs->regs[instr.rdst] &= *((cell_t*)&p_pcode[p_regs->IP]);
				p_regs->SR_set_flags(p_regs->regs[instr.rdst]);
				p_regs->IP_add(SVM_CELL_SIZE); //move next from imm32 instr arg
				break; /* 1 arg - 4 bytes */
			}
			break;

		case SVM_OP_OR:
			/* register with register */
			if (instr.mode == SVMI_ARG_REG) {
				p_regs->regs[instr.rdst] |= p_regs->regs[instr.rsrc];
				p_regs->SR_set_flags(p_regs->regs[instr.rdst]);
				break; /* no args */
			}
			/* register with imm32 */
			if (instr.mode == SVMI_ARG_IMM) {
				p_regs->regs[instr.rdst] |= *((cell_t*)&p_pcode[p_regs->IP]);
				p_regs->SR_set_flags(p_regs->regs[instr.rdst]);
				p_regs->IP_add(SVM_CELL_SIZE); //move next from imm32 instr arg
				break; /* 1 arg - 4 bytes */
			}
			break;

		case SVM_OP_XOR:
			/* register with register */
			if (instr.mode == SVMI_ARG_REG) {
				p_regs->regs[instr.rdst] ^= p_regs->regs[instr.rsrc];
				p_regs->SR_set_flags(p_regs->regs[instr.rdst]);
				break; /* no args */
			}
			/* register with imm32 */
			if (instr.mode == SVMI_ARG_IMM) {
				p_regs->regs[instr.rdst] ^= *((cell_t*)&p_pcode[p_regs->IP]);
				p_regs->SR_set_flags(p_regs->regs[instr.rdst]);
				p_regs->IP_add(SVM_CELL_SIZE); //move next from imm32 instr arg
				break; /* 1 arg - 4 bytes */
			}
			break;

		case SVM_OP_NOT:
			p_regs->regs[instr.rdst] = (cell_t)(!p_regs->regs[instr.rdst]);
			break;

		case SVM_OP_CMP:
			/* register with register */
			if (instr.mode == SVMI_ARG_REG) {
				/* is floatign poinnt? */
				if (instr.flags & VMI_F_FP_OP) {
					fb = SVM_ctof(p_regs->regs[instr.rsrc]) - SVM_ctof(p_regs->regs[instr.rdst]);
					p_regs->SR_set_flags(fb);
				} else {
					ib = p_regs->regs[instr.rsrc] - p_regs->regs[instr.rdst];
					p_regs->SR_set_flags(ib);
				}
				break; /* no args */
			}
			/* register with imm32 */
			if (instr.mode == SVMI_ARG_IMM) {
				/* is floatign poinnt? */
				if (instr.flags & VMI_F_FP_OP) {
					fc = SVM_ctof(p_regs->regs[instr.rsrc]) - SVM_ctof(p_pcode[p_regs->IP]);
					p_regs->SR_set_flags(fc);
				}
				else {
					ib = p_regs->regs[instr.rsrc] - p_pcode[p_regs->IP];
					p_regs->SR_set_flags(ib);
				}
				p_regs->IP_add(SVM_CELL_SIZE); //move next from imm32 instr arg
				break; /* 1 arg - 4 bytes */
			}
			break;

		case SVM_OP_JMP:
			/* imm32 */
			if (instr.mode == SVMI_ARG_IMM) {
				p_regs->IP += *((cell_t*)&p_pcode[p_regs->IP]);
				p_regs->IP_add(SVM_CELL_SIZE);
				break;
			}
			/* with register */
			if (instr.mode == SVMI_ARG_REG) {
				p_regs->IP += p_regs->regs[instr.rsrc];
				break;
			}
			break;

		case SVM_OP_JE:
		case SVM_OP_JZ:
			/* imm32 */
			if (instr.mode == SVMI_ARG_IMM) {
				if (p_regs->SR_is_set(VMSRF_ZF)) {
					p_regs->IP += *((cell_t*)&p_pcode[p_regs->IP]);
				}
				p_regs->IP_add(SVM_CELL_SIZE);
				break;
			}
			/* with register */
			if (p_regs->SR_is_set(VMSRF_ZF)) {
				if (instr.mode == SVMI_ARG_REG) {
					p_regs->IP += p_regs->regs[instr.rsrc];				
				}
				break;
			}
			break;

		case SVM_OP_JNE:
		case SVM_OP_JNZ:
			/* imm32 */
			if (instr.mode == SVMI_ARG_IMM) {
				if (!p_regs->SR_is_set(VMSRF_ZF)) {
					p_regs->IP += *((cell_t*)&p_pcode[p_regs->IP]);
				}
				p_regs->IP_add(SVM_CELL_SIZE);
				break;
			}
			/* with register */
			if (!p_regs->SR_is_set(VMSRF_ZF)) {
				if (instr.mode == SVMI_ARG_REG) {
					p_regs->IP += p_regs->regs[instr.rsrc];
				}
				break;
			}
			break;

		case SVM_OP_JL:
		case SVM_OP_JLE:
			ia = VMSRF_SF;
			if (instr.opcode == SVM_OP_JLE)
				ia |= VMSRF_ZF;

			/* imm32 */
			if (instr.mode == SVMI_ARG_IMM) {
				if (!p_regs->SR_is_set(ia)) {
					p_regs->IP += *((cell_t*)&p_pcode[p_regs->IP]);
				}
				p_regs->IP_add(SVM_CELL_SIZE);
				break;
			}

			/* with register */
			if (!p_regs->SR_is_set(ia)) {
				if (instr.mode == SVMI_ARG_REG) {
					p_regs->IP += p_regs->regs[instr.rsrc];
				}
				break;
			}
			break;

		case SVM_OP_JG:
		case SVM_OP_JGE:
			/* imm32 */
			if (instr.mode == SVMI_ARG_IMM) {
				/* sero flag is set? */
				if (p_regs->SR_is_set(ia))
					p_regs->IP += *((cell_t*)&p_pcode[p_regs->IP]);

				p_regs->IP_add(SVM_CELL_SIZE);
				break;
			}

			/* with register */
			ia =0; // remove SF
			if (instr.opcode == SVM_OP_JLE)
				ia |= VMSRF_ZF;

			if (p_regs->SR_is_set(ia)) {
				/* src is register? */
				if (instr.mode == SVMI_ARG_REG) {
					p_regs->IP += p_regs->regs[instr.rsrc];
					break;
				}
			}
			break;

		case SVM_OP_LOOP:
			/* counter register */
			if (p_regs->C > 0) {
				p_regs->IP += (instr.mode == SVMI_ARG_IMM) ? *((cell_t*)&p_pcode[p_regs->IP]) : p_regs->regs[instr.rsrc];
				p_regs->C--;
			}
			/* zero flag */
			p_regs->SR_is_set(VMSRF_ZF);		
			p_regs->IP_add(SVM_CELL_SIZE);
			break;

		case SVM_OP_CASE:
			break;

		case SVM_OP_PUSH:
			/* detect stack overflow */
			if (p_regs->SP >= p_ctx->get_stack_size())
				return SVMI_STATUS_STACK_OVERFLOW;

			if (instr.mode == SVMI_ARG_IMM) {
				p_stack[p_regs->SP++] = *((cell_t*)&p_pcode[p_regs->IP]);
				p_regs->IP_add(SVM_CELL_SIZE);
				break;
			}
			p_stack[p_regs->SP++] = p_regs->regs[instr.rsrc];
			break;

		case SVM_OP_PUSHSR:
			if (p_regs->SP >= p_ctx->get_stack_size())
				return SVMI_STATUS_STACK_OVERFLOW;

			p_stack[p_regs->SP++] = p_regs->SR;
			break;

		case SVM_OP_POP:
			/* detect stack overflow */
			if (!p_regs->SP)
				return SVMI_STATUS_STACK_OVERFLOW;

			if (instr.mode == SVMI_ARG_REG) {
				p_regs->regs[instr.rdst] = p_stack[p_regs->SP];
				p_regs->SP--;
				break;
			}
			p_regs->SP--;
			break;

		case SVM_OP_POPSR:
			/* detect stack overflow */
			if (!p_regs->SP)
				return SVMI_STATUS_STACK_OVERFLOW;

			p_regs->SR = p_stack[p_regs->SP++];
			break;

		case SVM_OP_CALL:
			break;

		case SVM_OP_NCALL:
			break;

		case SVM_OP_RET:
			break;

		case SVM_REG_BRK:
			break;

			/* HALT */
		case SVM_REG_HALT:
			return SVMI_STATUS_EXECUTION_HALTED; //finish p-code execution

		case SVM_REG_STOI:
			break;

		case SVM_REG_ITOS:
			break;

		case SVM_REG_FTOI:
			break;

		case SVM_REG_ITOF:
			break;

		case SVM_REG_ROUND:
			break;

		case SVM_REG_CEIL:
			break;

		case SVM_REG_FLOOR:
			break;

		default:
			return SVMI_STATUS_INVALID_INSTRUCTION;
		}
	}
	return SVMI_STATUS();
}

SVMI::SVMI()
{
}

SVMI::~SVMI()
{
}

SVMI_STATUS SVMI::init(SVMI_image_info* p_image, const SVMI_native_decl* p_natives)
{
	if (!p_image || !p_natives)
		return SVMI_STATUS_INVALID_PARAM;

	m_pnatives = p_natives;
	m_pimage_info = p_image;
	return SVMI_STATUS_OK;
}

SVMI_STATUS SVMI::shutdown()
{
	return SVMI_STATUS();
}

SVMI_context* SVMI::ctx_create()
{
	SVMI_context* p_ctx;
	std::lock_guard<std::mutex> lock(m_mutex);
	/* is not avalible free contexts? create new context */
	if (m_free_contexts.empty()) {
		/* create context  */
		p_ctx = new (std::nothrow)SVMI_context();
		if (!p_ctx) {
			/* memory allocation for new context failed! */
			return nullptr;
		}
		m_busy_contexts.push_back(p_ctx);
		return p_ctx;
	}
	/* free context avalible */
	p_ctx = m_free_contexts.back();
	m_free_contexts.pop_back();
	return p_ctx;
}

SVMI_STATUS SVMI::call(SVMI_context* p_ctx, uint32_t vm_func_address)
{
	SVMI_VCPU_registers* p_regs = p_ctx->get_regs();
	p_regs->IP = vm_func_address;
	return exec(p_ctx); //exec VM bytecode
}

void SVMI::ctx_destroy(SVMI_context* p_ctx)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_free_contexts.push_back(p_ctx);
}