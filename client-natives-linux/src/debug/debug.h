#ifndef MCRES_LICENSE_CLIENT_DEBUG_H
#define MCRES_LICENSE_CLIENT_DEBUG_H

//#define CLIENT_DEBUG

#ifdef CLIENT_DEBUG
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#endif // CLIENT_DEBUG

void triggerGcc();

#endif //MCRES_LICENSE_CLIENT_DEBUG_H
