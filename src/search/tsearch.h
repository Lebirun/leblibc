#include <search.h>
#include <features.h>

#define MAXH (sizeof(void*)*8*3/2)

struct node {
	const void *key;
	void *a[2];
	int h;
};

hidden int __tsearch_balance(void **);
