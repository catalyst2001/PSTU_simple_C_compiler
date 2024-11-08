#pragma once
#include <vector>

class bytewriter
{
	std::vector<uint8_t> m_bytes;
public:
	bytewriter();
	~bytewriter();
	void write8(int32_t dw);
	void write16(int32_t dw);
	void write32(int32_t dw);

	size_t   get_size();
	uint8_t* get_data();
};

