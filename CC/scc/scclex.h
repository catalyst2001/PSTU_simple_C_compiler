/**
* Simple C lexer
* 
* Created: 05.11.24
* Date: 07.11.24
* Author: Deryabin K.
*/
#pragma once
#include "scccom.h"
#include "memfile.h"
#include "outerr.h"
#include "textbuf.h"

#include <map>
#include <string>

enum SCC_TOK {
	SCCT_UNKNOWN = 0,
	SCCT_KEYWORD,
	SCCT_STRING,
	SCCT_IDENT, // identifier
	SCCT_INUM, // integer number
	SCCT_FNUM, // float/double number

	SCCT_LPAREN, SCCT_RPAREN, // '(' ')'

	SCCT_LQPAREN, SCCT_RQPAREN, // '[' ']'

	SCCT_LBRACE, SCCT_RBRACE, // '{' '}'

	SCCT_MOD, // '%'
	SCCT_MOD_ASSIGN, // '%='

	SCCT_ADD, // '+'
	SCCT_INC, // '++'
	SCCT_ADD_ASSIGN, // '+='

	SCCT_SUB, // '-'
	SCCT_DEC, // '--'
	SCCT_SUB_ASSIGN, // '-='
	SCCT_ARROW, // '->'

	SCCT_ASTERISK, // '*' mul numbers or ptrs declaration/dereference
	SCCT_MUL_ASSIGN, // '*='

	SCCT_DIV, // '/'
	SCCT_DIV_ASSIGN, // '/='

	SCCT_OR, // '|'
	SCCT_LOGICAL_OR, // '||'
	SCCT_OR_ASSIGN, // '|='

	SCCT_LESS, // '<'
	SCCT_LESS_EQUAL, // '<='
	SCCT_LSHIFT, // '<<'
	SCCT_LSHIFT_ASSIGN, // '<<='

	SCCT_GREATER, // '>'
	SCCT_GREATER_EQUAL, // '>='
	SCCT_RSHIFT, // '>>'
	SCCT_RSHIFT_ASSIGN, // '>>='

	SCCT_AND, // '&'
	SCCT_LOGICAL_AND, // '&&'
	SCCT_AND_ASSIGN, // '&='

	SCCT_XOR, // '^'
	SCCT_XOR_ASSIGN, // '^='

	SCCT_ASSIGNMENT, // '='
	SCCT_EQUAL, // '=='

	SCCT_NOT, // '!'
	SCCT_NOT_EQUAL, // '!='

	SCCT_SEMICOLON, // ';'
	SCCT_TILDE, // '~'
	SCCT_DOT, // '.'
	SCCT_COMMA, // ','
	SCCT_QUESTION, // '?'

	SCCT_EOF
};

enum SCCLEX_STATUS {
	SCCLEX_STATUS_OK = 0,
	SCCLEX_STATUS_INVALID_PARAMETER,
	SCCLEX_STATUS_INVALID_CHAR,
	SCCLEX_STATUS_NO_MORE_DATA
};

enum SCC_KW : uint32_t {
	SCCKW_UNKNOWN =0,

	/* type keywords */
	SCCKW_VOID,
	SCCKW_BOOL,
	SCCKW_CHAR,
	SCCKW_BYTE,
	SCCKW_SHORT,
	SCCKW_USHORT,
	SCCKW_INT,
	SCCKW_UINT,
	SCCKW_CONST,
	SCCKW_STRUCT,
	SCCKW_AUTO,
	SCCKW_TYPEDEF,

	/* special keywords */
	SCCKW_IMPORT,
	SCCKW_EXPORT,
	SCCKW_RETURN,

	SCCKW_IF,
	SCCKW_ELSE,
	SCCKW_DO,
	SCCKW_WHILE,
	SCCKW_FOR,

	SCCKW_BREAK,
	SCCKW_CONTINUE,
	SCCKW_EXTERN,

	SCCKW_ASM,
	SCCKW_EMIT,

	SCCKW_STATIC,
	SCCKW_ATOMIC,
	SCCKW_ASYNC
};

#define LEX_KW_IS_TYPE(x) ((x) >= SCCKW_BOOL && (x) <= SCCKW_TYPEDEF)

#define LEX_OK(x) ((x) == SCCLEX_STATUS_OK)

/* token struct */
#define SCCLEX_MAX_TOK_STRING 1024

/* token flags */
#define SCCTOK_OP_DEFAULT      (0)
#define SCCTOK_OP_ARG_LITERAL  (1 << 0)
#define SCCTOK_OP_ARG_VARIABLE (1 << 1)
#define SCCTOK_OP_INT_NUMERIC  (1 << 2)
#define SCCTOK_OP_FLT_NUMERIC  (1 << 3)
#define SCCTOK_OP_IDENT        (1 << 4)
#define SCCTOK_OP_BITWISE      (1 << 5)
#define SCCTOK_OP_LOGICAL      (1 << 6)
#define SCCTOK_OP_SCOPE        (1 << 7)
#define SCCTOK_OP_EXPR_COMPL   (1 << 8)
#define SCCTOK_OP_AGGREGATE_INIT (1 << 9)

/* helpers */
#define SCCTOK_OP_LITVAR (SCCTOK_OP_ARG_LITERAL|SCCTOK_OP_ARG_VARIABLE)
#define SCCTOK_OP_LITVARBIT (SCCTOK_OP_LITVAR|SCCTOK_OP_BITWISE)
#define SCCTOK_OP_LITVARLOG (SCCTOK_OP_LITVAR|SCCTOK_OP_LOGICAL)
#define SCCTOK_OP_LITVARNUMI (SCCTOK_OP_LITVAR|SCCTOK_OP_INT_NUMERIC)
#define SCCTOK_OP_LITVARNUMIF (SCCTOK_OP_LITVAR|SCCTOK_OP_INT_NUMERIC|SCCTOK_OP_FLT_NUMERIC)
#define SCCTOK_OP_ASSIGNMENT (SCCTOK_OP_LITVAR|SCCTOK_OP_LOGICAL|SCCTOK_OP_INT_NUMERIC|SCCTOK_OP_FLT_NUMERIC)

struct scclex_tok {
	int     flags;
	size_t  start_line;
	size_t  length;
	char    string[SCCLEX_MAX_TOK_STRING];
	SCC_TOK tok;
	SCC_KW  kw;
};

class scclex
{
	std::map<std::string, SCC_KW> m_keywords;
	scc_textparser m_src;
protected:
	void       make_eof_token(scclex_tok& tok);
	void       init_keywords(); //defines keywords here
protected:
	SCC_KW     find_keyword(const char* p_str);
	bool       process_string_literal(scclex_tok& tok);
	bool       read_alpha(scclex_tok& tok);
	bool       read_numeric(scclex_tok& tok);
	bool       read_delims(scclex_tok& tok);
public:
	scclex();
	~scclex();
	SCCLEX_STATUS init(memfile &mf);
	SCCLEX_STATUS next_tok(scclex_tok &tok);
};

