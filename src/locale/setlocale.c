#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include "locale_impl.h"
#include "libc.h"
#include "lock.h"

static char buf[LC_ALL*24];
static char *large_buf;
static size_t large_buf_size;

char *setlocale(int cat, const char *name)
{
	const struct __locale_map *lm;
	struct __locale_struct tmp_locale;
	const char *p;
	const char *z;
	const char *part;
	char *result;
	size_t length;
	size_t part_length;
	int i;
	int same;

	if ((unsigned)cat > LC_ALL) return 0;

	LOCK(__locale_lock);

	
	if (cat == LC_ALL) {
		if (name) {
			p = name;
			for (i=0; i<LC_ALL; i++) {
				z = __strchrnul(p, ';');
				part_length = (size_t)(z-p);
				result = malloc(part_length+1);
				if (!result) {
					UNLOCK(__locale_lock);
					return 0;
				}
				memcpy(result, p, part_length);
				result[part_length] = 0;
				if (*z) p = z+1;
				lm = __get_locale(i, result);
				free(result);
				if (lm == LOC_MAP_FAILED) {
					UNLOCK(__locale_lock);
					return 0;
				}
				tmp_locale.cat[i] = lm;
			}
			libc.global_locale = tmp_locale;
		}
		length = 0;
		same = 0;
		for (i=0; i<LC_ALL; i++) {
			lm = libc.global_locale.cat[i];
			if (lm == libc.global_locale.cat[0]) same++;
			part = lm ? lm->name : "C";
			part_length = strlen(part);
			if (length > SIZE_MAX-part_length-1) {
				UNLOCK(__locale_lock);
				return 0;
			}
			length += part_length+1;
		}
		if (same == LC_ALL) {
			part = libc.global_locale.cat[0] ?
				libc.global_locale.cat[0]->name : "C";
			UNLOCK(__locale_lock);
			return (char *)part;
		}
		if (length > sizeof buf) {
			if (length > large_buf_size) {
				result = realloc(large_buf, length);
				if (!result) {
					UNLOCK(__locale_lock);
					return 0;
				}
				large_buf = result;
				large_buf_size = length;
			}
			result = large_buf;
		} else {
			result = buf;
		}
		length = 0;
		for (i=0; i<LC_ALL; i++) {
			lm = libc.global_locale.cat[i];
			part = lm ? lm->name : "C";
			part_length = strlen(part);
			memcpy(result+length, part, part_length);
			length += part_length;
			result[length++] = ';';
		}
		result[length-1] = 0;
		UNLOCK(__locale_lock);
		return result;
	}

	if (name) {
		lm = __get_locale(cat, name);
		if (lm == LOC_MAP_FAILED) {
			UNLOCK(__locale_lock);
			return 0;
		}
		libc.global_locale.cat[cat] = lm;
	} else {
		lm = libc.global_locale.cat[cat];
	}
	result = lm ? (char *)lm->name : "C";

	UNLOCK(__locale_lock);

	return result;
}
