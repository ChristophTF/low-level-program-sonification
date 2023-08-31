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
	
	unsigned int res = rand();
	
	// Fast loop: Simple arithmetic instructions
	for(size_t i = 0; i < iterations * 20; i++)
	{
		res = res * 3 + 27;
	}

	// Slower loop: Call into standard library
	for(size_t i = 0; i < iterations; i++)
	{
		res += rand();
	}
	
	printf("%u\n", res);
}

