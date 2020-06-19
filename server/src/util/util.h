#ifndef MCRES_LICENSE_SERVER_UTIL_H
#define MCRES_LICENSE_SERVER_UTIL_H

#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <memory.h>

long currentTimeMillis(void);
int randInt(int maxValue, int minValue);
bool needExit();
void memcpyInversed(void * target, const void * source, size_t length);
void exit();

#endif //MCRES_LICENSE_SERVER_UTIL_H
