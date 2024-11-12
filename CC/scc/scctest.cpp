#include "scctest.h"
#include <stdio.h>

/* tested code */
#include "textbuf.h"
#include "bytewriter.h"
#include "scclex.h"

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

bool test_transform_CR_to_unix()
{
	size_t last_size = sizeof(buf);
	size_t size = last_size;
	printf("buffer size: %zd\n", size);
	scctest::print_with_CRLF(buf);
	size_t n_lines = scc_textparser::try_transform_line_completions_CRLF_to_LF(buf, &size);
	printf("new buffer size: %zd\n", size);
	scctest::print_with_CRLF(buf);
	return size < last_size;
}

#define CASESTR(x) case x: return #x;
const char* tok_to_string(SCC_TOK tok) {
	switch (tok) {
		CASESTR(SCCT_UNKNOWN)
		CASESTR(SCCT_KEYWORD)
		CASESTR(SCCT_STRING)
		CASESTR(SCCT_IDENT)
		CASESTR(SCCT_INUM)
		CASESTR(SCCT_FNUM)
		CASESTR(SCCT_LPAREN)
		CASESTR(SCCT_RPAREN)
		CASESTR(SCCT_LQPAREN)
		CASESTR(SCCT_RQPAREN)
		CASESTR(SCCT_LBRACE)
		CASESTR(SCCT_RBRACE)
		CASESTR(SCCT_MOD)
		CASESTR(SCCT_MOD_ASSIGN)
		CASESTR(SCCT_ADD)
		CASESTR(SCCT_INC)
		CASESTR(SCCT_ADD_ASSIGN)
		CASESTR(SCCT_SUB)
		CASESTR(SCCT_DEC)
		CASESTR(SCCT_SUB_ASSIGN)
		CASESTR(SCCT_ARROW)
		CASESTR(SCCT_ASTERISK)
		CASESTR(SCCT_MUL_ASSIGN)
		CASESTR(SCCT_DIV)
		CASESTR(SCCT_DIV_ASSIGN)
		CASESTR(SCCT_OR)
		CASESTR(SCCT_LOGICAL_OR)
		CASESTR(SCCT_OR_ASSIGN)
		CASESTR(SCCT_LESS)
		CASESTR(SCCT_LESS_EQUAL)
		CASESTR(SCCT_LSHIFT)
		CASESTR(SCCT_LSHIFT_ASSIGN)
		CASESTR(SCCT_GREATER)
		CASESTR(SCCT_GREATER_EQUAL)
		CASESTR(SCCT_RSHIFT)
		CASESTR(SCCT_RSHIFT_ASSIGN)
		CASESTR(SCCT_AND)
		CASESTR(SCCT_LOGICAL_AND)
		CASESTR(SCCT_AND_ASSIGN)
		CASESTR(SCCT_XOR)
		CASESTR(SCCT_XOR_ASSIGN)
		CASESTR(SCCT_ASSIGNMENT)
		CASESTR(SCCT_EQUAL)
		CASESTR(SCCT_NOT)
		CASESTR(SCCT_NOT_EQUAL)
		CASESTR(SCCT_SEMICOLON)
		CASESTR(SCCT_TILDE)
		CASESTR(SCCT_DOT)
		CASESTR(SCCT_COMMA)
		CASESTR(SCCT_QUESTION)
		CASESTR(SCCT_EOF)
	}
	return "<UNKNOWN TOKEN>";
}

const char* scclex_status_to_string(SCCLEX_STATUS s)
{
	switch (s) {
		CASESTR(SCCLEX_STATUS_OK)
		CASESTR(SCCLEX_STATUS_INVALID_PARAMETER)
		CASESTR(SCCLEX_STATUS_INVALID_CHAR)
		CASESTR(SCCLEX_STATUS_NO_MORE_DATA)
	}
	return "<UNKNOWN STATUS>";
}

const char* scclex_keyword_to_string(SCC_KW s)
{
	switch (s) {
		CASESTR(SCCLEX_STATUS_OK)
		CASESTR(SCCLEX_STATUS_INVALID_PARAMETER)
		CASESTR(SCCLEX_STATUS_INVALID_CHAR)
		CASESTR(SCCLEX_STATUS_NO_MORE_DATA)
	}
	return "<UNKNOWN STATUS>";
}

#undef CASESTR

bool scclex_test()
{
	const char p_code[] = R"(
	int var1;
	int var2 = 100;
	int var3=100;
	int arr1[10];
	int arr2[]={1,2,3};
	char *ptr1=0;
	const int constvar1 = 100;
	
int main(int argc, char **argv)
{
	printf("hello world!\n");
	return 0;
}
)";
	SCCLEX_STATUS s;
	memfile mf(p_code, sizeof(p_code)-1);
	scclex lexer;
	s = lexer.init(mf);
	if (s != SCCLEX_STATUS_OK) {
		printf("SCCLEX_STATUS_OK %d\n", s);
		return false;
	}

	scclex_tok tok;
	SCCLEX_STATUS status;
	while (1) {
		status = lexer.next_tok(tok);
		if (status == SCCLEX_STATUS_NO_MORE_DATA) {
			printf("END OF BUFFER\n");
			break;
		}

		/* other status */
		if (status != SCCLEX_STATUS_OK) {
			printf("SCCLEX_STATUS: %s\n", scclex_status_to_string(status));
		}

		if (tok.tok != SCCT_EOF) {
			if (tok.tok == SCCT_UNKNOWN)
				__asm int 3h;

			printf("TOK: %-24s  STR: %-32s\n", tok_to_string(tok.tok), tok.string);
		}
	}
	return false;
}

bool test_vm()
{






	return true;
}
