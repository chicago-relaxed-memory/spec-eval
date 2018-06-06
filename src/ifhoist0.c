#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define SECRET 243987

int main() {
	srand(time(NULL));
	int x = rand();
	int y;
	int z = 1;
	if(x) {
		y = 243987;
		z = rand();
	} else {
		y = SECRET;
	}
	printf("%i %i %i\n", y, z, x);
}
