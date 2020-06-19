#include "util.h"

long currentTimeMillis(void)
{
	long            ms; // Milliseconds
	time_t          s;  // Seconds
	struct timespec spec;
	
	clock_gettime(CLOCK_REALTIME, &spec);
	
	s  = spec.tv_sec;
	ms = (long) round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
	if (ms > 999) {
		s++;
		ms = 0;
	}
	
	return s * 1000 + ms;
}
