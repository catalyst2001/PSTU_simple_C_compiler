#include "outerr.h"
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

void cc_print(const char* p_str)
{
	printf(p_str);
}

void cc_error(const char* p_str)
{
	printf("error: %s\n", p_str);
}

void cc_warn(const char* p_str)
{
	printf("warning: %s\n", p_str);
}

void cc_fatal(const char* p_str)
{
	printf("internal error: %s\n", p_str);
}

const char* cc_format(const char* p_format, ...)
{
	static char buf[8092];
	va_list ap;
	va_start(ap, p_format);
	vsnprintf(buf, sizeof(buf), p_format, ap);
	va_end(ap);
	return buf;
}
