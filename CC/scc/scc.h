#pragma once
#include "scccom.h" //common defs
#include "scclex.h" //c lexer

class scc
{
	/* GLOBAL CONFIGURATION */
	size_t stack_reserve;
	size_t heap_reserve;

	/* DECLARATIONS */
	vec_cc<cc_typedecl *> m_types; //type decls
	std::map<std::string, cc_typedecl*> m_typenamesht;
	vec_cc<cc_func *>     m_funcs; //funcs
	std::map<std::string, cc_func*> m_funcnamesht;
	vec_cc<cc_var *>      m_gvars; //globalvars
	std::map<std::string, cc_var*> m_gvarnamesht;

	scclex lexer;

protected:
	/* decl type */
	cc_typedecl *decl_type(const char* p_typename, CCTDT t, size_t tsize, int tidx, bool is_unsigned);
	cc_typedecl *find_type(const char* p_typename);
	
	/* decl funcs */
	cc_func     *decl_func(const char* p_funcname, size_t argscount = 0);
	cc_func     *find_func(const char* p_funcname);

	/* decl globalvars */
	cc_var* decl_globalvar(const char* p_varname, cc_typedecl* p_type, size_t dim = 1, int flags = CCVAR_IS_NONE);
	cc_var* find_globalvar(const char* p_varname);
protected:
	/* init functions */
	bool init_builtin_types();

protected:


	/* MAIN*/
	bool run(int argc, char **argv);
};

