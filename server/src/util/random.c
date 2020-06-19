#include <stdlib.h>
#include <time.h>

int randInt(int maxValue, int minValue)
{
	srand((unsigned) time(NULL));
	int first = rand();
	int second = rand();
	
	return minValue + ((first * second) % (maxValue - minValue));
}
