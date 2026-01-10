#include <ftw.h>

int ftw(const char *path, int (*fn)(const char *, const struct stat *, int), int fd_limit)
{
	
	return nftw(path, (int (*)())fn, fd_limit, FTW_PHYS);
}
