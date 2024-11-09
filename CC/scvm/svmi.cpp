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
	uint8_t* p_pcode = m_pimage_info->get_code();
	uint8_t* p_data = m_pimage_info->get_data();
	SVMI_VCPU_registers* p_regs = p_ctx->get_regs();
	union {
		struct { float fa, fb, fc; };
		struct { int   ia, ib, ic; };
	};
	/* parse bytecode */
	while (1) {
		/* decode instruction 
		 structure: [opcode][mode][rdst][rsrc][reserved] */
		SVM_instr_fetch(instr, *((int *)&p_pcode[p_regs->IP]));
		/* handle opcodes */
		p_regs->add_IP(VM_I_SIZE); //skip instruction size, move next to args
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
				p_regs->regs[instr.rdst] = *((int*)&p_pcode[p_regs->IP]);
				p_regs->add_IP(SVM_CELL_SIZE); //move next from imm32 instr arg
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
					fc = SVM_ctof(p_regs->regs[SVM_REG_A]) + SVM_ctof(p_regs->regs[SVM_REG_B]) * (float)ia;		
					p_regs->regs[SVM_REG_A] = SVM_ftoc(fc);
				}
				else {
					/* integer */
					p_regs->regs[SVM_REG_A] = p_regs->regs[SVM_REG_A] + p_regs->regs[SVM_REG_B] * ia;
				}
				break; /* no args */
			}
			// add imm32 to register
			// s: SVM_OP_MOV SVMI_ARG_IMM rdst rsrc imm32
			if (instr.mode == SVMI_ARG_IMM) {
				if (instr.flags & VMI_F_FP_OP) {
					fc = SVM_ctof(p_regs->regs[SVM_REG_A]) + SVM_ctof(p_pcode[p_regs->IP]) * (float)ia;
					p_regs->regs[SVM_REG_A] = SVM_ftoc(fc);
				}
				else {
					/* integer */
					p_regs->regs[SVM_REG_A] = p_regs->regs[SVM_REG_A] + *((int*)&p_pcode[p_regs->IP]) * ia;
				}
				p_regs->add_IP(SVM_CELL_SIZE); //move next from imm32 instr arg
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
				}
				else {
					p_regs->regs[instr.rdst] += ia;
				}
				break;
			}
			break;

		case SVM_OP_MUL:
			break;
		case SVM_OP_DIV:
			break;
		case SVM_OP_MOD:
			break;
		case SVM_OP_SHL:
			break;
		case SVM_OP_SHR:
			break;
		case SVM_OP_AND:
			break;
		case SVM_OP_OR:
			break;
		case SVM_OP_XOR:
			break;
		case SVM_OP_NOT:
			break;
		case SVM_OP_CMP:
			break;
		case SVM_OP_JMP:
			break;
		case SVM_OP_JZ:
			break;
		case SVM_OP_JNZ:
			break;
		case SVM_OP_JE:
			break;
		case SVM_OP_JNE:
			break;
		case SVM_OP_JL:
			break;
		case SVM_OP_JLE:
			break;
		case SVM_OP_JG:
			break;
		case SVM_OP_JGE:
			break;
		case SVM_OP_LOOP:
			break;
		case SVM_OP_CASE:
			break;
		case SVM_OP_PUSH:
			break;
		case SVM_OP_PUSHSR:
			break;
		case SVM_OP_POP:
			break;
		case SVM_OP_POPSR:
			break;
		case SVM_OP_CALL:
			break;
		case SVM_OP_NCALL:
			break;
		case SVM_OP_RET:
			break;
		case SVM_REG_BRK:
			break;
		case SVM_REG_HALT:
			break;
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
			break;
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