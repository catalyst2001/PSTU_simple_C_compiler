#pragma once
#include "SVM.h"
#include <vector>

class scc_code_emitter
{
	//!!! here class for collecting generated byte-code
	std::vector<uint8_t> m_code;

	template<class _type>
	void write(_type &value) {
		m_code.resize(m_code.size() + sizeof(_type));
		*((_type*)&m_code[m_code.size()]) = value;
	}

	/**
	* write_instruction
	*  opcode
	*  mode
	*  rdst
	*  rsrc
	*  flags
	*/
	void write_instruction(SVM_OP opcode, int mode, int rdst, int rsrc, int flags) {
		/* for 0 arg instructions  */
		int instr_int;
		SVM_instruction instruction;
		instruction.opcode = opcode;
		instruction.mode = mode;
		instruction.rsrc = rsrc;
		instruction.rdst = rdst;
		instruction.flags = flags;
		SVM_instr_write(instr_int, instruction);
		write<int>(instr_int);
	}
	
	/**
	* write_instruction with cell_t arg
	*  opcode
	*  mode
	*  rdst
	*  rsrc
	*  flags
	*  arg
	*/
	void write_instruction(SVM_OP opcode, int mode, int rdst, int rsrc, int flags, cell_t arg) {
		write_instruction(opcode, mode, rdst, rsrc, flags);
		write<cell_t>(arg);
		/* for 1 arg instructions */
	}

public:
	scc_code_emitter() {}
	~scc_code_emitter() {}

	/* nop */
	inline void nop() {
		write_instruction(SVM_OP_NOP, 0, 0, 0, 0);
	}

	/* mov */
	inline void mov(SVM_REGS dst, cell_t value) {
		write_instruction(SVM_OP_MOV, SVMI_ARG_IMM, dst, 0, 0, value);
	}
	inline void mov(SVM_REGS dst, SVM_REGS src) {
		write_instruction(SVM_OP_MOV, SVMI_ARG_REG, dst, src, 0);
	}
	inline void mov(SVM_REGS dst, SVM_REGS src, bool) {
		write_instruction(SVM_OP_MOV, SVMI_ARG_ADDR, dst, src, 0);
	}
	
	/* add */
	/**
	* flags = 0 (default). For use float OP, set flag VMI_F_FP_OP
	*/
	inline void add(SVM_REGS dst, SVM_REGS src, int flags = 0) {
		write_instruction(SVM_OP_ADD, SVMI_ARG_REG, dst, src, flags);
	}
	inline void fadd(SVM_REGS dst, SVM_REGS src) {
		add(dst, src, VMI_F_FP_OP);
	}
	inline void add(SVM_REGS dst, cell_t value) {
		write_instruction(SVM_OP_ADD, SVMI_ARG_REG, dst, 0, 0, value);
	}
	inline void fadd(SVM_REGS dst, float value) {
		write_instruction(SVM_OP_ADD, SVMI_ARG_REG, dst, 0, VMI_F_FP_OP, SVM_ftoc(value));
	}

	/* sub */
	/**
	* flags = 0 (default). For use float OP, set flag VMI_F_FP_OP
	*/
	inline void sub(SVM_REGS dst, SVM_REGS src, int flags = 0) {
		write_instruction(SVM_OP_SUB, SVMI_ARG_REG, dst, src, flags);
	}
	inline void fsub(SVM_REGS dst, SVM_REGS src) {
		add(dst, src, VMI_F_FP_OP);
	}
	inline void sub(SVM_REGS dst, cell_t value) {
		write_instruction(SVM_OP_SUB, SVMI_ARG_IMM, dst, 0, 0, value);
	}
	inline void fsub(SVM_REGS dst, float value) {
		write_instruction(SVM_OP_SUB, SVMI_ARG_IMM, dst, 0, VMI_F_FP_OP, SVM_ftoc(value));
	}

	/**
	* flags = 0 (default). For use float OP, set flag VMI_F_FP_OP
	*/
	inline void inc(SVM_REGS dst, int flags = 0) {
		write_instruction(SVM_OP_INC, SVMI_ARG_REG, dst, 0, flags);
	}
	inline void finc(SVM_REGS dst) {
		inc(dst, VMI_F_FP_OP);
	}
	/**
	* flags = 0 (default). For use float OP, set flag VMI_F_FP_OP
	*/
	inline void dec(SVM_REGS dst, int flags = 0) {
		write_instruction(SVM_OP_DEC, SVMI_ARG_REG, dst, 0, flags);
	}
	inline void fdec(SVM_REGS dst) {
		dec(dst, VMI_F_FP_OP);
	}

	/**
	* flags = 0 (default). For use float OP, set flag VMI_F_FP_OP
	*/
	inline void mul(SVM_REGS dst, SVM_REGS src, int flags = 0) {
		write_instruction(SVM_OP_MUL, SVMI_ARG_REG, dst, src, flags);
	}
	inline void mul(SVM_REGS dst, cell_t value) {
		write_instruction(SVM_OP_MUL, SVMI_ARG_IMM, dst, 0, 0, value);
	}
	inline void fmul(SVM_REGS dst, float value) {
		write_instruction(SVM_OP_MUL, SVMI_ARG_IMM, dst, 0, VMI_F_FP_OP, SVM_ftoc(value));
	}
	inline void fmul(SVM_REGS dst, SVM_REGS src) {
		mul(dst, src, VMI_F_FP_OP);
	}

	inline void div(SVM_REGS dst, SVM_REGS src, int flags = 0) {
		write_instruction(SVM_OP_DIV, SVMI_ARG_REG, dst, src, flags);
	}
	inline void fdiv(SVM_REGS dst, SVM_REGS src) {
		div(dst, src, VMI_F_FP_OP);
	}
	inline void div(SVM_REGS dst, cell_t value) {
		write_instruction(SVM_OP_DIV, SVMI_ARG_IMM, dst, 0, 0, value);
	}
	inline void fdiv(SVM_REGS dst, float value) {
		write_instruction(SVM_OP_DIV, SVMI_ARG_IMM, dst, 0, VMI_F_FP_OP, SVM_ftoc(value));
	}

	inline void mod(SVM_REGS dst, SVM_REGS src, int flags = 0) {
		write_instruction(SVM_OP_MOD, SVMI_ARG_REG, dst, src, flags);
	}
	inline void fmod(SVM_REGS dst, SVM_REGS src) {
		mod(dst, src, VMI_F_FP_OP);
	}
	inline void mod(SVM_REGS dst, cell_t value) {
		write_instruction(SVM_OP_MOD, SVMI_ARG_IMM, dst, 0, 0, value);
	}
	inline void fmod(SVM_REGS dst, float value) {
		write_instruction(SVM_OP_MOD, SVMI_ARG_IMM, dst, 0, VMI_F_FP_OP, SVM_ftoc(value));
	}

	inline void shl(SVM_REGS dst, SVM_REGS src) {
	}

	inline void shr(SVM_REGS dst, SVM_REGS src) {
	}


	inline void and_(SVM_REGS dst, SVM_REGS src) {
	}

	inline void and_(SVM_REGS dst, cell_t value) {
	}


	inline void or_(SVM_REGS dst, SVM_REGS src) {
	}

	inline void or_(SVM_REGS dst, cell_t value) {
	}


	inline void xor_(SVM_REGS dst, SVM_REGS src) {
	}

	inline void xor_(SVM_REGS dst, cell_t value) {
	}


	inline void not_(SVM_REGS dst, SVM_REGS src) {
	}

	inline void cmp(SVM_REGS dst, cell_t value) {
	}


	inline void jmp(SVM_REGS reg) {
	}

	inline void jmp(cell_t offset) {
	}


	inline void je(SVM_REGS reg) {
	}

	inline void je(cell_t offset) {
	}


	inline void jz(SVM_REGS reg) {
	}

	inline void jz(cell_t offset) {
	}


	inline void jne(SVM_REGS reg) {
	}

	inline void jne(cell_t offset) {
	}


	inline void jnz(SVM_REGS reg) {
	}

	inline void jnz(cell_t offset) {
	}


	inline void jl(SVM_REGS reg) {
	}

	inline void jl(cell_t offset) {
	}


	inline void jle(SVM_REGS reg) {
	}

	inline void jle(cell_t offset) {
	}


	inline void jg(SVM_REGS reg) {
	}

	inline void jg(cell_t offset) {
	}


	inline void jge(SVM_REGS reg) {
	}

	inline void jge(cell_t offset) {
	}


	inline void loop(SVM_REGS reg) {
	}

	inline void loop(cell_t offset) {
	}


	inline void push(SVM_REGS src) {
	}

	inline void push(cell_t value) {
	}

	inline void pop(SVM_REGS dst) {
	}

	inline void pop() {
	}


	inline void pushSR() {
	}

	inline void popSR() {
	}


	inline void call(SVM_REGS reg) {
	}

	inline void call(cell_t offset) {
	}


	inline void ncall(cell_t nativeidx) {
	}


	inline void ret() {
	}


	inline void brk() {
	}


	inline void halt() {
	}


	/* get code */
	inline size_t get_code_size() {
	}

	inline uint8_t* get_code() {
	}
};