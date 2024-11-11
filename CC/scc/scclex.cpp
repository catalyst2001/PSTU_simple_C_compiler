#include "scclex.h"

//https://cmcmsu.info/download/cpl.lexis.pdf
const char* keywords[] = {
	/* builtin typ]es */
	"bool", "char", "byte", "short", "ushort", "int", "uint", "const"
	
	/* special */
	"import", "export",

	/*  */
	"struct", "auto", "typedef"

	/* operators */
	"if", "else", "do", "while", "for"
};

void scclex::make_eof_token(scclex_tok& tok)
{
	tok.tok = SCCT_EOF;
	tok.string[0] = 0;
}

void scclex::init_keywords()
{
	/* types and qualifiers 
	"bool", "char", "byte", "short", "ushort", "int", "uint", "const", "auto"
	*/
	m_keywords["bool"] = SCCKW_BOOL;
	m_keywords["char"] = SCCKW_CHAR;
	m_keywords["byte"] = SCCKW_BYTE;
	m_keywords["short"] = SCCKW_SHORT;
	m_keywords["ushort"] = SCCKW_USHORT;
	m_keywords["int"] = SCCKW_INT;
	m_keywords["uint"] = SCCKW_UINT;
	m_keywords["const"] = SCCKW_CONST;
	m_keywords["auto"] = SCCKW_AUTO;

	//"import", "export",
	m_keywords["import"] = SCCKW_IMPORT;
	m_keywords["export"] = SCCKW_EXPORT;
	m_keywords["return"] = SCCKW_RETURN;

	//"struct", "typedef"
	m_keywords["struct"] = SCCKW_STRUCT;	
	m_keywords["typedef"] = SCCKW_TYPEDEF;

	//"if", "else", "do", "while", "for"
	m_keywords["if"] = SCCKW_IF;
	m_keywords["else"] = SCCKW_ELSE;
	m_keywords["do"] = SCCKW_DO;
	m_keywords["while"] = SCCKW_WHILE;
	m_keywords["for"] = SCCKW_FOR;
	m_keywords["break"] = SCCKW_BREAK;
	m_keywords["continue"] = SCCKW_CONTINUE;
}

SCC_KW scclex::find_keyword(const char* p_str)
{
	/* find kw in hashtable */
	assert(m_keywords.size() && "keywords is not defined! hash table is empty!");

	/* kw exists? */
	if (m_keywords.find(p_str) == m_keywords.end())
		return SCCKW_UNKNOWN; // kw not found

	return m_keywords[p_str]; //kw found
}

bool scclex::process_string_literal(scclex_tok& tok)
{
	return false;
}

bool scclex::read_alpha(scclex_tok& tok)
{
	/* read alphabet symbols */
	int curr_char;
	int num_quotes = 0;
	tok.length = 0;
	tok.start_line = m_src.get_current_line();
	if (!isalpha(m_src.get_char()) || m_src.get_char() != '_' || m_src.get_char() != '"')
		return false;

	/* process string literals */
	if (m_src.get_char() == '"') {
		tok.tok = SCCT_STRING;
		m_src.pos_increment(); // move next from '"'
		/* is not end of buffer? */
		while (!m_src.is_end()) {
			curr_char = m_src.get_char();
			/* finish string? */
			if (curr_char == '"') {
				m_src.pos_increment();
				tok.string[tok.length++] = '\0';
				return true;
			}
			/* escaped strings or escape sequences? */
			if (curr_char == '\\') {
				m_src.pos_increment(); //skip '\'
				if (m_src.is_end()) {
					break;
				}
				/* handle escape sequences */
				curr_char = m_src.get_char();
				switch (curr_char) {
				case 'r':
					tok.string[tok.length++] = '\r';
					break;
				case 'n':
					tok.string[tok.length++] = '\n';
					break;
				case 't':
					tok.string[tok.length++] = '\t';
					break;
				case 'v':
					tok.string[tok.length++] = '\v';
					break;
				case '"':
					tok.string[tok.length++] = '"';
					break;
				case '\'':
					tok.string[tok.length++] = '\'';
					break;
				case '\\':
					tok.string[tok.length++] = '\\';
					break;
				case '0':
					tok.string[tok.length++] = '\0';
					break;
				case '?':
					tok.string[tok.length++] = '\?';
					break;
				case 'a':
					tok.string[tok.length++] = '\a';
					break;
				default:
					tok.string[tok.length++] = curr_char;
					break;
				}
				continue;
			}
			tok.string[tok.length++] = curr_char;
		}
		return true;
	}

	/* process identifiers and keywords */
	while (!m_src.is_end()) {
		if (m_src.get_char() && (isalpha(m_src.get_char()) || m_src.get_char() == '_' || m_src.get_char() == '"')) {
			tok.string[tok.length++] = m_src.get_char();
			m_src.pos_increment();
			continue;
		}
		return false;
	}
	tok.string[tok.length] = '\0';

	/* checking for keyword */
	tok.kw = find_keyword(tok.string);
	/* this tok is keyword? */
	if (tok.kw != SCCKW_UNKNOWN) {
		tok.tok = SCCT_KEYWORD;
		tok.flags = 0;
		return true; //yes, it is keyword
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
	tok.start_line = m_src.get_current_line();
	if (!m_src.get_char() || !isdigit(m_src.get_char()))
		return false;

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
	tok.tok = SCCT_INUM;
	return true;
}

bool scclex::read_delims(scclex_tok& tok)
{
	scctp_ctx parser_ctx;
	/* read single chars */
	tok.length = 0;
	if (m_src.get_char()) {
		tok.flags = 0;
		switch (m_src.get_char())
		{
		case '(':
			tok.start_line = m_src.get_current_line();
			tok.flags = SCCTOK_OP_DEFAULT;
			tok.tok = SCCT_LPAREN;
			break;

		case ')':
			tok.start_line = m_src.get_current_line();
			tok.flags = SCCTOK_OP_DEFAULT;
			tok.tok = SCCT_RPAREN;
			break;

		case '[':
			tok.start_line = m_src.get_current_line();
			tok.flags = SCCTOK_OP_DEFAULT;
			tok.tok = SCCT_LQPAREN;
			break;

		case ']':
			tok.start_line = m_src.get_current_line();
			tok.flags = SCCTOK_OP_DEFAULT;
			tok.tok = SCCT_RQPAREN;
			break;

		case '{':
			tok.start_line = m_src.get_current_line();
			tok.flags = SCCTOK_OP_SCOPE;
			tok.tok = SCCT_LBRACE;
			break;

		case '}':
			tok.start_line = m_src.get_current_line();
			tok.flags = SCCTOK_OP_SCOPE;
			tok.tok = SCCT_RBRACE;
			break;

		case ';':
			tok.start_line = m_src.get_current_line();
			tok.flags = SCCTOK_OP_EXPR_COMPL;
			tok.tok = SCCT_SEMICOLON;
			break;

			//TODO: SCCT_MOD OK!
		case '%': {
			tok.start_line = m_src.get_current_line();
			tok.flags = SCCTOK_OP_LITVARNUMI;
			tok.tok = SCCT_MOD;
			if (m_src.pos_increment()) {
				if (m_src.get_char() == '=') {
					tok.start_line = m_src.get_current_line();
					//TODO: SCCT_MOD_ASSIGN OK!
					tok.tok = SCCT_MOD_ASSIGN; // %=
					break;
				}
			}
			break;
		} //END case '%'

			/* + (add) */
		//TODO: SCCT_ADD
		case '+': {
			tok.start_line = m_src.get_current_line();
			tok.flags = SCCTOK_OP_LITVARNUMIF;
			tok.tok = SCCT_ADD;
			if (m_src.pos_increment()) {
				//TODO: SCCT_INC OK!
				if (m_src.get_char() == '+') {
					tok.start_line = m_src.get_current_line();
					tok.tok = SCCT_INC; //++
					break;
				}
				//TODO: SCCT_ADD_ASSIGN OK!
				if (m_src.get_char() == '=') {
					tok.start_line = m_src.get_current_line();
					tok.tok = SCCT_ADD_ASSIGN; //+=
					break;
				}
			}
			break;
		} // END case '+'

			/* - (sub) */
		//TODO: SCCT_SUB OK!
		case '-': {
			tok.tok = SCCT_SUB;
			tok.start_line = m_src.get_current_line();
			if (m_src.pos_increment()) {
				if (m_src.get_char() == '-') {
					tok.start_line = m_src.get_current_line();
					//TODO: SCCT_INC OK!
					tok.tok = SCCT_INC; //--
					break;
				}
				if (m_src.get_char() == '=') {
					tok.start_line = m_src.get_current_line();
					//TODO: SCCT_SUB_ASSIGN OK!
					tok.tok = SCCT_SUB_ASSIGN; //-=
					break;
				}
				if (m_src.get_char() == '>') {
					tok.start_line = m_src.get_current_line();
					//TODO: SCCT_ARROW OK!
					tok.tok = SCCT_ARROW; //->
					break;
				}
			}
			break;
		} //END case '-'

			/* * (mul) */
		//TODO: SCCT_MUL OK!
		case '*': {
			tok.flags = SCCTOK_OP_LITVARNUMIF;
			tok.tok = SCCT_ASTERISK;
			tok.start_line = m_src.get_current_line();
			if (m_src.pos_increment()) {
				if (m_src.get_char() == '=') {
					tok.start_line = m_src.get_current_line();
					//TODO: SCCT_MUL_ASSIGN OK!
					tok.tok = SCCT_MUL_ASSIGN; //*=
					break;
				}
			}
			break;
		} //END case '*'

			/* / (div) */
		//TODO: SCCT_DIV OK!
		case '/': {
			tok.flags = SCCTOK_OP_LITVARNUMIF;
			tok.tok = SCCT_DIV;
			tok.start_line = m_src.get_current_line();
			if (m_src.pos_increment()) {			
				if (m_src.get_char() == '=') {
					tok.start_line = m_src.get_current_line();
					//TODO: SCCT_DIV_ASSIGN OK!
					tok.tok = SCCT_DIV_ASSIGN; // /=
					break;
				}
			}
			break;
		} //END case '/'

			/* | (biwise OR ) */
		//TODO: SCCT_OR OK!
		case '|': {
			tok.flags = SCCTOK_OP_LITVARBIT;
			tok.tok = SCCT_OR;
			tok.start_line = m_src.get_current_line();
			if (m_src.pos_increment()) {
				if (m_src.get_char() == '|') {
					tok.start_line = m_src.get_current_line();
					//TODO: SCCT_LOGICAL_OR OK!
					tok.tok = SCCT_LOGICAL_OR; // ||
					break;
				}
				if (m_src.get_char() == '=') {
					tok.start_line = m_src.get_current_line();
					//TODO: SCCT_OR_ASSIGN OK!
					tok.tok = SCCT_OR_ASSIGN; // |=
					break;
				}
			}
			break;
		} //END case '|'

	 /* < (less) */
		//TODO: SCCT_LESS OK!
		case '<': {
			tok.flags = SCCTOK_OP_LITVARLOG;
			tok.tok = SCCT_LESS;
			tok.start_line = m_src.get_current_line();
			if (m_src.pos_increment()) {			
				if (m_src.get_char() == '=') {
					tok.start_line = m_src.get_current_line();
					//TODO: SCCT_LESS_EQUAL OK!
					tok.tok = SCCT_LESS_EQUAL; // <=
					break;
				}

				if (m_src.get_char() == '<') {
					//TODO: SCCT_LSHIFT OK!
					tok.start_line = m_src.get_current_line();
					tok.flags = SCCTOK_OP_LITVARBIT;
					tok.tok = SCCT_LSHIFT; // <<
					m_src.store_context(parser_ctx);
					if (m_src.pos_increment()) {					
						if (m_src.get_char() == '=') {
							tok.start_line = m_src.get_current_line();
							//TODO: SCCT_LSHIFT_ASSIGN OK!
							tok.tok = SCCT_LSHIFT_ASSIGN; // <<=
							break;
						}
					}
					m_src.restore_context(parser_ctx); //rollback
					break;
				}
			}
			break;
		} //END case '<'	

			/* > (greater) */
		//TODO: SCCT_GREATER OK!
		case '>': {
			tok.start_line = m_src.get_current_line();
			tok.flags = SCCTOK_OP_LITVARLOG;
			tok.tok = SCCT_GREATER;
			if (m_src.pos_increment()) {			
				if (m_src.get_char() == '=') {
					tok.start_line = m_src.get_current_line();
					//TODO: SCCT_GREATER_EQUAL OK!
					tok.tok = SCCT_GREATER_EQUAL; // >=
					break;
				}

				if (m_src.get_char() == '>') {
					//TODO: SCCT_RSHIFT OK!
					tok.start_line = m_src.get_current_line();
					tok.flags = SCCTOK_OP_LITVARBIT;
					tok.tok = SCCT_RSHIFT; // >>
					m_src.store_context(parser_ctx);
					if (m_src.pos_increment()) {				
						if (m_src.get_char() == '=') {
							tok.start_line = m_src.get_current_line();
							//TODO: SCCT_RSHIFT_ASSIGN OK!
							tok.tok = SCCT_RSHIFT_ASSIGN; // >>=
							break;
						}
					}
					m_src.restore_context(parser_ctx); //rollback
					break;
				}
			}
			break;
		} //END case '>'	

			/* & (biwise AND ) */
		//TODO: SCCT_AND OK!
		case '&': {
			tok.flags = SCCTOK_OP_LITVARBIT;
			tok.tok = SCCT_AND;
			tok.start_line = m_src.get_current_line();
			if (m_src.pos_increment()) {			
				if (m_src.get_char() == '&') {
					tok.start_line = m_src.get_current_line();
					//TODO: SCCT_LOGICAL_AND OK!
					tok.flags = SCCTOK_OP_LITVARLOG;
					tok.tok = SCCT_LOGICAL_AND; // &&
					break;
				}
				if (m_src.get_char() == '=') {
					tok.start_line = m_src.get_current_line();
					//TODO: SCCT_AND_ASSIGN OK!
					tok.tok = SCCT_AND_ASSIGN; // &=
					break;
				}
			}
			break;
		} //END case '&'

			/* ^ (XOR) */
		//TODO: SCCT_XOR OK!
		case '^': {
			tok.flags = SCCTOK_OP_LITVARBIT;
			tok.tok = SCCT_XOR;
			tok.start_line = m_src.get_current_line();
			if (m_src.pos_increment()) {
				if (m_src.get_char() == '=') {
					tok.start_line = m_src.get_current_line();
					//TODO: SCCT_XOR_ASSIGN OK!
					tok.tok = SCCT_XOR_ASSIGN; // ^=
					break;
				}
			}
			break;
		} //END case '^'

			//TODO: SCCT_ASSIGNMENT
		case '=': {
			tok.flags = SCCTOK_OP_ASSIGNMENT;
			tok.tok = SCCT_ASSIGNMENT;
			tok.start_line = m_src.get_current_line();
			if (m_src.pos_increment()) {
				if (m_src.get_char() == '=') {
					tok.start_line = m_src.get_current_line();
					tok.flags = SCCTOK_OP_LITVARLOG;
					tok.tok = SCCT_EQUAL; // ==
					break;
				}
			}
			break;
		}

		//TODO: SCCT_NOT OK!
		case '!': {
			tok.flags = SCCTOK_OP_LITVARLOG;
			tok.tok = SCCT_NOT;
			tok.start_line = m_src.get_current_line();
			if (m_src.pos_increment()) {
				if (m_src.get_char() == '=') {
					tok.start_line = m_src.get_current_line();
					//TODO: SCCT_NOT_EQUAL OK!
					tok.tok = SCCT_NOT_EQUAL; // !=
					break;
				}
			}
			break;
		} //END case '!'

		//TODO: SCCT_XOR OK!
		case '~': {
			tok.flags = SCCTOK_OP_BITWISE|SCCTOK_OP_ARG_LITERAL;
			tok.tok = SCCT_XOR;
			tok.start_line = m_src.get_current_line();
			break;
		} //END case '~'

		//TODO: SCCT_DOT OK!
		case '.': {
			tok.flags = SCCTOK_OP_DEFAULT;
			tok.tok = SCCT_DOT;
			tok.start_line = m_src.get_current_line();
			break;
		} //END case '.'

//TODO: SCCT_DOT OK!
		case ',': {
			tok.flags = SCCTOK_OP_DEFAULT;
			tok.tok = SCCT_COMMA;
			tok.start_line = m_src.get_current_line();
			break;
		} //END case ','

		//TODO: SCCT_QUESTION OK!
		case '?': {
			tok.flags = SCCTOK_OP_DEFAULT;
			tok.tok = SCCT_QUESTION;
			tok.start_line = m_src.get_current_line();
			break;
		} //END case '?'

		default:
			/* unrecognized sequence */
			return false;
		}
		m_src.pos_increment();
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
	init_keywords();
	return SCCLEX_STATUS_OK;
}

SCCLEX_STATUS scclex::next_tok(scclex_tok& tok)
{
	/* reset token */
	tok.flags = SCCTOK_OP_DEFAULT;
	tok.string[0] = 0;
	tok.tok = SCCT_UNKNOWN;

	/* skip spaces */
	if (!m_src.skip_spaces()) {
		/* skip_spaces walked to EOF */
		make_eof_token(tok);
		return SCCLEX_STATUS_NO_MORE_DATA;
	}

	/* try read delimeters */
	if (read_delims(tok))
		return SCCLEX_STATUS_OK;

	/* try read identifs and keywords */
	if (read_alpha(tok))
		return SCCLEX_STATUS_OK;

	/* try read numeric */
	if (read_alpha(tok))
		return SCCLEX_STATUS_OK;

	/* try read numbers */
	if(read_numeric(tok))
		return SCCLEX_STATUS_OK;

	/* nothing not match. is end position??! */
	if (m_src.is_end())
		return SCCLEX_STATUS_NO_MORE_DATA;

	/* input char is invalid */
	return SCCLEX_STATUS_INVALID_CHAR;
}
