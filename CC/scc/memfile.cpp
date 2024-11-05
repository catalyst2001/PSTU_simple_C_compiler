#include "memfile.h"
#include <exception>
#include <assert.h>

memfile::memfile() : m_ptext(nullptr), m_nsize(0)
{
}

memfile::~memfile()
{
}

MEMFILE_STATUS memfile::load(const char* p_file_path)
{
  FILE          *fp;
  size_t         n_elems;
  MEMFILE_STATUS status;

#define MEMF_ERR_EXIT(s) do {\
    status = s;\
    goto __failure;\
  } while(0);
  
  fp = fopen(p_file_path, "rb");
  if (!fp)
    return MEMFILE_STATUS_OPEN_FAILED;

  fseek(fp, 0, SEEK_END);
  m_nsize = (size_t)ftell(fp);
  rewind(fp);
  if (!m_nsize)
    MEMF_ERR_EXIT(MEMFILE_STATUS_EMPTY)

  m_ptext = new (std::nothrow)char[m_nsize];
  if (!m_ptext)
    MEMF_ERR_EXIT(MEMFILE_STATUS_OUT_OF_MEMORY)

  /* load data from file */
  n_elems = fread(m_ptext, 1, m_nsize, fp);
  if (n_elems != m_nsize)
    MEMF_ERR_EXIT(MEMFILE_STATUS_IO_READ_FAILED)

  return MEMFILE_STATUS_OK;

  /* errors handling */
__failure:
  m_nsize = 0;
  if (fp)
    fclose(fp);

  memfile::release();
  return status;
#undef MEMF_ERR_EXIT
}

MEMFILE_STATUS memfile::release()
{
  m_nsize = 0;
  if (m_ptext) {
    delete[] m_ptext;
    m_ptext = nullptr;
  }
  return MEMFILE_STATUS_OK;
}

bool memfile::is_empty()
{
  return !!m_nsize;
}

char* memfile::get_text()
{
  return m_ptext;
}

size_t memfile::get_text_size()
{
  return m_nsize;
}

const char* memfile::status_to_string(MEMFILE_STATUS s)
{
#define DECL_MSG(s, m) m
  static const char* p_msgs[MEMFILE_STATUS_LAST] = {
    DECL_MSG(MEMFILE_STATUS_OK, "no errors"),
    DECL_MSG(MEMFILE_STATUS_OPEN_FAILED, "failed to open file"),
    DECL_MSG(MEMFILE_STATUS_EMPTY, "empty file"),
    DECL_MSG(MEMFILE_STATUS_OUT_OF_MEMORY, "out of memory. file is too big?!"),
    DECL_MSG(MEMFILE_STATUS_IO_READ_FAILED, "I/O error. data reading failed")
  };
  assert((int)s < MEMFILE_STATUS_LAST && "memfile::status_to_string(): status out of bounds");
  return p_msgs[(int)s];
#undef DECL_MSG
}
