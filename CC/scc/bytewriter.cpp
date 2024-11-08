#include "bytewriter.h"

bytewriter::bytewriter()
{
}

bytewriter::~bytewriter()
{
}

void bytewriter::write8(int32_t dw)
{
}

void bytewriter::write16(int32_t dw)
{
}

void bytewriter::write32(int32_t dw)
{
}

size_t bytewriter::get_size()
{
	return m_bytes.size();
}

uint8_t* bytewriter::get_data()
{
	return m_bytes.data();
}
