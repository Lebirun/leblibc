#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

static size_t slash_len(const char *s)
{
	const char *s0 = s;
	while (*s == '/') s++;
	return s-s0;
}

char *realpath(const char *restrict filename, char *restrict resolved)
{
	char stack[PATH_MAX+1];
	char output[PATH_MAX];
	size_t p, q, l, l0, cnt=0, nup=0;
	int check_dir=0;

	if (!filename) {
		errno = EINVAL;
		return 0;
	}
	l = strnlen(filename, sizeof stack);
	if (!l) {
		errno = ENOENT;
		return 0;
	}
	if (l >= PATH_MAX) goto toolong;
	p = sizeof stack - l - 1;
	q = 0;
	memcpy(stack+p, filename, l+1);

	
restart:
	for (; ; p+=slash_len(stack+p)) {
		
		if (stack[p] == '/') {
			check_dir=0;
			nup=0;
			q=0;
			output[q++] = '/';
			p++;
			
			if (stack[p] == '/' && stack[p+1] != '/')
				output[q++] = '/';
			continue;
		}

		char *z = __strchrnul(stack+p, '/');
		l0 = l = z-(stack+p);

		if (!l && !check_dir) break;

		
		if (l==1 && stack[p]=='.') {
			p += l;
			continue;
		}

		
		if (q && output[q-1] != '/') {
			if (!p) goto toolong;
			stack[--p] = '/';
			l++;
		}
		if (q+l >= PATH_MAX) goto toolong;
		memcpy(output+q, stack+p, l);
		output[q+l] = 0;
		p += l;

		int up = 0;
		if (l0==2 && stack[p-2]=='.' && stack[p-1]=='.') {
			up = 1;
			
			if (q <= 3*nup) {
				nup++;
				q += l;
				continue;
			}
			
			if (!check_dir) goto skip_readlink;
		}
		ssize_t k = readlink(output, stack, p);
		if (k==p) goto toolong;
		if (!k) {
			errno = ENOENT;
			return 0;
		}
		if (k<0) {
			if (errno != EINVAL) return 0;
skip_readlink:
			check_dir = 0;
			if (up) {
				while(q && output[q-1]!='/') q--;
				if (q>1 && (q>2 || output[0]!='/')) q--;
				continue;
			}
			if (l0) q += l;
			check_dir = stack[p];
			continue;
		}
		if (++cnt == SYMLOOP_MAX) {
			errno = ELOOP;
			return 0;
		}

		
		if (stack[k-1]=='/') while (stack[p]=='/') p++;
		p -= k;
		memmove(stack+p, stack, k);

		
		goto restart;
	}

 	output[q] = 0;

	if (output[0] != '/') {
		if (!getcwd(stack, sizeof stack)) return 0;
		l = strlen(stack);
		
		p = 0;
		while (nup--) {
			while(l>1 && stack[l-1]!='/') l--;
			if (l>1) l--;
			p += 2;
			if (p<q) p++;
		}
		if (q-p && stack[l-1]!='/') stack[l++] = '/';
		if (l + (q-p) + 1 >= PATH_MAX) goto toolong;
		memmove(output + l, output + p, q - p + 1);
		memcpy(output, stack, l);
		q = l + q-p;
	}

	if (resolved) return memcpy(resolved, output, q+1);
	else return strdup(output);

toolong:
	errno = ENAMETOOLONG;
	return 0;
}
