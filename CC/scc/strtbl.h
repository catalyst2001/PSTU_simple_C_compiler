#pragma once
#include <vector>
#include <string>
#include <assert.h>

struct strlayout_s {
	strlayout_s() : start_offset(0), end_offset(0) {}
	uint32_t start_offset;
	uint32_t end_offset; // end_offset == next string start_offset
};

class strtbl
{
	strlayout_s m_strinf;
	size_t  m_nbuf_size;
	char   *m_pbuffer;
	std::vector<strlayout_s> m_strings;
public:
	strtbl();
	~strtbl();

	/**
	* @brief add new string to strin table and returns offset to start of the this string
	* @return - offset in bytes at start of this string
	*/
	bool add_string(uint32_t *p_dst, const char *p_string);

	inline size_t get_num_strings() { return m_strings.size(); }
	inline strlayout_s& get_string_info(size_t stridx) {
		assert(stridx < get_num_strings() && "string indedx in string table out of bounds");
		return m_strings[stridx];
	}

	size_t get_size();
	char * get_data();
	bool   release();
};

