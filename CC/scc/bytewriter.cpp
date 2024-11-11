#include "bytewriter.h"

bytewriter::bytewriter()
{
}

bytewriter::~bytewriter()
{
}

void bytewriter::write8(int32_t dw)
{
	m_bytes.push_back(dw & 0xff);
}

void bytewriter::write16(int32_t dw)
{
	m_bytes.push_back(dw & 0xff);
	m_bytes.push_back((dw >> 8) & 0xff);
}

void bytewriter::write32(int32_t dw)
{
	m_bytes.push_back(dw & 0xff);
	m_bytes.push_back((dw >> 8) & 0xff);
	m_bytes.push_back((dw >> 16) & 0xff);
	m_bytes.push_back((dw >> 24) & 0xff);
}

size_t bytewriter::get_size()
{
	return m_bytes.size();
}

uint8_t* bytewriter::get_data()
{
	return m_bytes.data();
}
