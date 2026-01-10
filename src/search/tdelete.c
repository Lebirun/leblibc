#include <stdlib.h>
#include <search.h>
#include "tsearch.h"

void *tdelete(const void *restrict key, void **restrict rootp,
	int(*cmp)(const void *, const void *))
{
	if (!rootp)
		return 0;

	void **a[MAXH+1];
	struct node *n = *rootp;
	struct node *parent;
	struct node *child;
	int i=0;
	
	a[i++] = rootp;
	a[i++] = rootp;
	for (;;) {
		if (!n)
			return 0;
		int c = cmp(key, n->key);
		if (!c)
			break;
		a[i++] = &n->a[c>0];
		n = n->a[c>0];
	}
	parent = *a[i-2];
	if (n->a[0]) {
		
		struct node *deleted = n;
		a[i++] = &n->a[0];
		n = n->a[0];
		while (n->a[1]) {
			a[i++] = &n->a[1];
			n = n->a[1];
		}
		deleted->key = n->key;
		child = n->a[0];
	} else {
		child = n->a[1];
	}
	
	free(n);
	*a[--i] = child;
	while (--i && __tsearch_balance(a[i]));
	return parent;
}
