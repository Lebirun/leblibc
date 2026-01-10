#include <time.h>
#include <stdio.h>
#include <langinfo.h>
#include "locale_impl.h"
#include "atomic.h"

char *__asctime_r(const struct tm *restrict tm, char *restrict buf)
{
	if (snprintf(buf, 26, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
		__nl_langinfo_l(ABDAY_1+tm->tm_wday, C_LOCALE),
		__nl_langinfo_l(ABMON_1+tm->tm_mon, C_LOCALE),
		tm->tm_mday, tm->tm_hour,
		tm->tm_min, tm->tm_sec,
		1900 + tm->tm_year) >= 26)
	{
		
		a_crash();
	}
	return buf;
}

weak_alias(__asctime_r, asctime_r);
