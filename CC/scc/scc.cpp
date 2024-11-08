#include "scc.h"
#include "outerr.h"

void scc::register_linkers()
{
  iscclinkmgr* p_linkersmgr;
  p_linkersmgr = scc_get_linkmgr();
  
}

scc::scc()
{
  /* regster linkers */
  scc::register_linkers();
}

scc::~scc()
{
}

bool scc::run(int argc, char** argv)
{
  memfile         memf;
  scclex          lexer;
  iscclink       *p_linker;
  union {
    MEMFILE_STATUS  mfstatus;
    SCCLEX_STATUS   lexstatus;
    ASTBUILD_STATUS aststatus;
  };

  //HACKHACK: PARSE COMMAND LINE ARGS HERE !!!!!!!!!!!!!!!!!!!!
  
  iscclinkmgr* p_linkersmgr = scc_get_linkmgr();
  p_linker = p_linkersmgr->get_linker("svm_image_v1"); //HACKHACK: svm_image_v1 retrived from command line defined here
  if (!p_linker) {
    cc_error("unknown target image type");
    return 0;
  }

  /* process args */
  if (argc == 1) {
    cc_error("no input files!");
    return 0;
  }

  /* load file from disk */
  mfstatus = memf.load(argv[1]);
  if (!MF_OK(mfstatus)) {
    cc_error(cc_format("load error. %s", memfile::status_to_string(mfstatus)));
    return 0;
  }

  /* PASS0. run preprocessor */
  //TODO: SKIPPED preprocessor

  /* PASS1. run lexer */
  lexstatus = lexer.init(memf);
  if (!LEX_OK(lexstatus)) {
    cc_error("lexer initializing failed");
    memf.release();
    return 0;
  }

  /* PASS2. build abstract syntax tree */
  m_pAST_root = new (std::nothrow)ast_root();
  if (!m_pAST_root) {
    cc_fatal("AST memory allocation failed");
    memf.release();
    return 1;
  }

  /* declare standard types */
  try {
    m_pAST_root->decl_type("bool", CCTDT_TYPE_INT_NUMB, 1, -1, true);
    m_pAST_root->decl_type("char", CCTDT_TYPE_INT_NUMB, 1, -1, false);
    m_pAST_root->decl_type("uchar", CCTDT_TYPE_INT_NUMB, 1, -1, true);
    m_pAST_root->decl_type("short", CCTDT_TYPE_INT_NUMB, 2, -1, false);
    m_pAST_root->decl_type("ushort", CCTDT_TYPE_INT_NUMB, 2, -1, true);
    m_pAST_root->decl_type("int", CCTDT_TYPE_INT_NUMB, 4, -1, false);
    m_pAST_root->decl_type("uint", CCTDT_TYPE_INT_NUMB, 4, -1, true);
    m_pAST_root->decl_type("float", CCTDT_TYPE_FP_NUMB, 4, -1, false);
    m_pAST_root->decl_type("double", CCTDT_TYPE_FP_NUMB, 8, -1, false);
  }
  catch (...) {
    cc_fatal("standard type declarations failed. exception occurred");
    memf.release();
    return false;
  }

  /* build abstract syntax tree from lexer tokens */
  aststatus = m_pAST_root->build(lexer);
  if (!AST_OK(aststatus)) {
    switch (aststatus)
    {
    case SCCAST_BUILDER_STATUS_SYNTAX_ERROR:
      cc_error("compilation stopped");
      memf.release();
      return false;
      
    case SCCAST_BUILDER_STATUS_INPUT_ERROR:
      cc_error("invalid char detected");
      memf.release();
      return false;
    }
  }

  /* PASS3. build control-flow graph */
  //TODO: SKIPPED

  /* PASS4. run code emitter */
  m_pcode_emmitter = new (std::nothrow)sccemit();
  m_pcode_emmitter->emit_code(m_pAST_root);

  return true;
}
