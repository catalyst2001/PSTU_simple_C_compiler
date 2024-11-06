#include "textbuf.h"
#include "scccom.h"

#define WINDOWS_CR "\r\n"
#define WINDOWS_CR_SIZE (sizeof(WINDOWS_CR) - 1)

char* scc_textparser::get_curr_address()
{
	return m_ppos;
}

size_t scc_textparser::try_transform_CRLF_to_LF(char* p_input, size_t* p_size)
{
	/* replace all Windows CRLF '\r\n' to UNIX LF '\n' */
	char* p_endaddr = p_input + *p_size;
	size_t n_lines = 0;
	size_t n_bytes_to_copy;
	while (p_input < p_endaddr) {
		char* p_crlf = strstr(p_input, WINDOWS_CR);
		if (!p_crlf || p_crlf >= p_endaddr) 
			break; /* end of buffer */

		/* replace '\r\n' to '\n' */
		*p_crlf = '\n';  // rewrite '\r' to '\n'
		n_bytes_to_copy = (size_t)(p_endaddr - (p_crlf + WINDOWS_CR_SIZE));
		memmove(p_crlf+1, p_crlf+WINDOWS_CR_SIZE, n_bytes_to_copy);
		p_endaddr--; //update  end address of buffer (removed '\r')
		(*p_size)--; //decrement buffer size
		n_lines++; //new line
		p_input = p_crlf + 1; //  move to next char
	}
	return n_lines;
}

scc_textparser::scc_textparser() : m_nsize(0), m_nlines(1), m_ncurr_line(1), m_base(nullptr), m_pend(nullptr), m_ppos(nullptr)
{
}

scc_textparser::~scc_textparser()
{
}

bool scc_textparser::init(char* p_text_base, size_t n_textbuf_size)
{
	/* check parameters for valid */
	if (!p_text_base || !n_textbuf_size)
		return false;

	m_nsize = n_textbuf_size;
	m_base = p_text_base;
	m_nlines = try_transform_CRLF_to_LF(m_base, &m_nsize);
	m_pend = m_base + m_nsize;
	m_ppos = m_base; //position starts at buffer begin
	return true;
}

size_t scc_textparser::get_num_lines()
{
	return m_nlines;
}

bool scc_textparser::is_initialized()
{
	return !!m_base;
}

void scc_textparser::reset_cursor()
{
	m_ppos = m_base;
}

void scc_textparser::get_context(scctp_ctx &dst)
{
	dst.current_line = m_ncurr_line;
	dst.p_position = m_ppos;
}

void scc_textparser::set_context(const scctp_ctx &src)
{
	m_ncurr_line = src.current_line;
	m_ppos = src.p_position;
}

bool scc_textparser::is_end()
{
	return m_ppos >= m_pend;
}

bool scc_textparser::pos_increment()
{
	m_ppos++; // increment position
	if (is_end())
		return false; // end of buffer

	/* check line */
	if(get_char() == '\n')
		m_ncurr_line++; //increment lines counter

	return true; //OK, processed
}

int scc_textparser::get_char()
{
	return *m_ppos;
}

bool scc_textparser::skip_spaces()
{
	while (get_char() && isspace(get_char())) {
		if (!is_end()) {
			pos_increment();
			continue;
		}
		return false;
	}
	return true;
}

SCCTP_STATUS scc_textparser::skip_all_to_substr(const char* p_substr, size_t substrlen)
{
	/* find substring */
	char* p_sstr = strstr(m_ppos, p_substr);
	if (!p_sstr) {
		/* is end of buffer? */
		if (is_end())
			return SCCTP_STATUS_END_OF_BUFFER;

		/* it is not end of buffer, substring not found */
		return SCCTP_STATUS_SUBSTR_NOT_FOUND;
	}

	/* precomputed size for optimize */
	if(!substrlen)
		substrlen = strlen(p_substr);

	p_sstr += substrlen; //move to end of substr
	increment_lines_count_to_current_pos(p_sstr); //count lines to current position
	return SCCTP_STATUS_OK;
}

bool scc_textparser::skip_all_to_nextline()
{
	while (get_char() && get_char() != '\n') {
		if (!is_end()) {
			pos_increment();
			continue;
		}
		return false;
	}
	return true;
}

bool scc_textparser::increment_lines_count_to_current_pos(char* p_pos)
{
	/* increment position if 'p_pos' address greater 'm_ppos' */
	while ((get_char() && get_char() != '\n') && (get_curr_address() < p_pos)) {
		if (!is_end()) {
			pos_increment();
			continue;
		}
		return false;
	}
	return true;
}
