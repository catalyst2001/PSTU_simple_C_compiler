#pragma once
#include "scccom.h" //common defs
#include "scclex.h" //c lexer
#include <map>

enum CCOBJ_CAT {
	CCOBJ_CAT_BUILTIN = 0,
	CCOBJ_CAT_CUSTOM
};

/* BASIC OBJECT FOR ALL !! */
#define CCOBJ_EMPTY  (0)
#define CCOBJ_IS_STD (1 << 0)

#define CCOBJ_CATG_NONE (0)

class cc_entity
{
	int flags;
	int ocat;
public:
	cc_entity() : flags(CCOBJ_EMPTY), ocat(CCOBJ_CATG_NONE) {}
	~cc_entity() {}

	inline int get_flags() { return flags; }
	inline int get_object_category() { return ocat; }

protected:
	inline void set_flags(int f) { flags = f; }
	inline void set_object_category(int oc) { ocat = oc; }
};

template<size_t size>
class cc_named {
	char name[size];
public:
	cc_named() { name[0] = 0; }
	cc_named(const char *p_str) {
		set_name(p_str);
	}
	~cc_named() {}

protected:
	void set_name(const char *p_str) { 
		strncpy(name, p_str, sizeof(name));
	}

	const char *nameprintf(const char* f, ...) {
		va_list aptr;
		va_start(aprt, f);
		vsnprintf(name, sizeof(name), f, aptr);
		va_end(aptr);
		return name;
	}

	void set_zeroname() {
		name[0] = 0;
	}

public:
	inline size_t      get_namemaxlen() { return size; }
	inline const char* get_name() { return name; }
	inline bool        is_noname() { return !get_name()[0]; }
};

/* named object */
template<size_t nsize>
class cc_named_object : public cc_entity, public cc_named<nsize>
{
public:
	cc_named_object() {}
	~cc_named_object() {}

};

/**
* ============================================================
* TYPES
* ============================================================
*/
/* typedecl flags */
#define CCT_DEF   (0)
#define CCT_IS_CUSTOM (1 << 0)
#define CCT_NUM_IS_UNSIGNED (1 << 1)

#define CCTD_NAME_MAX_LEN 64

/* CC typedecl types */
enum CCTDT {
	CCTDT_TYPE_EMPTY,
	CCTDT_TYPE_DEFAULT = 0,
	CCTDT_TYPE_PTR,
	CCTDT_TYPE_FUNCPTR,
	CCTDT_TYPE_INT,
	CCTDT_TYPE_FP
};

/* typedecls */
class cc_typedecl : public cc_entity, public cc_named<CCTD_NAME_MAX_LEN>
{
protected:
	CCTDT  type;
	size_t size;
	int    typeidx;

	void set_decltype(CCTDT dt) { type = dt; }
	void set_typesize(size_t s) { size = s; }
	void set_typeidx(int idx) { typeidx = idx; }

public:
	cc_typedecl() : type(CCTDT_TYPE_EMPTY), size(0), typeidx(-1) {}
	cc_typedecl(const char *p_typename, CCTDT t, size_t tsize, int tidx, bool is_unsigned = true) : type(t), size(tsize), typeidx(tidx) {
		assert(p_typename && "TYPENAME MUST BE SET!");
		set_name(p_typename);
		if (is_unsigned) {
			/* set type unsigned */
			set_flags(get_flags()|CCT_NUM_IS_UNSIGNED);
		}
	}
	~cc_typedecl() {}

	const CCTDT get_typedecltype() { return type; }
	size_t      get_typesize() { return size; }
	int         get_typeidx() { return typeidx; }
	inline bool is_without_typeidx() { return get_typeidx() == -1; }

	inline bool is_emptydecl() { return get_typedecltype() == CCTDT_TYPE_EMPTY; }
	inline bool is_default() { return get_typedecltype() == CCTDT_TYPE_DEFAULT; }
	inline bool is_ptr() { return get_typedecltype() == CCTDT_TYPE_PTR; }
	inline bool is_funcptr() { return get_typedecltype() == CCTDT_TYPE_FUNCPTR; }
	inline bool is_unsigned() { return get_flags() & CCT_NUM_IS_UNSIGNED; }
};

/**
* =========================================================
* SCOPE
* =========================================================
*/
class cc_scope : public cc_entity {
	cc_entity* p_assignto;
public:
	cc_scope() {}
	~cc_scope() {}

};

/*
*========================================================
* VARS 
*= =======================================================
*/
#define MAX_VARNAME 128

#define CCVAR_IS_NONE   (0)
#define CCVAR_IS_GLOBAL (1 << 0)
#define CCVAR_IS_ARRAY (1 << 1)
class cc_var : public cc_entity, public cc_named<MAX_VARNAME>
{
	int          varflags;
	size_t       adim; //array dimension
	cc_typedecl* p_assigned_typedecl;
public:
	cc_var(const char *p_varname, cc_typedecl* p_type, size_t dim = 1, int flags = CCVAR_IS_NONE) {
		varflags = flags;
		adim = dim;
		if (p_varname) {
			set_name(p_varname);
		}
		else {
			set_zeroname();
		}
	}
	~cc_var() {}
	inline cc_typedecl* get_type() { return p_assigned_typedecl; }

	inline bool is_global() { return varflags & CCVAR_IS_GLOBAL; }
	inline bool is_array() { return varflags & CCVAR_IS_ARRAY; }
	inline size_t get_dim() { return adim; }

	/* is flexible array?? */
	inline bool is_FA() { return adim==0; }

	/* get full var or array size */
	inline size_t get_size_in_bytes() {
		return get_dim() * get_type()->get_typesize();
	}

};

/**
* =========================================================
* FUNCTIONS
* =========================================================
*/
#define MAX_FUNC_NAME_LEN 256
#define MAX_FUNC_ARG_NAME_LENGTH 64
// nfargs - no limited

/* typedecls */
class cc_func : public cc_entity, public cc_named<MAX_FUNC_NAME_LEN>
{
protected:
	using ccfargs = vec_cc<cc_var*>;
	using cclvars = vec_cc<cc_var*>;
	ccfargs func_args;
	cclvars func_lvars;
	int     checksum;

public:
	/* conmstructor */
	cc_func(const char* p_funcname, size_t argscount = 0) {
		set_name(p_funcname);
		checksum = 0;
		if (argscount) {
			func_args.reserve(argscount);
		}
	}
	~cc_func() {}
	
	/* ARGS */
	void add_funcarg(cc_var* p_farg) { func_args.push_back(p_farg); }
	cc_var* new_funcarg(const char* p_argname, cc_typedecl* p_type, size_t dim = 1, int flags = CCVAR_IS_NONE) {
		cc_var* p_farg = new cc_var(p_argname, p_type, dim, flags);
		add_funcarg(p_farg);
		return p_farg;
	}
	size_t num_fargs() { return func_args.size(); }
	cc_var* get_farg(size_t idx) {
		assert(idx < func_args.size() && "get_farg(): idx out of bounds func_args!");
		return func_args[idx];
	}

	/* LVARS */
	void add_localvar(cc_var* p_lvar) { func_lvars.push_back(p_lvar); }
	cc_var* new_localvar(const char* p_varname, cc_typedecl* p_type, size_t dim = 1, int flags = CCVAR_IS_NONE) {
		cc_var* p_var = new cc_var(p_varname, p_type, dim, flags);
		add_localvar(p_var);
		return p_var;
	}
	size_t num_lvars() { return func_lvars.size(); }
	cc_var* get_lvar(size_t idx) {
		assert(idx < func_lvars.size() && "get_lvar(): idx out of bounds func_lvars!");
		return func_lvars[idx];
	}

	size_t stackframe_size() {
		size_t i, stack_frame_total_size = 0;
		/* fargs */
		for (i = 0; i < num_fargs(); i++)
			stack_frame_total_size += get_farg(i)->get_size_in_bytes();
		/* lvars */
		for (i = 0; i < num_lvars(); i++)
			stack_frame_total_size += get_lvar(i)->get_size_in_bytes();
		
		return stack_frame_total_size;
	}

	void gen_checksum() {
		CCUNIMPL
	}
	const int get_checksum() {
		CCUNIMPL
			return checksum;
	}
};

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

