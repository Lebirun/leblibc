#include <features.h>

struct k_sigaction {
	void (*handler)(int);
	unsigned long flags;
#ifdef SA_RESTORER
	void (*restorer)(void);
#endif
	unsigned mask[2];
#ifndef SA_RESTORER
	void *unused;
#endif
};

hidden void __restore(), __restore_rt();
