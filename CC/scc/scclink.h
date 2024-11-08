#pragma once
#include "sccAST.h"
#include "strtbl.h"

#define SCC_MAX_IMAGE_LINKERS 16

/* linker complete coees */
enum SCCLINK_STATUS {
	SCCLINK_STATUS_OK = 0,
	SCCLINK_STATUS_IO_WRITE_ERROR,
	SCCLINK_STATUS_OUT_OF_MEMORY,
	SCCLINK_STATUS_UNKNOWN_IMAGE,
	SCCLINK_STATUS_LINKERS_LIMIT_EXCEEDED
};

/* primary image generator interface */
class iscclink
{
public:
	virtual SCCLINK_STATUS init(ast_root *p_AST, strtbl *p_strtable) = 0;
	virtual SCCLINK_STATUS shutdown() = 0;
	virtual SCCLINK_STATUS set_prop(const char *p_prop, int value) = 0;
	virtual SCCLINK_STATUS link() = 0;
	virtual const char* get_name() = 0;
};

/* linkers manager for multiple image types */
class iscclinkmgr
{
public:
	virtual iscclink       *get_linker(const char *p_linker) = 0;
	virtual SCCLINK_STATUS  register_linker(iscclink *p_linker) = 0;
};

iscclinkmgr* scc_get_linkmgr();