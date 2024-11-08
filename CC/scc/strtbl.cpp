#include "strtbl.h"

strtbl::strtbl() : m_nbuf_size(0), m_pbuffer(nullptr)
{
}

strtbl::~strtbl()
{
}

bool strtbl::add_string(uint32_t* p_dst, const char* p_string)
{
	size_t next_buf_size;
	/* start offset = end_offset of last string */
	size_t string_length = strlen(p_string) + 1;
	m_strinf.start_offset = m_strinf.end_offset;
	next_buf_size = m_strinf.start_offset + string_length; //+1 '\0'
	if (next_buf_size >= m_nbuf_size) {
		m_nbuf_size = next_buf_size + string_length * 2; //preallocate last_size+string_length*2 size
		m_pbuffer = (char*)realloc(m_pbuffer, m_nbuf_size); //TODO: VS triggering warning here :)
		if (!m_pbuffer) {
			/* memory allocation failed */
			return false;
		}
		/* copy string to buffer */
		strncpy(&m_pbuffer[m_strinf.start_offset], p_string, string_length);
	}
	/* add string layout */
	m_strings.push_back(m_strinf);
	*p_dst = m_strinf.start_offset;
	return true;
}

size_t strtbl::get_size()
{
	return m_nbuf_size;
}

char* strtbl::get_data()
{
	return m_pbuffer;
}

bool strtbl::release()
{
	if (m_pbuffer) {
		free(m_pbuffer);
		m_pbuffer = nullptr;
		return true;
	}
	return false;
}
