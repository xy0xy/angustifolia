#include <memory.h>
#include "motd.h"
#include "../util/util.h"

// ☭

FILE * fetch_motd_file()
{
	FILE * f = fopen("motd", "r");
	if (f == NULL)
	{
		f = fopen("motd", "w");
		fputs("env.country==\"cn\"#aHR0cHM6Ly9iMjMudHYvYXY3MzM3MjkwMA==", f);
	}
	
	return f;
}

void get_motd(char * target, size_t * size)
{
	FILE * f = fetch_motd_file();
	fscanf(f, "%[^\n]", target);
	*size = strlen(target);
}
