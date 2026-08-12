#include <libintl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

static char *current_domain;

char *__gettextdomain()
{
	return current_domain ? current_domain : "messages";
}

char *textdomain(const char *domainname)
{
	char *new_domain;
	size_t domlen;

	if (!domainname) return __gettextdomain();

	domlen = strlen(domainname);
	new_domain = malloc(domlen+1);
	if (!new_domain) return 0;
	memcpy(new_domain, domainname, domlen+1);
	free(current_domain);
	current_domain = new_domain;

	return current_domain;
}

char *gettext(const char *msgid)
{
	return dgettext(0, msgid);
}

char *ngettext(const char *msgid1, const char *msgid2, unsigned long int n)
{
	return dngettext(0, msgid1, msgid2, n);
}
