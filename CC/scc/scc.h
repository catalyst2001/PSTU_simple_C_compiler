/**
* Simple C compiler
* 
* Main compiler file
* 
* 
*/
#pragma once
#include "scccom.h" // common defs
#include "scclex.h" // c lexer
#include "sccAST.h" // abstract syntax tree
#include "sccemit.h" // code emmitter
#include "scclink.h" // linkers manager

/* specified linkers */


class scc
{
	/* GLOBAL CONFIGURATION */
	size_t    stack_reserve;
	size_t    heap_reserve;
	ast_root *m_pAST_root;
	sccemit  *m_pcode_emmitter;

protected:
	void register_linkers();

public:
	scc();
	~scc();

	/* MAIN*/
	bool run(int argc, char **argv);
};

