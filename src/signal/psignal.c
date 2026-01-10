#include "stdio_impl.h"
#include <string.h>
#include <signal.h>
#include <errno.h>

void psignal(int sig, const char *msg)
{
	FILE *f = stderr;
	char *s = strsignal(sig);

	FLOCK(f);

	
	void *old_locale = f->locale;
	int old_mode = f->mode;
	int old_errno = errno;

	if (fprintf(f, "%s%s%s\n", msg?msg:"", msg?": ":"", s)>=0)
		errno = old_errno;
	f->mode = old_mode;
	f->locale = old_locale;

	FUNLOCK(f);
}
