#pragma once
#include <stdint.h>
/**
* simple virtual machine
*
*
*/

/* SVM registers */
enum SVM_REGS : uint8_t {
	/* general purpose registers */
	SVM_REG_A, SVM_REG_B, SVM_REG_C, SVM_REG_D,

	/* IP */
	SVM_REG_IP, SVM_REG_SP,
	SVM_REG_CS, SVM_REG_DS, SVM_REG_SS, SVM_REG_HS


};

/* SVM instruction arg */
struct _svm_instruction_arg {

};