#include "scc.h"
#include "outerr.h"

cc_typedecl *scc::decl_type(const char* p_typename, CCTDT t, size_t tsize, int tidx, bool is_unsigned)
{
  cc_typedecl* p_typedecl = new cc_typedecl(p_typename, t, tsize, tidx, is_unsigned);
  m_types.push_back(p_typedecl);
  m_typenamesht.emplace(p_typename, p_typedecl);
  return p_typedecl;
}

cc_typedecl* scc::find_type(const char* p_typename)
{
  return m_typenamesht[p_typename];
}

cc_func* scc::decl_func(const char* p_funcname, size_t argscount)
{
  cc_func* p_func = new cc_func(p_funcname, argscount);
  m_funcs.push_back(p_func);
  m_funcnamesht.emplace(p_funcname, p_func);
  return p_func;
}

cc_func* scc::find_func(const char* p_funcname)
{
  return m_funcnamesht[p_funcname];
}

cc_var* scc::decl_globalvar(const char* p_varname, cc_typedecl* p_type, size_t dim, int flags)
{
  cc_var* p_var = new cc_var(p_varname, p_type, dim, flags);
  m_gvars.push_back(p_var);
  m_gvarnamesht.emplace(p_varname, p_var);
  return p_var;
}

cc_var* scc::find_globalvar(const char* p_varname)
{
  return m_gvarnamesht[p_varname];
}

bool scc::init_builtin_types()
{
  cc_typedecl* p_type;
  try {
    decl_type("bool", CCTDT_TYPE_INT, 1, -1, true);
    decl_type("char", CCTDT_TYPE_INT, 1, -1, false);
    decl_type("short", CCTDT_TYPE_INT, 2, -1, false);
    decl_type("int", CCTDT_TYPE_INT, 4, -1, false);
    decl_type("long", CCTDT_TYPE_INT, 4, -1, false);
    decl_type("float", CCTDT_TYPE_FP, 4, -1, false);
    return true;
  }
  catch (...) {
    return false;
  }
}

bool scc::run(int argc, char** argv)
{
  memfile        memf;
  MEMFILE_STATUS mfstatus;
  SCCLEX_STATUS  lexstatus;

  /* print hello screen */
  cc_msg::phello();
  if (argc == 1) {
    cc_msg::msg(CCMSG_TYPE_ERROR, "no input files!");
    return 0;
  }

  /* load file from disk */
  mfstatus = memf.load(argv[1]);
  if (!MF_OK(mfstatus)) {
    cc_msg::msg(CCMSG_TYPE_ERROR, "load error. %s", memfile::status_to_string(mfstatus));
    return 0;
  }

  /* PASS1. run lexer analyze and build token tree */
  lexstatus = lexer.init(memf);
  if (!LEX_OK(lexstatus)) {
    cc_msg::msg(CCMSG_TYPE_ERROR, "lexer initializing failed");
    memf.release();
    return 0;
  }

  /* PASS2. build parse tree */
  if (!init_builtin_types()) {
    cc_msg::msg(CCMSG_TYPE_INTRERR, "init_builtin_types() failed");
    return false;
  }

  /* PASS3. build control-flow graph */
  //TODO: SKIPPED

  /* PASS4. run code emitter */


  return true;
}
