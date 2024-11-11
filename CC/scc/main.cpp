#include "scc.h"
#include "scctest.h"
#include "bytewriter.h"

const int maxbits = sizeof(int) * 8;

bool str2bits(int &value, const char *p_src, int nbits)
{
	value = 0;
	if (nbits > maxbits)
		nbits = maxbits;

	for (int i = 0; i < nbits; i++) {
		if (p_src[i] == '0') {
			value &= ~(1 << i);
			continue;
		}
		if (p_src[i] == '1') {
			value |= (1 << i);
			continue;
		}
		return false;
	}
	return true;
}

void bits2str(char *p_dst, int maxlen, int d, int nbits)
{
	int i;
	for (i = 0; i < maxlen && i < nbits; i++)
		p_dst[i] = (d & (1 << i)) ? '1' : '0';

	p_dst[i] = '\0';
}

inline void bitprint(const char *p_pref, int d, int nbits = 8)
{
	char text[64];
	bits2str(text, sizeof(text), d, nbits);
	printf("%s: %s\n", p_pref, text);
}

inline void bitread(int& dst, int nbits=8)
{
	char text[64];
	do {
		putc('\n', stdout);
		scanf_s("%s", text, (unsigned int)sizeof(text));
	} while (!str2bits(dst, text, nbits));
}

int main()
{
	scclex_test();
  return 0;
}