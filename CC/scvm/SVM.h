/**
* simple virtual machine
*/
#pragma once
#include <stdint.h>
#include "bitop.h"

/* processor state flags */
#define VMSRF_ZF (1 << 0) /*< zero flag */
#define VMSRF_SF (1 << 1) /*< sign flag */
#define VMSRF_CF (1 << 2) /*< carry flag */

/* cell registers and stack align type */
typedef int cell_t;
typedef unsigned int ucell_t;

#define SVM_CELL_SIZE sizeof(cell_t)

/* float to cell */
#define SVM_ftoc(x) (*((cell_t *)&x))
#define SVM_ctof(x) (*((float *)&x))

/* SVM registers */
enum SVM_REGS : uint8_t {
	/* general purpose registers */
	SVM_REG_A, SVM_REG_B, SVM_REG_C, SVM_REG_D,
	SVM_REG_X, SVM_REG_Y, SVM_REG_Z, SVM_REG_W,
	SVM_REG_SR,

	/* IP */
	SVM_REG_IP,
	SVM_REG_SP,
	SVM_REG_CS,
	SVM_REG_DS,
	SVM_REG_SS
};

/* instruction flags */
enum SVMI_ARG_TYPE {
	SVMI_ARG_REG = 0,
	SVMI_ARG_ADDR,
	SVMI_ARG_IMM
};

/**
* instruction structure
* 
* 16 bit [0 -15] - operation code
* 2  bit [16-17] - mode (SVMI_ARG_REG|SVMI_ARG_ADDR|SVMI_ARG_IMM)
* 4  bit [18-21] - dest register
* 4  bit [22-25] - source register
* 6  bit [26-31] - flags
*/

#define VM_I_SIZE         (sizeof(int)) // VM each instruction size
#define VM_I_OPC_BITS     (16)
#define VM_I_MODE_BITS    (2)
#define VM_I_REG_DIR_BITS (4)

enum SVM_OP : uint32_t
{
	SVM_OP_NOP = 0,
	SVM_OP_MOV,
	SVM_OP_ADD,
	SVM_OP_INC,
	SVM_OP_SUB,
	SVM_OP_DEC,
	SVM_OP_MUL,
	SVM_OP_DIV,
	SVM_OP_MOD,  //TODO: K.D. FMOD NOT IMPLEMENTED IN VM!

	SVM_OP_SHL,
	SVM_OP_SHR,
	SVM_OP_AND,
	SVM_OP_OR,
	SVM_OP_XOR,

	SVM_OP_NOT,
	SVM_OP_CMP,

	SVM_OP_JMP,
	SVM_OP_JZ,
	SVM_OP_JNZ,
	SVM_OP_JE,
	SVM_OP_JNE,
	SVM_OP_JL,
	SVM_OP_JLE,
	SVM_OP_JG,
	SVM_OP_JGE,

	SVM_OP_LOOP,

	SVM_OP_CASE,

	SVM_OP_PUSH,
	SVM_OP_PUSHSR,
	SVM_OP_POP,
	SVM_OP_POPSR,

	SVM_OP_CALL,
	SVM_OP_NCALL,
	SVM_OP_RET,

	SVM_REG_BRK,
	SVM_REG_HALT,

	SVM_REG_STOI,
	SVM_REG_ITOS,
	SVM_REG_FTOI,
	SVM_REG_ITOF,
	SVM_REG_ROUND,
	SVM_REG_CEIL,
	SVM_REG_FLOOR
};

struct SVM_instruction {
	SVM_OP opcode;
	int    mode;
	int    rsrc;
	int    rdst;
	int    flags;
};

/* instruction flags */
#define VMI_F_FP_OP (1 << 0) //is floating point ariphmetical operation?

inline void SVM_instr_write(int& insruction, const SVM_instruction& src)
{
	insruction = 0;
	WRITE_BITS(insruction, src.opcode, 0, 15);
	WRITE_BITS(insruction, src.mode, 16, 17);
	WRITE_BITS(insruction, src.rsrc, 18, 21);
	WRITE_BITS(insruction, src.rdst, 22, 25);
	WRITE_BITS(insruction, src.flags, 26, 31);
}

inline void SVM_instr_fetch(SVM_instruction& dst, int instruction)
{
	dst.opcode = (SVM_OP)READ_BITS(instruction, 0, 15);
	dst.mode = READ_BITS(instruction, 16, 17);
	dst.rsrc = READ_BITS(instruction, 18, 21);
	dst.rdst = READ_BITS(instruction, 22, 25);
	dst.flags = READ_BITS(instruction, 26, 31);
}