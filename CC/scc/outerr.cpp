#include "outerr.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

const char* ccmsg_tag_from_type(CCMSG_TYPE t)
{
	static const char* p_tags[CCMSG_MAX_TYPES] = {
		"warning", "error", "internal error"
	};
	assert((int)t < CCMSG_MAX_TYPES && "message type tag out of bounds!");
	return p_tags[(int)t];
}

void cc_msg::msg(int num, CCMSG_TYPE t)
{
}

void cc_msg::msg(CCMSG_TYPE t, const char* pf, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, pf);
	vsnprintf(buf, sizeof(buf), pf, ap);
	va_end(ap);
	printf("%s: %s\n", ccmsg_tag_from_type(t), buf);
}

void cc_msg::phello()
{
	printf(
		"-----------------------------------\n"
		" Simple C compiler for bytecode VM \n"
		" by Deryabin K. \n"
		"-----------------------------------\n"
	);
}
