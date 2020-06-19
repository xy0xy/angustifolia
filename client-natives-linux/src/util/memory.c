#include "util.h"

void memcpyInversed(void * target, const void * source, size_t length)
{
	void * data = target + length - 1;
	void * tempSource = (void *)source;
	
	for (size_t i = 0; i < length; i ++)
	{
		memcpy(data, tempSource ++, 1);
		data --;
	}
}
