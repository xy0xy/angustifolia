#include <memory.h>
#include "hardware.h"

static inline void native_cpuid(unsigned int * eax, unsigned int * ebx, unsigned int * ecx, unsigned int * edx)
{
	asm volatile("cpuid" : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx) : "0" (*eax));
}

bool fetchHardwareInformation(char ** target, size_t * length)
{
	unsigned int eax = 1, ebx, ecx, edx;
	native_cpuid(&eax, &ebx, &ecx, &edx);
	
	unsigned long result = eax;
	eax = 2;
	native_cpuid(&eax, &ebx, &ecx, &edx);
	unsigned long temp = edx;
	temp <<= 32;
	temp |= ecx;
	result ^= temp;
	
	*length = sizeof(unsigned long);
	*target = malloc(*length);
	
	memcpy(*target, &result, *length);
	
	return true;
}


