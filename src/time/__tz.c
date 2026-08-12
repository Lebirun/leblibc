#include "time_impl.h"
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <ctype.h>
#include "libc.h"
#include "lock.h"
#include "fork_impl.h"

#define malloc __libc_malloc
#define calloc undef
#define realloc undef
#define free undef

long  __timezone = 0;
int   __daylight = 0;
char *__tzname[2] = { 0, 0 };

weak_alias(__timezone, timezone);
weak_alias(__daylight, daylight);
weak_alias(__tzname, tzname);

static char std_name_inline[TZNAME_MAX+1];
static char dst_name_inline[TZNAME_MAX+1];
static char *std_name = std_name_inline;
static char *dst_name = dst_name_inline;
static size_t std_name_size = sizeof std_name_inline;
static size_t dst_name_size = sizeof dst_name_inline;

static int dst_off;
static int r0[5], r1[5];

static const unsigned char *zi, *trans, *index, *types, *abbrevs, *abbrevs_end;
static size_t map_size;

static char old_tz_buf[32];
static char *old_tz = old_tz_buf;
static size_t old_tz_size = sizeof old_tz_buf;

static volatile int lock[1];
volatile int *const __timezone_lockptr = lock;

static int getint(const char **p)
{
	unsigned x;
	for (x=0; **p-'0'<10U; (*p)++) x = **p-'0' + 10*x;
	return x;
}

static int getoff(const char **p)
{
	int neg = 0;
	if (**p == '-') {
		++*p;
		neg = 1;
	} else if (**p == '+') {
		++*p;
	}
	int off = 3600*getint(p);
	if (**p == ':') {
		++*p;
		off += 60*getint(p);
		if (**p == ':') {
			++*p;
			off += getint(p);
		}
	}
	return neg ? -off : off;
}

static void getrule(const char **p, int rule[5])
{
	int r = rule[0] = **p;

	if (r!='M') {
		if (r=='J') ++*p;
		else rule[0] = 0;
		rule[1] = getint(p);
	} else {
		++*p; rule[1] = getint(p);
		++*p; rule[2] = getint(p);
		++*p; rule[3] = getint(p);
	}

	if (**p=='/') {
		++*p;
		rule[4] = getoff(p);
	} else {
		rule[4] = 7200;
	}
}

static void getname(char **destination, size_t *capacity,
                    char *inline_buffer, const char **p)
{
	const char *start;
	char *grown;
	size_t length;

	if (**p == '<') {
		++*p;
		start = *p;
		while (**p && **p != '>') ++*p;
		length = (size_t)(*p-start);
		if (**p) ++*p;
	} else {
		start = *p;
		while ((**p|32)-'a'<26U) ++*p;
		length = (size_t)(*p-start);
	}
	if (length == SIZE_MAX) length = 0;
	if (length+1 > *capacity) {
		grown = malloc(length+1);
		if (grown) {
			if (*destination != inline_buffer) __libc_free(*destination);
			*destination = grown;
			*capacity = length+1;
		}
	}
	if (length+1 <= *capacity) {
		memcpy(*destination, start, length);
		(*destination)[length] = 0;
	} else {
		length = *capacity-1;
		memcpy(*destination, start, length);
		(*destination)[length] = 0;
	}
}

#define VEC(...) ((const unsigned char[]){__VA_ARGS__})

static uint32_t zi_read32(const unsigned char *z)
{
	return (unsigned)z[0]<<24 | z[1]<<16 | z[2]<<8 | z[3];
}

static size_t zi_dotprod(const unsigned char *z, const unsigned char *v, size_t n)
{
	size_t y;
	uint32_t x;
	for (y=0; n; n--, z+=4, v++) {
		x = zi_read32(z);
		y += x * *v;
	}
	return y;
}

static void do_tzset()
{
	char *pathbuf = 0;
	char *pathname;
	char *grown_tz;
	char dummy_inline[TZNAME_MAX+1];
	char *dummy_name;
	size_t dummy_size;
	const char *try, *s, *p;
	const unsigned char *map = 0;
	size_t i;
	size_t l;
	size_t skip;
	size_t grown_size;
	int posix_form;
	int scale;
	const unsigned char *zone_type;
	static const char search[] =
		"/usr/share/zoneinfo/\0/share/zoneinfo/\0/etc/zoneinfo/\0";

	s = getenv("TZ");
	if (!s) s = "/etc/localtime";
	if (!*s) s = __utc;

	if (old_tz && !strcmp(s, old_tz)) return;

	for (i=0; i<5; i++) r0[i] = r1[i] = 0;

	if (zi) __munmap((void *)zi, map_size);

	
	i = strlen(s);
	if (i >= old_tz_size) {
		if (i == SIZE_MAX) s = __utc, i = 3;
		else {
			grown_size = i+1;
			grown_tz = malloc(grown_size);
			if (grown_tz) {
				if (old_tz != old_tz_buf) __libc_free(old_tz);
				old_tz = grown_tz;
				old_tz_size = grown_size;
			}
		}
	}
	if (i < old_tz_size) memcpy(old_tz, s, i+1);
	else old_tz[0] = 0;

	posix_form = 0;
	if (*s != ':') {
		p = s;
		dummy_name = dummy_inline;
		dummy_size = sizeof dummy_inline;
		getname(&dummy_name, &dummy_size, dummy_inline, &p);
		if (p!=s && (*p == '+' || *p == '-' || isdigit(*p)
		             || !strcmp(dummy_name, "UTC")
		             || !strcmp(dummy_name, "GMT")))
			posix_form = 1;
		if (dummy_name != dummy_inline) __libc_free(dummy_name);
	}	

	
	if (!posix_form) {
		if (*s == ':') s++;
		if (*s == '/' || *s == '.') {
			if (!libc.secure || !strcmp(s, "/etc/localtime"))
				map = __map_file(s, &map_size);
		} else {
			l = strlen(s);
			if (l <= SIZE_MAX-25 && !strchr(s, '.') &&
			    (pathbuf = malloc(l+25))) {
				pathname = pathbuf+24;
				memcpy(pathname, s, l+1);
				pathname[l] = 0;
				for (try=search; !map && *try; try+=l+1) {
					l = strlen(try);
					memcpy(pathname-l, try, l);
					map = __map_file(pathname-l, &map_size);
				}
				__libc_free(pathbuf);
				pathbuf = 0;
			}
		}
		if (!map) s = __utc;
	}
	if (map && (map_size < 44 || memcmp(map, "TZif", 4))) {
		__munmap((void *)map, map_size);
		map = 0;
		s = __utc;
	}

	zi = map;
	if (map) {
		scale = 2;
		if (map[4]!='1') {
			skip = zi_dotprod(zi+20, VEC(1,1,8,5,6,1), 6);
			trans = zi+skip+44+44;
			scale++;
		} else {
			trans = zi+44;
		}
		index = trans + (zi_read32(trans-12) << scale);
		types = index + zi_read32(trans-12);
		abbrevs = types + 6*zi_read32(trans-8);
		abbrevs_end = abbrevs + zi_read32(trans-4);
		if (zi[map_size-1] == '\n') {
			for (s = (const char *)zi+map_size-2; *s!='\n'; s--);
			s++;
		} else {
			zone_type = types;
			__tzname[0] = __tzname[1] = 0;
			__daylight = __timezone = dst_off = 0;
			for (; zone_type<abbrevs; zone_type+=6) {
				if (!zone_type[4] && !__tzname[0]) {
					__tzname[0] = (char *)abbrevs + zone_type[5];
					__timezone = -zi_read32(zone_type);
				}
				if (zone_type[4] && !__tzname[1]) {
					__tzname[1] = (char *)abbrevs + zone_type[5];
					dst_off = -zi_read32(zone_type);
					__daylight = 1;
				}
			}
			if (!__tzname[0]) __tzname[0] = __tzname[1];
			if (!__tzname[0]) __tzname[0] = (char *)__utc;
			if (!__daylight) {
				__tzname[1] = __tzname[0];
				dst_off = __timezone;
			}
			return;
		}
	}

	if (!s) s = __utc;
	getname(&std_name, &std_name_size, std_name_inline, &s);
	__tzname[0] = std_name;
	__timezone = getoff(&s);
	getname(&dst_name, &dst_name_size, dst_name_inline, &s);
	__tzname[1] = dst_name;
	if (dst_name[0]) {
		__daylight = 1;
		if (*s == '+' || *s=='-' || *s-'0'<10U)
			dst_off = getoff(&s);
		else
			dst_off = __timezone - 3600;
	} else {
		__daylight = 0;
		dst_off = __timezone;
	}

	if (*s == ',') s++, getrule(&s, r0);
	if (*s == ',') s++, getrule(&s, r1);
}

static size_t scan_trans(long long t, int local, size_t *alt)
{
	int scale = 3 - (trans == zi+44);
	uint64_t x;
	int off = 0;

	size_t a = 0, n = (index-trans)>>scale, m;

	if (!n) {
		if (alt) *alt = 0;
		return 0;
	}

	
	while (n > 1) {
		m = a + n/2;
		x = zi_read32(trans + (m<<scale));
		if (scale == 3) x = x<<32 | zi_read32(trans + (m<<scale) + 4);
		else x = (int32_t)x;
		if (local) off = (int32_t)zi_read32(types + 6 * index[m-1]);
		if (t - off < (int64_t)x) {
			n /= 2;
		} else {
			a = m;
			n -= n/2;
		}
	}

	
	n = (index-trans)>>scale;
	if (a == n-1) return -1;
	if (a == 0) {
		x = zi_read32(trans);
		if (scale == 3) x = x<<32 | zi_read32(trans + 4);
		else x = (int32_t)x;
		
		size_t j = 0;
		for (size_t i=abbrevs-types; i; i-=6) {
			if (!types[i-6+4]) j = i-6;
		}
		if (local) off = (int32_t)zi_read32(types + j);
		
		if (t - off < (int64_t)x) {
			if (alt) *alt = index[0];
			return j/6;
		}
	}

	
	if (alt) {
		if (a && types[6*index[a-1]+4] != types[6*index[a]+4])
			*alt = index[a-1];
		else if (a+1<n && types[6*index[a+1]+4] != types[6*index[a]+4])
			*alt = index[a+1];
		else
			*alt = index[a];
	}

	return index[a];
}

static int days_in_month(int m, int is_leap)
{
	if (m==2) return 28+is_leap;
	else return 30+((0xad5>>(m-1))&1);
}

static long long rule_to_secs(const int *rule, int year)
{
	int is_leap;
	long long t = __year_to_secs(year, &is_leap);
	int x, m, n, d;
	if (rule[0]!='M') {
		x = rule[1];
		if (rule[0]=='J' && (x < 60 || !is_leap)) x--;
		t += 86400 * x;
	} else {
		m = rule[1];
		n = rule[2];
		d = rule[3];
		t += __month_to_secs(m-1, is_leap);
		int wday = (int)((t + 4*86400) % (7*86400)) / 86400;
		int days = d - wday;
		if (days < 0) days += 7;
		if (n == 5 && days+28 >= days_in_month(m, is_leap)) n = 4;
		t += 86400 * (days + 7*(n-1));
	}
	t += rule[4];
	return t;
}

void __secs_to_zone(long long t, int local, int *isdst, long *offset, long *oppoff, const char **zonename)
{
	LOCK(lock);

	do_tzset();

	if (zi) {
		size_t alt, i = scan_trans(t, local, &alt);
		if (i != -1) {
			*isdst = types[6*i+4];
			*offset = (int32_t)zi_read32(types+6*i);
			*zonename = (const char *)abbrevs + types[6*i+5];
			if (oppoff) *oppoff = (int32_t)zi_read32(types+6*alt);
			UNLOCK(lock);
			return;
		}
	}

	if (!__daylight) goto std;

	
	long long y = t / 31556952 + 70;
	while (__year_to_secs(y, 0) > t) y--;
	while (__year_to_secs(y+1, 0) < t) y++;

	long long t0 = rule_to_secs(r0, y);
	long long t1 = rule_to_secs(r1, y);

	if (!local) {
		t0 += __timezone;
		t1 += dst_off;
	}
	if (t0 < t1) {
		if (t >= t0 && t < t1) goto dst;
		goto std;
	} else {
		if (t >= t1 && t < t0) goto std;
		goto dst;
	}
std:
	*isdst = 0;
	*offset = -__timezone;
	if (oppoff) *oppoff = -dst_off;
	*zonename = __tzname[0];
	UNLOCK(lock);
	return;
dst:
	*isdst = 1;
	*offset = -dst_off;
	if (oppoff) *oppoff = -__timezone;
	*zonename = __tzname[1];
	UNLOCK(lock);
}

static void __tzset()
{
	LOCK(lock);
	do_tzset();
	UNLOCK(lock);
}

weak_alias(__tzset, tzset);

const char *__tm_to_tzname(const struct tm *tm)
{
	const void *p = tm->__tm_zone;
	LOCK(lock);
	do_tzset();
	if (p != __utc && p != __tzname[0] && p != __tzname[1] &&
	    (!zi || (uintptr_t)p-(uintptr_t)abbrevs >= abbrevs_end - abbrevs))
		p = "";
	UNLOCK(lock);
	return p;
}
