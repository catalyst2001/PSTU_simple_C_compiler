#pragma once
#include <stdio.h>

enum MEMFILE_STATUS {
	MEMFILE_STATUS_OK = 0,
	MEMFILE_STATUS_OPEN_FAILED,
	MEMFILE_STATUS_EMPTY,
	MEMFILE_STATUS_OUT_OF_MEMORY,
	MEMFILE_STATUS_IO_READ_FAILED,

	MEMFILE_STATUS_LAST
};

#define MF_OK(x) ((x) == MEMFILE_STATUS_OK)

class memfile
{
	char  *m_ptext;
	size_t m_nsize;
public:
	memfile();
	memfile(const char *p_text, size_t textlen);
	~memfile();

	MEMFILE_STATUS load(const char *p_file_path);
	MEMFILE_STATUS release();

	bool           is_empty();
	char          *get_text();
	size_t         get_text_size();

	static const char* status_to_string(MEMFILE_STATUS s);
};

