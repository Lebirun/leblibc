#define _BSD_SOURCE
#include <nl_types.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>
#include <errno.h>
#include <langinfo.h>
#include <locale.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "libc.h"

#define V(p) be32toh(*(uint32_t *)(p))

static nl_catd do_catopen(const char *name)
{
	size_t size;
	const unsigned char *map = __map_file(name, &size);
	
	if (!map || V(map) != 0xff88ff89 || 20+V(map+8) != size) {
		if(map) munmap((void *)map, size);
		errno = ENOENT;
		return (nl_catd)-1;
	}
	return (nl_catd)map;
}

static int catopen_reserve(char **buffer, size_t *capacity, size_t need)
{
	char *grown;
	size_t next;

	if (need <= *capacity) return 0;
	next = *capacity;
	while (next < need) {
		if (next > SIZE_MAX/2) {
			next = need;
			break;
		}
		next *= 2;
	}
	grown = realloc(*buffer, next);
	if (!grown) return -1;
	*buffer = grown;
	*capacity = next;
	return 0;
}

nl_catd catopen(const char *name, int oflag)
{
	nl_catd catd;
	char *buf;
	size_t capacity;
	size_t i;
	const char *path, *lang, *p, *z;
	const char *v;
	size_t l;

	if (strchr(name, '/')) return do_catopen(name);
	capacity = 64;
	buf = malloc(capacity);
	if (!buf) return (nl_catd)-1;
	if (libc.secure || !(path = getenv("NLSPATH"))) {
		free(buf);
		errno = ENOENT;
		return (nl_catd)-1;
	}
	lang = oflag ? nl_langinfo(_NL_LOCALE_NAME(LC_MESSAGES)) : getenv("LANG");
	if (!lang) lang = "";
	for (p=path; *p; p=z) {
		i = 0;
		z = __strchrnul(p, ':');
		for (; p<z; p++) {
			if (*p!='%') v=p, l=1;
			else switch (*++p) {
			case 'N': v=name; l=strlen(v); break;
			case 'L': v=lang; l=strlen(v); break;
			case 'l': v=lang; l=strcspn(v,"_.@"); break;
			case 't':
				v=__strchrnul(lang,'_');
				if (*v) v++;
				l=strcspn(v,".@");
				break;
			case 'c': v="UTF-8"; l=5; break;
			case '%': v="%"; l=1; break;
			default: v=0;
			}
			if (!v || i > SIZE_MAX-l-1 ||
			    catopen_reserve(&buf, &capacity, i+l+1)) {
				break;
			}
			memcpy(buf+i, v, l);
			i += l;
		}
		if (!*z && (p<z || !i)) break;
		if (p<z) continue;
		if (*z) z++;
		buf[i] = 0;
		
		catd = do_catopen(i ? buf : name);
		if (catd != (nl_catd)-1) {
			free(buf);
			return catd;
		}
	}
	free(buf);
	errno = ENOENT;
	return (nl_catd)-1;
}
