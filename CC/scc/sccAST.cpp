#include "sccAST.h"

ast_root::ast_root() : ast_node(nullptr, AST_NODE_TYPE_NONE)
{
}

ast_root::~ast_root()
{
}

ASTBUILD_STATUS ast_root::build(scclex& lexer)
{
  scclex_tok    tok;
  SCCLEX_STATUS status;
  scctp_ctx     parserctx;

  /* init C language constructions detector */
  while (1) {
    status = lexer.next_tok(tok);
    if (status == SCCLEX_STATUS_INVALID_CHAR) {
      printf("invalid character");
      return SCCAST_BUILDER_STATUS_INPUT_ERROR;
    }

    /* end of buffer? */
    if (status == SCCLEX_STATUS_NO_MORE_DATA)
      break; /* finish AST build */

    /* keyword found */
    if (tok.tok == SCCT_KEYWORD) {
      /* start var decl or function prototype/decl */
      if (LEX_KW_IS_TYPE(tok.kw)) {

      }
    }

  }
  return SCCAST_BUILDER_STATUS_OK;
}

ASTBUILD_STATUS ast_root::shutdown()
{
  return ASTBUILD_STATUS();
}

ast_typedecl* ast_root::decl_type(const char* p_typename, CCTDT t, size_t tsize, int tidx, bool is_unsigned)
{
  ast_typedecl* p_typedecl = new ast_typedecl(this, p_typename, t, tsize, tidx, is_unsigned);
  m_types.push_back(p_typedecl);
  m_typenamesht.emplace(p_typename, p_typedecl);
  return p_typedecl;
}

ast_typedecl* ast_root::find_type(const char* p_typename)
{
  return m_typenamesht[p_typename];
}

ast_func* ast_root::decl_func(const char* p_funcname, size_t argscount)
{
  ast_func* p_func = new ast_func(this, p_funcname, argscount);
  m_funcs.push_back(p_func);
  m_funcnamesht.emplace(p_funcname, p_func);
  return p_func;
}

ast_func* ast_root::find_func(const char* p_funcname)
{
  return m_funcnamesht[p_funcname];
}

ast_var* ast_root::decl_globalvar(const char* p_varname, ast_typedecl* p_type, size_t dim, int flags)
{
  ast_var* p_var = new ast_var(this, p_varname, p_type, dim, flags);
  m_gvars.push_back(p_var);
  m_gvarnamesht.emplace(p_varname, p_var);
  return p_var;
}

ast_var* ast_root::find_globalvar(const char* p_varname)
{
  return m_gvarnamesht[p_varname];
}
