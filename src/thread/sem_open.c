#include <semaphore.h>
#include <sys/mman.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include "lock.h"
#include "fork_impl.h"

#define malloc __libc_malloc
#define calloc __libc_calloc
#define realloc __libc_realloc
#define free __libc_free

static struct {
	ino_t ino;
	sem_t *sem;
	size_t refcnt;
} *semtab;
static size_t semtab_cap;
static volatile int lock[1];
volatile int *const __sem_open_lockptr = lock;

#define FLAGS (O_RDWR|O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK)

sem_t *sem_open(const char *name, int flags, ...)
{
	va_list ap;
	mode_t mode;
	unsigned value;
	int fd, e, first=1, cs;
	size_t i, slot;
	sem_t newsem;
	void *map;
	char tmp[64];
	struct timespec ts;
	struct stat st;
	char *mapped_name;
	void *newtab;
	size_t newcap;
	int saved_errno;

	mapped_name = __shm_mapname(name);
	if (!mapped_name)
		return SEM_FAILED;
	name = mapped_name;

	LOCK(lock);
	
	slot = SIZE_MAX;
	for (i=0; i<semtab_cap; i++) {
		if (!semtab[i].sem && slot == SIZE_MAX) slot = i;
	}
	
	if (slot == SIZE_MAX) {
		newcap = semtab_cap ? semtab_cap * 2 : 1;
		if (newcap <= semtab_cap || newcap > SIZE_MAX / sizeof(*semtab)) {
			errno = ENOMEM;
			UNLOCK(lock);
			free(mapped_name);
			return SEM_FAILED;
		}
		newtab = realloc(semtab, newcap * sizeof(*semtab));
		if (!newtab) {
			saved_errno = errno;
			UNLOCK(lock);
			free(mapped_name);
			errno = saved_errno;
			return SEM_FAILED;
		}
		semtab = newtab;
		memset(semtab + semtab_cap, 0,
		       (newcap - semtab_cap) * sizeof(*semtab));
		slot = semtab_cap;
		semtab_cap = newcap;
	}
	
	semtab[slot].sem = (sem_t *)-1;
	UNLOCK(lock);

	flags &= (O_CREAT|O_EXCL);

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);

	
	if (flags == (O_CREAT|O_EXCL) && access(name, F_OK) == 0) {
		errno = EEXIST;
		goto fail;
	}

	for (;;) {
		
		if (flags != (O_CREAT|O_EXCL)) {
			fd = open(name, FLAGS);
			if (fd >= 0) {
				if (fstat(fd, &st) < 0 ||
				    (map = mmap(0, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
					close(fd);
					goto fail;
				}
				close(fd);
				break;
			}
			if (errno != ENOENT)
				goto fail;
		}
		if (!(flags & O_CREAT))
			goto fail;
		if (first) {
			first = 0;
			va_start(ap, flags);
			mode = va_arg(ap, mode_t) & 0666;
			value = va_arg(ap, unsigned);
			va_end(ap);
			if (value > SEM_VALUE_MAX) {
				errno = EINVAL;
				goto fail;
			}
			sem_init(&newsem, 1, value);
		}
		
		clock_gettime(CLOCK_REALTIME, &ts);
		snprintf(tmp, sizeof(tmp), "/dev/shm/tmp-%d", (int)ts.tv_nsec);
		fd = open(tmp, O_CREAT|O_EXCL|FLAGS, mode);
		if (fd < 0) {
			if (errno == EEXIST) continue;
			goto fail;
		}
		if (write(fd, &newsem, sizeof newsem) != sizeof newsem || fstat(fd, &st) < 0 ||
		    (map = mmap(0, sizeof(sem_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
			close(fd);
			unlink(tmp);
			goto fail;
		}
		close(fd);
		e = link(tmp, name) ? errno : 0;
		unlink(tmp);
		if (!e) break;
		munmap(map, sizeof(sem_t));
		
		if (e != EEXIST || flags == (O_CREAT|O_EXCL))
			goto fail;
	}

	
	LOCK(lock);
	for (i=0; i<semtab_cap && semtab[i].ino != st.st_ino; i++);
	if (i<semtab_cap) {
		munmap(map, sizeof(sem_t));
		semtab[slot].sem = 0;
		slot = i;
		map = semtab[i].sem;
	}
	if (semtab[slot].refcnt == SIZE_MAX) {
		UNLOCK(lock);
		free(mapped_name);
		pthread_setcancelstate(cs, 0);
		errno = EMFILE;
		return SEM_FAILED;
	}
	semtab[slot].refcnt++;
	semtab[slot].sem = map;
	semtab[slot].ino = st.st_ino;
	UNLOCK(lock);
	free(mapped_name);
	pthread_setcancelstate(cs, 0);
	return map;

fail:
	saved_errno = errno;
	free(mapped_name);
	pthread_setcancelstate(cs, 0);
	LOCK(lock);
	semtab[slot].sem = 0;
	UNLOCK(lock);
	errno = saved_errno;
	return SEM_FAILED;
}

int sem_close(sem_t *sem)
{
	size_t i;
	size_t active;
	void *oldtab;

	LOCK(lock);
	for (i=0; i<semtab_cap && semtab[i].sem != sem; i++);
	if (i == semtab_cap) {
		UNLOCK(lock);
		errno = EINVAL;
		return -1;
	}
	if (--semtab[i].refcnt) {
		UNLOCK(lock);
		return 0;
	}
	semtab[i].sem = 0;
	semtab[i].ino = 0;
	active = 0;
	for (i=0; i<semtab_cap; i++) {
		if (semtab[i].sem) {
			active = 1;
			break;
		}
	}
	oldtab = 0;
	if (!active) {
		oldtab = semtab;
		semtab = 0;
		semtab_cap = 0;
	}
	UNLOCK(lock);
	if (oldtab) free(oldtab);
	munmap(sem, sizeof *sem);
	return 0;
}
