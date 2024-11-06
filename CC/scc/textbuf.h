#pragma once

enum SCCTP_STATUS {
	SCCTP_STATUS_OK = 0,
	SCCTP_STATUS_SUBSTR_NOT_FOUND,
	SCCTP_STATUS_END_OF_BUFFER
};

class scc_textparser;

class scctp_ctx {
	friend class scc_textparser;
	char*  p_position;
	size_t current_line;
public:
	scctp_ctx() : p_position(nullptr), current_line(0) {}
	~scctp_ctx() {}

	char  *get_pos() { return p_position; }
	size_t get_current_line() { return current_line; }
};

class scc_textparser
{
	size_t m_nsize;
	size_t m_nlines;
	size_t m_ncurr_line;
	char  *m_base;
	char  *m_pend;
	char  *m_ppos;

	char* get_curr_address();

	/**
	* @brief try_transform_CRLF_to_LF
	* @return number of lines
	*/
public:
	static size_t try_transform_CRLF_to_LF(char *p_input, size_t *p_size);

public:
	scc_textparser();
	~scc_textparser();

	bool         init(char *p_text_base, size_t n_textbuf_size);

	size_t       get_num_lines();
	bool         is_initialized();
	void         reset_cursor();
	void         get_context(scctp_ctx &dst);
	void         set_context(const scctp_ctx &src);
	bool         is_end();
	bool         pos_increment();
	int          get_char();
	bool         skip_spaces();
	SCCTP_STATUS skip_all_to_substr(const char *p_substr, size_t substrlen=0);
	bool         skip_all_to_nextline();
	bool         increment_lines_count_to_current_pos(char *p_pos);

	/* text buffer address ranges comparations */
	inline bool  addr_greater_end(char* p_addr) { return p_addr > m_pend; }
	inline bool  addr_greater_oreq_end(char* p_addr) { return p_addr >= m_pend; }
	inline bool  addr_lessoreq_end(char* p_addr) { return p_addr <= m_pend; }
};