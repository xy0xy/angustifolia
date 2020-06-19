#include "util.h"

bool need_exit = false;

bool needExit()
{
	return need_exit;
}

void exit()
{
	need_exit = true;
}
