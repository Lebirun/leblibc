#include <unistd.h>
#include <string.h>
#include <errno.h>

int getlogin_r(char *name, size_t size)
{
	char *logname = getlogin();
	if (!logname) return ENXIO; 
	if (strlen(logname) >= size) return ERANGE;
	strcpy(name, logname);
	return 0;
}
