#include "scclex.h"

//https://cmcmsu.info/download/cpl.lexis.pdf
const char* keywords[] = {
	/* builtin typ]es */
	"bool", "char", "byte", "short", "ushort", "int", "uint",
	
	/* special */
	"import", "export",

	/*  */

	/* operators */
	"if", "else", "do", "while", "for"
};

void scclex::make_eof_token(scclex_tok& tok)
{
	tok.tok = SCCT_EOF;
	tok.string[0] = 0;
}

bool scclex::is_keyword(const char* p_str)
{
	//TODO: K.D. OPTIMIZE THIS!!!! STORE keywords IN HASHMAP!
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
	while (m_src.get_char() && (isalpha(m_src.get_char()) || m_src.get_char() == '_')) {
		if (!m_src.is_end()) {
			tok.string[tok.length++] = m_src.get_char();
			m_src.pos_increment();
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
	while (m_src.get_char() && isdigit(m_src.get_char())) {
		if (!m_src.is_end()) {
			tok.string[tok.length++] = m_src.get_char();
			m_src.pos_increment();
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
	if (m_src.get_char()) {
		tok.flags = 0;
		switch (m_src.get_char())
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

		case '=': {
			tok.tok = SCCT_ASSIGNMENT;
			if (m_src.pos_increment()) {
				if (m_src.get_char() == '=') {
					tok.tok = SCCT_EQUAL; //++
					return true;
				}
			}
			return true;
		}

			/* handle +, -, */

			/* + (add) */
		case '+': {
			tok.tok = SCCT_ADD;
			if (m_src.pos_increment()) {
				if (m_src.get_char() == '+') {
					tok.tok = SCCT_INC; //++
					return true;
				}
				if (m_src.get_char() == '=') {
					tok.tok = SCCT_ADD_ASSIGN; //+=
					return true;
				}
			}
			return true;
		} // END case '+'

			/* - (sub) */
		case '-': {
			tok.tok = SCCT_SUB;
			if (m_src.pos_increment()) {
				if (m_src.get_char() == '-') {
					tok.tok = SCCT_INC; //--
					return true;
				}
				if (m_src.get_char() == '=') {
					tok.tok = SCCT_SUB_ASSIGN; //-=
					return true;
				}
				if (m_src.get_char() == '>') {
					tok.tok = SCCT_ARROW; //->
					return true;
				}
			}
			return true;
		} //END case '-'





		default:
			return false;
		}
	}
	return true;
}

scclex::scclex()
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

	m_src.init(mf.get_text(), mf.get_text_size());
	return SCCLEX_STATUS_OK;
}

SCCLEX_STATUS scclex::store_ctx(scclex_ctx& dst)
{
	if (!m_src.is_initialized())
		return SCCLEX_STATUS_NO_MORE_DATA;

	dst.m_ppos = m_src.get_context();
	return SCCLEX_STATUS_OK;
}

SCCLEX_STATUS scclex::restore_ctx(const scclex_ctx& src)
{
	if (!m_src.is_initialized())
		return SCCLEX_STATUS_NO_MORE_DATA;

	m_src.set_context(src.m_ppos);
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
	if (!m_src.skip_spaces()) {
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
	if (m_src.is_end())
		return SCCLEX_STATUS_NO_MORE_DATA;

	/* input char is invalid */
	return SCCLEX_STATUS_INVALID_CHAR;
}
