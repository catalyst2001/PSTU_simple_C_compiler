#pragma once

namespace scctest
{
	void print_with_CRLF(const char* p_text);
};

namespace scctests
{
	bool test_transform_CR_to_unix();

	/* all tests */
	bool run_all_tests();
};

