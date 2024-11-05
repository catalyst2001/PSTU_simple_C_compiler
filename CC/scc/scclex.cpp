#include "scclex.h"

//https://cmcmsu.info/download/cpl.lexis.pdf
const char* keywords[] = {
	/* builtin typ]es */
	"bool", "char", "short", "ushort", "int", "uint",
	
	/* special */
	"import", "export",

	/* operators */
	"if", "else", "do", "while", "for"
};

void scclex::make_eof_token(scclex_tok& tok)
{
	tok.tok = SCCT_EOF;
	tok.string[0] = 0;
}

bool scclex::skip_spaces()
{
	while (*m_ppos && isspace(*m_ppos)) {
		if (!is_end()) {
			m_ppos++;
			continue;
		}
		return false;
	}
	return true;
}

bool scclex::is_keyword(const char* p_str)
{
	//TODO: K.D. OPTIMIZE THIS!!!!
	const size_t num_keywords = sizeof(keywords) / sizeof(keywords[0]);
	for (size_t i = 0; i < num_keywords; i++) {
		if (!strcmp(keywords[i], p_str)) {
			return true;
		}
	}
	return false;
}

bool scclex::read_alpha(scclex_tok& tok)
{
	/* read alphabet symbols */
	tok.length = 0;
	while (*m_ppos && (isalpha(*m_ppos) || *m_ppos == '_')) {
		if (!is_end()) {
			tok.string[tok.length++] = *m_ppos;
			m_ppos++;
			continue;
		}
		return false;
	}
	tok.string[tok.length] = '\0';

	/* checking for keyword */
	if (is_keyword(tok.string)) {
		tok.tok = SCCT_KEYWORD;
		tok.flags = 0;
		return true;
	}

	/* it is identifier */
	tok.flags = 0;
	tok.tok = SCCT_IDENT;
	return true;
}

bool scclex::read_numeric(scclex_tok& tok)
{
	/* read numbers */
	tok.length = 0;
	while (*m_ppos && isdigit(*m_ppos)) {
		if (!is_end()) {
			tok.string[tok.length++] = *m_ppos;
			m_ppos++;
			continue;
		}
		return false;
	}
	tok.string[tok.length] = '\0';

	/* fill token */
	tok.flags = 0;
	tok.tok = SCCT_NUM;
	return true;
}

bool scclex::read_delims(scclex_tok& tok)
{
	/* read single chars */
	tok.length = 0;
	if (*m_ppos) {
		tok.flags = 0;
		switch (*m_ppos)
		{
		case '(':
			tok.tok = SCCT_LPAREN;
			break;

		case ')':
			tok.tok = SCCT_RPAREN;
			break;

		case '[':
			tok.tok = SCCT_LQPAREN;
			break;

		case ']':
			tok.tok = SCCT_RQPAREN;
			break;

		case '{':
			tok.tok = SCCT_LBRACE;
			break;

		case '}':
			tok.tok = SCCT_RBRACE;
			break;

		case ';':
			tok.tok = SCCT_SEMICOLON;
			break;

		default:
			return false;
		}
	}
	return true;
}

scclex::scclex() : m_ppos(nullptr), m_nsize(0), m_psource(nullptr), m_psource_end(nullptr)
{
}

scclex::~scclex()
{
}

SCCLEX_STATUS scclex::init(memfile& mf)
{
	/* check for correct */
	if (mf.is_empty())
		return SCCLEX_STATUS_NO_MORE_DATA;

	m_nsize = mf.get_text_size();
	m_psource = mf.get_text();
	m_psource_end = m_psource + m_nsize;
	m_ppos = m_psource;
	return SCCLEX_STATUS_OK;
}

SCCLEX_STATUS scclex::store_ctx(scclex_ctx& dst)
{
	if (!m_psource)
		return SCCLEX_STATUS_NO_MORE_DATA;

	dst.m_ppos = m_ppos;
	return SCCLEX_STATUS_OK;
}

SCCLEX_STATUS scclex::restore_ctx(const scclex_ctx& src)
{
	if (!m_psource)
		return SCCLEX_STATUS_NO_MORE_DATA;

	m_ppos =  src.m_ppos;
	return SCCLEX_STATUS_OK;
}

SCCLEX_STATUS scclex::next_tok(scclex_tok& tok)
{
	SCCLEX_STATUS status;

	/* reset token */
	tok.flags = 0;
	tok.string[0] = 0;
	tok.tok = SCCT_UNKNOWN;

	/* skip spaces */
	if (!skip_spaces()) {
		/* skip_spaces walked to EOF */
		make_eof_token(tok);
		return SCCLEX_STATUS_NO_MORE_DATA;
	}

	/* try read idents and keywords */
	if (read_alpha(tok))
		return SCCLEX_STATUS_OK;

	/* try read numeric */
	if (read_alpha(tok))
		return SCCLEX_STATUS_OK;

	/* read numbers */
	if(read_numeric(tok))
		return SCCLEX_STATUS_OK;

	/* read delimeters */
	if(read_delims(tok))
		return SCCLEX_STATUS_OK;

	/* all checks failed. is end position??! */
	if (is_end())
		return SCCLEX_STATUS_NO_MORE_DATA;

	/* input char is invalid */
	return SCCLEX_STATUS_INVALID_CHAR;
}
