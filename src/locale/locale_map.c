#include <locale.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "locale_impl.h"
#include "libc.h"
#include "lock.h"
#include "fork_impl.h"

#define malloc __libc_malloc
#define calloc undef
#define realloc undef
#define free undef

const char *__lctrans_impl(const char *msg, const struct __locale_map *lm)
{
	const char *trans = 0;
	if (lm) trans = __mo_lookup(lm->map, lm->map_size, msg);
	return trans ? trans : msg;
}

static const char envvars[][12] = {
	"LC_CTYPE",
	"LC_NUMERIC",
	"LC_TIME",
	"LC_COLLATE",
	"LC_MONETARY",
	"LC_MESSAGES",
};

volatile int __locale_lock[1];
volatile int *const __locale_lockptr = __locale_lock;

const struct __locale_map *__get_locale(int cat, const char *val)
{
	static void *volatile loc_head;
	const struct __locale_map *p;
	struct __locale_map *new = 0;
	const char *path = 0, *z;
	char *buf;
	size_t l, n;
	size_t path_size;
	size_t map_size;
	const void *map;
	int builtin;

	if (!*val) {
		(val = getenv("LC_ALL")) && *val ||
		(val = getenv(envvars[cat])) && *val ||
		(val = getenv("LANG")) && *val ||
		(val = "C.UTF-8");
	}

	
	for (n=0; val[n] && val[n]!='/'; n++);
	if (val[0]=='.' || val[n]) val = "C.UTF-8";
	builtin = (val[0]=='C' && !val[1])
		|| !strcmp(val, "C.UTF-8")
		|| !strcmp(val, "POSIX");

	if (builtin) {
		if (cat == LC_CTYPE && val[1]=='.')
			return (void *)&__c_dot_utf8;
		return 0;
	}

	for (p=loc_head; p; p=p->next)
		if (!strcmp(val, p->name)) return p;

	if (!libc.secure) path = getenv("MUSL_LOCPATH");
	buf = 0;

	if (path) for (; *path; path=z+!!*z) {
		z = __strchrnul(path, ':');
		l = z - path;
		if (l > SIZE_MAX-n-2) continue;
		path_size = l+n+2;
		buf = malloc(path_size);
		if (!buf) break;
		memcpy(buf, path, l);
		buf[l] = '/';
		memcpy(buf+l+1, val, n);
		buf[l+1+n] = 0;
		map = __map_file(buf, &map_size);
		__libc_free(buf);
		buf = 0;
		if (map) {
			if (n > SIZE_MAX-sizeof *new-1) {
				__munmap((void *)map, map_size);
				break;
			}
			new = malloc(sizeof *new+n+1);
			if (!new) {
				__munmap((void *)map, map_size);
				break;
			}
			new->map = map;
			new->map_size = map_size;
			new->name = (char *)(new+1);
			memcpy((char *)new->name, val, n);
			((char *)new->name)[n] = 0;
			new->next = loc_head;
			loc_head = new;
			break;
		}
	}

	
	if (!new && n <= SIZE_MAX-sizeof *new-1 &&
	    (new = malloc(sizeof *new+n+1))) {
		new->map = __c_dot_utf8.map;
		new->map_size = __c_dot_utf8.map_size;
		new->name = (char *)(new+1);
		memcpy((char *)new->name, val, n);
		((char *)new->name)[n] = 0;
		new->next = loc_head;
		loc_head = new;
	}

	
	if (!new && cat == LC_CTYPE) new = (void *)&__c_dot_utf8;

	return new;
}
