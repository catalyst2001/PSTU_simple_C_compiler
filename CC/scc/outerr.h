#pragma once

enum CCMSG_TYPE : unsigned int {
	CCMSG_TYPE_WARN = 0,//warning
	CCMSG_TYPE_ERROR, //error
	CCMSG_TYPE_INTRERR, //internal error

	CCMSG_MAX_TYPES
};

class cc_msg
{
public:
	static void msg(int num, CCMSG_TYPE t);
	static void msg(CCMSG_TYPE t, const char *pf, ...);
	static void phello();
};