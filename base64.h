#ifndef BASE64_H
#define BASE64_H

unsigned int
base64_encode(const unsigned char *in, unsigned int inlen, char *out);

#endif

/*

#include "base64.h"

int	main()
{
	char a[10000];
	int len = base64_encode("4242", 4, a);
	printf("%s\n", a);
	base64_decode(a, len, a);
	printf("%s\n", a);
}

*/