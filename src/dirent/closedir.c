#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include "__dirent.h"

int closedir(DIR *dir)
{
	int ret;

	ret = close(dir->fd);
	free(dir->buf);
	free(dir);
	return ret;
}
