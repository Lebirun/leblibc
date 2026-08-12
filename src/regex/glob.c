#define _BSD_SOURCE
#include <glob.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <pwd.h>
#include <stdint.h>

struct match
{
	struct match *next;
	char name[];
};

static int reserve_path(char **buf, size_t *capacity, size_t need)
{
	char *grown;
	size_t next;

	if (need <= *capacity) return 0;
	next = *capacity ? *capacity : 64;
	while (next < need) {
		if (next > SIZE_MAX/2) {
			next = need;
			break;
		}
		next *= 2;
	}
	grown = realloc(*buf, next);
	if (!grown) return -1;
	*buf = grown;
	*capacity = next;
	return 0;
}

static int append(struct match **tail, const char *name, size_t len, int mark)
{
	struct match *new = malloc(sizeof(struct match) + len + 2);
	if (!new) return -1;
	(*tail)->next = new;
	new->next = NULL;
	memcpy(new->name, name, len+1);
	if (mark && len && name[len-1]!='/') {
		new->name[len] = '/';
		new->name[len+1] = 0;
	}
	*tail = new;
	return 0;
}

static int do_glob(char **bufp, size_t *capacity, size_t pos, int type,
	char *pat, int flags, int (*errfunc)(const char *path, int err),
	struct match **tail)
{
	char *buf;
	ptrdiff_t i, j;
	int in_bracket;
	char *p2;
	char saved_sep;
	DIR *dir;
	int old_errno;
	struct dirent *de;
	size_t l;
	int fnm_flags;
	int r;
	int readerr;
	struct stat st;
	char *p;

	buf = *bufp;
	
	if (!type && !(flags & GLOB_MARK)) type = DT_REG;

	
	if (*pat && type!=DT_DIR) type = 0;
	while (*pat=='/') {
		if (reserve_path(bufp, capacity, pos+2)) return GLOB_NOSPACE;
		buf = *bufp;
		buf[pos++] = *pat++;
	}

	
	i = 0;
	j = 0;
	in_bracket = 0;
	for (; pat[i]!='*' && pat[i]!='?' && (!in_bracket || pat[i]!=']'); i++) {
		if (!pat[i]) {
			pat += i;
			pos += j;
			i = j = 0;
			break;
		} else if (pat[i] == '[') {
			in_bracket = 1;
		} else if (pat[i] == '\\' && !(flags & GLOB_NOESCAPE)) {
			
			if (in_bracket && pat[i+1]==']') break;
			
			if (!pat[i+1]) return 0;
			i++;
		}
		if (pat[i] == '/') {
			in_bracket = 0;
			pat += i+1;
			i = -1;
			pos += j+1;
			j = -1;
		}
		
		if ((size_t)j > SIZE_MAX-pos-2) return GLOB_NOSPACE;
		if (reserve_path(bufp, capacity, pos+(size_t)j+2))
			return GLOB_NOSPACE;
		buf = *bufp;
		buf[pos+j++] = pat[i];
		
		type = 0;
	}
	buf[pos] = 0;
	if (!*pat) {
		
		if ((flags & GLOB_MARK) && (!type||type==DT_LNK) && !stat(buf, &st)) {
			if (S_ISDIR(st.st_mode)) type = DT_DIR;
			else type = DT_REG;
		}
		if (!type && lstat(buf, &st)) {
			if (errno!=ENOENT && (errfunc(buf, errno) || (flags & GLOB_ERR)))
				return GLOB_ABORTED;
			return 0;
		}
		if (append(tail, buf, pos, (flags & GLOB_MARK) && type==DT_DIR))
			return GLOB_NOSPACE;
		return 0;
	}
	p2 = strchr(pat, '/');
	saved_sep = '/';
	
	if (p2 && !(flags & GLOB_NOESCAPE)) {
		for (p=p2; p>pat && p[-1]=='\\'; p--);
		if ((p2-p)%2) {
			p2--;
			saved_sep = '\\';
		}
	}
	dir = opendir(pos ? buf : ".");
	if (!dir) {
		if (errfunc(buf, errno) || (flags & GLOB_ERR))
			return GLOB_ABORTED;
		return 0;
	}
	old_errno = errno;
	while (errno=0, de=readdir(dir)) {
		
		if (p2 && de->d_type && de->d_type!=DT_DIR && de->d_type!=DT_LNK)
			continue;

		l = strlen(de->d_name);
		if (l > SIZE_MAX-pos-1 ||
		    reserve_path(bufp, capacity, pos+l+1)) {
			closedir(dir);
			return GLOB_NOSPACE;
		}
		buf = *bufp;

		if (p2) *p2 = 0;

		fnm_flags= ((flags & GLOB_NOESCAPE) ? FNM_NOESCAPE : 0)
			| ((!(flags & GLOB_PERIOD)) ? FNM_PERIOD : 0);

		if (fnmatch(pat, de->d_name, fnm_flags))
			continue;

		
		if (p2 && (flags & GLOB_PERIOD) && de->d_name[0]=='.'
		    && (!de->d_name[1] || de->d_name[1]=='.' && !de->d_name[2])
		    && fnmatch(pat, de->d_name, fnm_flags | FNM_PERIOD))
			continue;

		memcpy(buf+pos, de->d_name, l+1);
		if (p2) *p2 = saved_sep;
		r = do_glob(bufp, capacity, pos+l, de->d_type,
		            p2 ? p2 : "", flags, errfunc, tail);
		if (r) {
			closedir(dir);
			return r;
		}
		buf = *bufp;
	}
	readerr = errno;
	if (p2) *p2 = saved_sep;
	closedir(dir);
	if (readerr && (errfunc(buf, errno) || (flags & GLOB_ERR)))
		return GLOB_ABORTED;
	errno = old_errno;
	return 0;
}

static int ignore_err(const char *path, int err)
{
	return 0;
}

static void freelist(struct match *head)
{
	struct match *match, *next;
	for (match=head->next; match; match=next) {
		next = match->next;
		free(match);
	}
}

static int sort(const void *a, const void *b)
{
	return strcmp(*(const char **)a, *(const char **)b);
}

static int expand_tilde(char **pat, char **bufp, size_t *capacity, size_t *pos)
{
	char *p;
	char *name_end;
	char *home;
	char *scratch;
	char *grown;
	size_t scratch_size;
	size_t home_len;
	char delim;
	struct passwd pw;
	struct passwd *res;
	int result;
	char *buf;

	p = *pat + 1;
	name_end = __strchrnul(p, '/');
	if ((delim = *name_end)) *name_end++ = 0;
	*pat = name_end;

	home = *p ? NULL : getenv("HOME");
	scratch = NULL;
	if (!home) {
		scratch_size = 256;
		scratch = malloc(scratch_size);
		if (!scratch) return GLOB_NOSPACE;
		for (;;) {
			result = *p ? getpwnam_r(p, &pw, scratch, scratch_size, &res)
			            : getpwuid_r(getuid(), &pw, scratch, scratch_size, &res);
			if (result != ERANGE) break;
			if (scratch_size > SIZE_MAX/2) {
				free(scratch);
				return GLOB_NOSPACE;
			}
			scratch_size *= 2;
			grown = realloc(scratch, scratch_size);
			if (!grown) {
				free(scratch);
				return GLOB_NOSPACE;
			}
			scratch = grown;
		}
		if (result || !res) {
			free(scratch);
			return result == ENOMEM ? GLOB_NOSPACE : GLOB_NOMATCH;
		}
		home = pw.pw_dir;
	}
	home_len = strlen(home);
	if (home_len > SIZE_MAX-2 ||
	    reserve_path(bufp, capacity, home_len+2)) {
		if (scratch) free(scratch);
		return GLOB_NOSPACE;
	}
	buf = *bufp;
	memcpy(buf, home, home_len);
	if (scratch) free(scratch);
	if (delim) {
		buf[home_len] = delim;
		buf[home_len+1] = 0;
		*pos = home_len+1;
	} else {
		buf[home_len] = 0;
		*pos = home_len;
	}
	return 0;
}

int glob(const char *restrict pat, int flags, int (*errfunc)(const char *path, int err), glob_t *restrict g)
{
	struct match head = { .next = NULL }, *tail = &head;
	size_t cnt, i;
	size_t offs = (flags & GLOB_DOOFFS) ? g->gl_offs : 0;
	int error = 0;
	char *buf;
	size_t capacity;
	char *p;
	char *s;
	size_t pos;
	char **pathv;
	size_t pointer_count;
	size_t pointer_base;

	capacity = 64;
	buf = malloc(capacity);
	if (!buf) return GLOB_NOSPACE;
	buf[0] = 0;
	
	if (!errfunc) errfunc = ignore_err;

	if (!(flags & GLOB_APPEND)) {
		g->gl_offs = offs;
		g->gl_pathc = 0;
		g->gl_pathv = NULL;
	}

	if (*pat) {
		p = strdup(pat);
		if (!p) {
			free(buf);
			return GLOB_NOSPACE;
		}
		buf[0] = 0;
		pos = 0;
		s = p;
		if ((flags & (GLOB_TILDE | GLOB_TILDE_CHECK)) && *p == '~')
			error = expand_tilde(&s, &buf, &capacity, &pos);
		if (!error)
			error = do_glob(&buf, &capacity, pos, 0, s, flags,
			                errfunc, &tail);
		free(p);
	}

	if (error == GLOB_NOSPACE) {
		freelist(&head);
		free(buf);
		return error;
	}
	
	for (cnt=0, tail=head.next; tail; tail=tail->next, cnt++);
	if (!cnt) {
		if (flags & GLOB_NOCHECK) {
			tail = &head;
			if (append(&tail, pat, strlen(pat), 0)) {
				free(buf);
				return GLOB_NOSPACE;
			}
			cnt++;
		} else if (!error) {
			free(buf);
			return GLOB_NOMATCH;
		}
	}

	if (offs > SIZE_MAX-g->gl_pathc) {
		freelist(&head);
		free(buf);
		return GLOB_NOSPACE;
	}
	pointer_base = offs+g->gl_pathc;
	if (cnt == SIZE_MAX || pointer_base > SIZE_MAX-cnt-1) {
		freelist(&head);
		free(buf);
		return GLOB_NOSPACE;
	}
	pointer_count = pointer_base+cnt+1;
	if (pointer_count > SIZE_MAX/sizeof(char *)) {
		freelist(&head);
		free(buf);
		return GLOB_NOSPACE;
	}
	if (flags & GLOB_APPEND) {
		pathv = realloc(g->gl_pathv, pointer_count * sizeof(char *));
		if (!pathv) {
			freelist(&head);
			free(buf);
			return GLOB_NOSPACE;
		}
		g->gl_pathv = pathv;
		offs += g->gl_pathc;
	} else {
		g->gl_pathv = malloc(pointer_count * sizeof(char *));
		if (!g->gl_pathv) {
			freelist(&head);
			free(buf);
			return GLOB_NOSPACE;
		}
		for (i=0; i<offs; i++)
			g->gl_pathv[i] = NULL;
	}
	for (i=0, tail=head.next; i<cnt; tail=tail->next, i++)
		g->gl_pathv[offs + i] = tail->name;
	g->gl_pathv[offs + i] = NULL;
	g->gl_pathc += cnt;

	if (!(flags & GLOB_NOSORT))
		qsort(g->gl_pathv+offs, cnt, sizeof(char *), sort);
	
	free(buf);
	return error;
}

void globfree(glob_t *g)
{
	size_t i;
	for (i=0; i<g->gl_pathc; i++)
		free(g->gl_pathv[g->gl_offs + i] - offsetof(struct match, name));
	free(g->gl_pathv);
	g->gl_pathc = 0;
	g->gl_pathv = NULL;
}
