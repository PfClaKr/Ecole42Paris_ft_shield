#ifndef BASE64_H
#define BASE64_H

unsigned int
base64_encode(const unsigned char *in, unsigned int inlen, char *out);

#endif

/*

#include "base64.h"

int	main()
{
	char a[100];
	int len = base64_encode("password", 8, a);
	printf("%s\n", a);
}

*/