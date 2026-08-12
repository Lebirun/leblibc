#include <ftw.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>

struct history
{
	struct history *chain;
	dev_t dev;
	ino_t ino;
	int level;
	int base;
};

#undef dirfd
#define dirfd(d) (*(int *)d)

static int do_nftw(char *path, int (*fn)(const char *, const struct stat *, int, struct FTW *), int fd_limit, int flags, struct history *h)
{
	size_t l = strlen(path), j = l && path[l-1]=='/' ? l-1 : l;
	size_t name_len;
	size_t child_len;
	char *child;
	struct stat st;
	struct history new;
	int type;
	int r;
	int dfd;
	int err;
	struct FTW lev;

	st.st_dev = st.st_ino = 0;

	if ((flags & FTW_PHYS) ? lstat(path, &st) : stat(path, &st) < 0) {
		if (!(flags & FTW_PHYS) && errno==ENOENT && !lstat(path, &st))
			type = FTW_SLN;
		else if (errno != EACCES) return -1;
		else type = FTW_NS;
	} else if (S_ISDIR(st.st_mode)) {
		if (flags & FTW_DEPTH) type = FTW_DP;
		else type = FTW_D;
	} else if (S_ISLNK(st.st_mode)) {
		if (flags & FTW_PHYS) type = FTW_SL;
		else type = FTW_SLN;
	} else {
		type = FTW_F;
	}

	if ((flags & FTW_MOUNT) && h && type != FTW_NS && st.st_dev != h->dev)
		return 0;
	
	new.chain = h;
	new.dev = st.st_dev;
	new.ino = st.st_ino;
	new.level = h ? h->level+1 : 0;
	new.base = j+1;
	
	lev.level = new.level;
	if (h) {
		lev.base = h->base;
	} else {
		size_t k;
		for (k=j; k && path[k]=='/'; k--);
		for (; k && path[k-1]!='/'; k--);
		lev.base = k;
	}

	if (type == FTW_D || type == FTW_DP) {
		dfd = open(path, O_RDONLY);
		err = errno;
		if (dfd < 0 && err == EACCES) type = FTW_DNR;
		if (!fd_limit) close(dfd);
	}

	if (!(flags & FTW_DEPTH) && (r=fn(path, &st, type, &lev)))
		return r;

	for (; h; h = h->chain)
		if (h->dev == st.st_dev && h->ino == st.st_ino)
			return 0;

	if ((type == FTW_D || type == FTW_DP) && fd_limit) {
		if (dfd < 0) {
			errno = err;
			return -1;
		}
		DIR *d = fdopendir(dfd);
		if (d) {
			struct dirent *de;
			while ((de = readdir(d))) {
				if (de->d_name[0] == '.'
				 && (!de->d_name[1]
				  || (de->d_name[1]=='.'
				   && !de->d_name[2]))) continue;
				name_len = strlen(de->d_name);
				if (name_len > SIZE_MAX-2 || j > SIZE_MAX-name_len-2) {
					errno = ENOMEM;
					closedir(d);
					return -1;
				}
				child_len = j+name_len+2;
				child = malloc(child_len);
				if (!child) {
					closedir(d);
					return -1;
				}
				memcpy(child, path, j);
				child[j]='/';
				memcpy(child+j+1, de->d_name, name_len+1);
				r = do_nftw(child, fn, fd_limit-1, flags, &new);
				free(child);
				if (r) {
					closedir(d);
					return r;
				}
			}
			closedir(d);
		} else {
			close(dfd);
			return -1;
		}
	}

	path[l] = 0;
	if ((flags & FTW_DEPTH) && (r=fn(path, &st, type, &lev)))
		return r;

	return 0;
}

int nftw(const char *path, int (*fn)(const char *, const struct stat *, int, struct FTW *), int fd_limit, int flags)
{
	int r, cs;
	char *pathbuf;

	if (fd_limit <= 0) return 0;

	pathbuf = strdup(path);
	if (!pathbuf) return -1;
	
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	r = do_nftw(pathbuf, fn, fd_limit, flags, NULL);
	pthread_setcancelstate(cs, 0);
	free(pathbuf);
	return r;
}
