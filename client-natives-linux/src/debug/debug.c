#include "debug.h"

#ifdef CLIENT_DEBUG
int debuggingPid = 0;
#endif

void triggerGcc()
{
#ifdef CLIENT_DEBUG
	int pid = fork();
	
	if (pid < 0)
		abort();
	else if (pid)
	{
		debuggingPid = pid;
		sleep(50);
	}
	else
	{
		char commandLine[255] = { 0 };
		sprintf(commandLine, "--pid=%d", debuggingPid);
		execl("ddd", "ddd", "--debugger", "gdb", commandLine, (char *) 0);
	}
#endif // CLIENT_DEBUG
}
