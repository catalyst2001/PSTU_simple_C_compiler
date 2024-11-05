// C lexer
#pragma once
#include "scccom.h"
#include "memfile.h"
#include "outerr.h"

extern const char* keywords[];
extern const char* lexems[];

enum SCCT_TOKEN {
	SCCT_UNKNOWN = 0,
	SCCT_KEYWORD,
	SCCT_IDENT,
	SCCT_NUM,
	SCCT_FNUM,
	SCCT_LPAREN, SCCT_RPAREN, // '(' ')'
	SCCT_LQPAREN, SCCT_RQPAREN, // '[' ']'
	SCCT_LBRACE, SCCT_RBRACE, // '{' '}'
	SCCT_SEMICOLON, // ';'
	SCCT_ASSIGNMENT, // '='
	SCCT_NOT, // '!'
	SCCT_MOD, // '%'
	SCCT_XOR, // '^'
	SCCT_MUL, // '*'
	SCCT_SUB, // '-'
	SCCT_ADD, // '+'
	SCCT_ASSIGN, // '='
	SCCT_TILDE, // '~'
	SCCT_AND, // '&'
	SCCT_OR, // '|'
	SCCT_DOT, // '.'
	SCCT_LESS, // '<'
	SCCT_GREATER, // '>'
	SCCT_DIV, // '/'
	SCCT_QUESTION, // '?'
	SCCT_ADD_ASSIGN, // '+='
	SCCT_SUB_ASSIGN, // '-='
	SCCT_MUL_ASSIGN, // '*='
	SCCT_DIV_ASSIGN, // '/='
	SCCT_MOD_ASSIGN, // '%='
	SCCT_LSHIFT_ASSIGN, // '<<='
	SCCT_RSHIFT_ASSIGN, // '>>='
	SCCT_AND_ASSIGN, // '&='
	SCCT_XOR_ASSIGN, // '^='
	SCCT_OR_ASSIGN, // '|='
	SCCT_ARROW, // '->'
	SCCT_INC, // '++'
	SCCT_DEC, // '--'
	SCCT_LSHIFT, // '<<'
	SCCT_RSHIFT, // '>>'
	SCCT_LESS_EQUAL, // '<='
	SCCT_GREATER_EQUAL, // '>='
	SCCT_EQUAL, // '=='
	SCCT_NOT_EQUAL, // '!='
	SCCT_LOGICAL_AND, // '&&'
	SCCT_LOGICAL_OR, // '||'
	SCCT_EOF
};

enum SCCLEX_STATUS {
	SCCLEX_STATUS_OK = 0,
	SCCLEX_STATUS_INVALID_PARAMETER,
	SCCLEX_STATUS_INVALID_CHAR,
	SCCLEX_STATUS_NO_MORE_DATA
};

#define LEX_OK(x) ((x) == SCCLEX_STATUS_OK)

/* token struct */
#define SCCLEX_MAX_TOK_STRING 1024

struct scclex_tok {
	int        flags;
	size_t     length;
	char       string[SCCLEX_MAX_TOK_STRING];
	SCCT_TOKEN tok;
};

/* lexer context */
struct scclex_ctx {
	char *m_ppos;
};

class scclex
{
	size_t m_nsize;
	char  *m_ppos;
	char  *m_psource;
	char  *m_psource_end;

protected:
	inline bool is_end() { return m_ppos >= m_psource_end; }

	void       make_eof_token(scclex_tok& tok);

	bool       skip_spaces();
	bool       is_keyword(const char* p_str);
	bool       read_alpha(scclex_tok& tok);
	bool       read_numeric(scclex_tok& tok);
	bool       read_delims(scclex_tok& tok);
public:
	scclex();
	~scclex();

	SCCLEX_STATUS init(memfile &mf);
	SCCLEX_STATUS store_ctx(scclex_ctx &dst);
	SCCLEX_STATUS restore_ctx(const scclex_ctx &src);
	SCCLEX_STATUS next_tok(scclex_tok &tok);
};

