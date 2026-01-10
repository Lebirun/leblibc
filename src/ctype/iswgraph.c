#include <wctype.h>

int iswgraph(wint_t wc)
{
	
	return !iswspace(wc) && iswprint(wc);
}

int __iswgraph_l(wint_t c, locale_t l)
{
	return iswgraph(c);
}

weak_alias(__iswgraph_l, iswgraph_l);
