#ifndef MCRES_LICENSE_CLIENT_UTIL_H
#define MCRES_LICENSE_CLIENT_UTIL_H

#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>

void setupJni(JNIEnv * jni);
void callJavaStringMethod(char * class, char * name, char * str);
char * fetchJarLocation();
long currentTimeMillis(void);
char * getDataFolder();
bool fetchResourceInfo(unsigned int * userId, unsigned int * resourceId, char ** order, size_t * orderLen);
void memcpyInversed(void * target, const void * source, size_t length);

#endif //MCRES_LICENSE_CLIENT_UTIL_H
