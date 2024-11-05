#pragma once
#include <stdio.h>
#include <vector>
#include <string>
#include <assert.h>

/* dbgooyutput */
#if defined (_DEBUG)
#define CCDBG(f, ...) printf(__FUNCSIG__ ": " f "\n", __VA_ARGS__)
#else
#define CCDBG(f, ...) ((void)0)
#endif

#define CCDERR(x) assert(__FUNCSIG__ ":" x)
#define CCUNIMPL CCDERR("unimplemented");

/* common helpers */
template<class type>
struct vec_cc : public std::vector<type> {};