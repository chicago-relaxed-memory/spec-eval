#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef SECRET
#error Please define SECRET when compiling (e.g. pass -DSECRET=123 to the compiler)
#endif

int main() {
	srand(time(NULL));
	int x = rand();
	int y;
	int z = 1;
	if(x) {
		y = 1;
		z = rand();
	} else {
		y = SECRET;
	}
	printf("%i %i %i\n", y, z, x);
}
