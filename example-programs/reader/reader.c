#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
		exit(1);
	}

	size_t iterations = atoi(argv[1]);

	unsigned int res = 0;
	for(size_t i = 0; i < iterations; i++)
	{
		res += getchar_unlocked();
	}
	
	printf("%u\n", res);
}
