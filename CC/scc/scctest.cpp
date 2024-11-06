#include "scctest.h"
#include <stdio.h>

/* tested code */
#include "textbuf.h"

void scctest::print_with_CRLF(const char* p_text)
{
	puts("--------------------\n");
	while (*p_text) {
		if (*p_text == '\r')
			puts("[CR]");
		if (*p_text == '\n')
			puts("[LF]");

		putc(*p_text, stdout);
		p_text++;
	}
	puts("--------------------\n");
}

char buf[1024] =
"\r\n"
"int main()\r\n"
"{\r\n"
"\treturn 0;\r\n"
"}\r\n"
"\r\n"
"int c = 19;\r\n"
"\r\n";

bool scctests::test_transform_CR_to_unix()
{
	size_t last_size = sizeof(buf);
	size_t size = last_size;
	printf("buffer size: %zd\n", size);
	scctest::print_with_CRLF(buf);
	size_t n_lines = scc_textparser::try_transform_CRLF_to_LF(buf, &size);
	printf("new buffer size: %zd\n", size);
	scctest::print_with_CRLF(buf);
	return size < last_size;
}

bool scctests::run_all_tests()
{
	return test_transform_CR_to_unix() &&
		1;
}
