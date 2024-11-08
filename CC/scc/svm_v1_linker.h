#pragma once
#include "scclink.h"

/* SVM v1 linker immpl */
class svm_v1_linker : public iscclink
{
public:
	svm_v1_linker();
	~svm_v1_linker();
	virtual SCCLINK_STATUS init(ast_root* p_AST, strtbl* p_strtable);
	virtual SCCLINK_STATUS shutdown();
	virtual SCCLINK_STATUS set_prop(const char* p_prop, int value);
	virtual SCCLINK_STATUS link();
	virtual const char    *get_name();
};

