#pragma once
#include <map>
#include "scccom.h"

enum AST_NODE_TYPE {
	AST_NODE_TYPE_NONE = 0,
	AST_NODE_TYPE_OPER, //+, -, 
	AST_NODE_TYPE_VAR,
	AST_NODE_TYPE_FUNC,
	AST_NODE_TYPE_FUNC_CALL,
	AST_NODE_TYPE_SCOPE,
	AST_NODE_TYPE_EXP,
	AST_NODE_TYPE_TYPE,
	AST_NODE_TYPE_CYCLEFOR,
	AST_NODE_TYPE_CYCLEWHILE,
	AST_NODE_TYPE_CONDITION,
	AST_NODE_TYPE_SWITCH_CASE,
	AST_NODE_TYPE_BREAK,
	AST_NODE_TYPE_CONTINUE,
	AST_NODE_TYPE_CAST,
	AST_NODE_TYPE_RETURN,
	AST_NODE_TYPE_LIT
};

/**
* AST entity base class
*/
#define ASTENT_EMPTY  (0)
#define ASTENT_IS_STD (1 << 0)
#define ASTENT_CATG_NONE (0)

class ast_node
{
	using astnodes = std::vector<ast_node*>;
	int           flags;
	ast_node     *m_pparent;
	AST_NODE_TYPE m_node_type;
	astnodes      m_child_nodes;
public:
	ast_node(ast_node *pparent, AST_NODE_TYPE type) : flags(ASTENT_EMPTY), m_node_type(type), m_pparent(pparent) {}
	~ast_node() {}

	/* modifiers */
	inline void          set_parent(ast_node* pparent) { m_pparent = pparent; }

	/* selectors */
	inline int           get_flags() { return flags; }
	inline AST_NODE_TYPE get_node_type() { return m_node_type; }
	inline ast_node     *get_parent() { return m_pparent; }

protected:
	inline void          set_flags(int f) { flags = f; }
	inline void          set_node_type(AST_NODE_TYPE nt) { m_node_type = nt; }
};

/**
* names helper
*/
template<size_t size>
class ast_named {
	char name[size];
public:
	ast_named() { name[0] = 0; }
	ast_named(const char* p_str) {
		set_name(p_str);
	}
	~ast_named() {}

protected:
	void set_name(const char* p_str) {
		strncpy(name, p_str, sizeof(name));
	}

	const char* nameprintf(const char* f, ...) {
		va_list aptr;
		va_start(aptr, f);
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
class scc_ast_named_node : public ast_node, public ast_named<nsize>
{
public:
	scc_ast_named_node() {}
	~scc_ast_named_node() {}
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
	CCTDT_TYPE_INT_NUMB,
	CCTDT_TYPE_FP_NUMB
};

/* typedecls */
class ast_typedecl : public ast_node, public ast_named<CCTD_NAME_MAX_LEN>
{
protected:
	CCTDT  type;
	size_t size;
	int    typeidx;

	void set_decltype(CCTDT dt) { type = dt; }
	void set_typesize(size_t s) { size = s; }
	void set_typeidx(int idx) { typeidx = idx; }

public:
	ast_typedecl(ast_node *) : ast_node(), type(CCTDT_TYPE_EMPTY), size(0), typeidx(-1) {}
	ast_typedecl(const char* p_typename, CCTDT t, size_t tsize, int tidx, bool is_unsigned = true) : type(t), size(tsize), typeidx(tidx) {
		assert(p_typename && "TYPENAME MUST BE SET!");
		set_name(p_typename);
		if (is_unsigned) {
			/* set type unsigned */
			set_flags(get_flags() | CCT_NUM_IS_UNSIGNED);
		}
	}
	~ast_typedecl() {}

	const CCTDT get_decltype() { return type; }
	size_t      get_typesize() { return size; }
	int         get_typeidx() { return typeidx; }
	inline bool is_without_typeidx() { return get_typeidx() == -1; }

	inline bool is_emptydecl() { return get_decltype() == CCTDT_TYPE_EMPTY; }
	inline bool is_default() { return get_decltype() == CCTDT_TYPE_DEFAULT; }
	inline bool is_ptr() { return get_decltype() == CCTDT_TYPE_PTR; }
	inline bool is_funcptr() { return get_decltype() == CCTDT_TYPE_FUNCPTR; }
	inline bool is_unsigned() { return get_flags() & CCT_NUM_IS_UNSIGNED; }
};

/**
* =========================================================
* SCOPE
* =========================================================
*/
class ast_scope : public ast_node {
	ast_node* p_assignto;
public:
	ast_scope() {}
	~ast_scope() {}

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
class ast_var : public ast_node, public ast_named<MAX_VARNAME>
{
	int           varflags;
	size_t        adim; //array dimension
	ast_typedecl* p_assigned_typedecl;
public:
	ast_var(const char* p_varname, ast_typedecl* p_type, size_t dim = 1, int flags = CCVAR_IS_NONE) {
		p_assigned_typedecl = p_type;
		varflags = flags;
		adim = dim;
		if (p_varname) {
			set_name(p_varname);
		}
		else {
			set_zeroname();
		}
	}
	~ast_var() {}

	/* selectors */
	inline ast_typedecl* get_type() { return p_assigned_typedecl; }
	inline size_t        get_dim() { return adim; }
	/* get full var or array size */
	inline size_t get_size_in_bytes() {
		return get_dim() * get_type()->get_typesize();
	}

	/* checkers */
	inline bool          is_global() { return varflags & CCVAR_IS_GLOBAL; }
	inline bool          is_array() { return varflags & CCVAR_IS_ARRAY; }

	/* is flexible array?? */
	inline bool is_FA() { return adim == 0; }
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
class ast_func : public ast_node, public ast_named<MAX_FUNC_NAME_LEN>
{
protected:
	using ccfargs = vec_cc<ast_var*>;
	using cclvars = vec_cc<ast_var*>;
	ccfargs func_args;
	cclvars func_lvars;
	int     checksum;

public:
	/* conmstructor */
	ast_func(ast_node *p_parent, const char* p_funcname, size_t argscount = 0) {
		set_parent(p_parent);
		set_name(p_funcname);
		checksum = 0;
		if (argscount) {
			func_args.reserve(argscount);
		}
	}
	~ast_func() {}

	/* ARGS */
	void add_funcarg(ast_var* p_farg) { func_args.push_back(p_farg); }
	ast_var* new_funcarg(const char* p_argname, ast_typedecl* p_type, size_t dim = 1, int flags = CCVAR_IS_NONE) {
		ast_var* p_farg = new ast_var(p_argname, p_type, dim, flags);
		add_funcarg(p_farg);
		return p_farg;
	}
	size_t num_fargs() { return func_args.size(); }
	ast_var* get_farg(size_t idx) {
		assert(idx < func_args.size() && "get_farg(): idx out of bounds func_args!");
		return func_args[idx];
	}

	/* LVARS */
	void add_localvar(ast_var* p_lvar) { func_lvars.push_back(p_lvar); }
	ast_var* new_localvar(const char* p_varname, ast_typedecl* p_type, size_t dim = 1, int flags = CCVAR_IS_NONE) {
		ast_var* p_var = new ast_var(p_varname, p_type, dim, flags);
		add_localvar(p_var);
		return p_var;
	}
	size_t num_lvars() { return func_lvars.size(); }
	ast_var* get_lvar(size_t idx) {
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

class sccAST
{
};

