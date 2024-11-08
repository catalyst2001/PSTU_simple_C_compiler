#pragma once
#include "sccAST.h"

enum SCCEMIT_STATUS {
	SCCEMIT_STATUS_OK = 0,




};

class sccemit
{

protected:


public:
	sccemit();
	~sccemit();

	SCCEMIT_STATUS emit_code(ast_root* p_astroot);
	SCCEMIT_STATUS shutdown();



};

