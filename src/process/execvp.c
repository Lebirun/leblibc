#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

extern char **__environ;

int __execvpe(const char *file, char *const argv[], char *const envp[])
{
	const char *p, *z, *path = getenv("PATH");
	size_t k, segment_len, total;
	char *b;
	int seen_eacces = 0;

	errno = ENOENT;
	if (!*file) return -1;

	if (strchr(file, '/'))
		return execve(file, argv, envp);

	if (!path) path = "/usr/local/bin:/bin:/usr/bin";
	k = strlen(file);

	for(p=path; ; p=z) {
		z = __strchrnul(p, ':');
		segment_len = z-p;
		if (k > SIZE_MAX-2 || segment_len > SIZE_MAX-k-2) {
			errno = ENOMEM;
			return -1;
		}
		total = segment_len+k+(segment_len != 0)+1;
		b = malloc(total);
		if (!b) return -1;
		memcpy(b, p, segment_len);
		if (segment_len) b[segment_len] = '/';
		memcpy(b+segment_len+(segment_len != 0), file, k+1);
		execve(b, argv, envp);
		free(b);
		switch (errno) {
		case EACCES:
			seen_eacces = 1;
		case ENOENT:
		case ENOTDIR:
			break;
		default:
			return -1;
		}
		if (!*z++) break;
	}
	if (seen_eacces) errno = EACCES;
	return -1;
}

int execvp(const char *file, char *const argv[])
{
	return __execvpe(file, argv, __environ);
}

weak_alias(__execvpe, execvpe);
