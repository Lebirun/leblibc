#define _GNU_SOURCE
#include <grp.h>
#include <limits.h>
#include <stdlib.h>

int initgroups(const char *user, gid_t gid)
{
	gid_t buf[32], *groups = buf;
	int count = sizeof buf / sizeof *buf, prev_count = count;
	int ret;

	while (getgrouplist(user, gid, groups, &count) < 0) {
		if (groups != buf) free(groups);

		if (count <= prev_count)
			return -1;

		if (count < prev_count + (prev_count>>1))
			count = prev_count + (prev_count>>1);

		groups = calloc(count, sizeof *groups);
		if (!groups) return -1;
		prev_count = count;
	}
	ret = setgroups(count, groups);
	if (groups != buf) free(groups);
	return ret;
}
