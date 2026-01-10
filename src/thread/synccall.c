#include "pthread_impl.h"
#include <semaphore.h>
#include <string.h>

static void dummy_0(void)
{
}

weak_alias(dummy_0, __tl_lock);
weak_alias(dummy_0, __tl_unlock);

static int target_tid;
static void (*callback)(void *), *context;
static sem_t target_sem, caller_sem, exit_sem;

static void dummy(void *p)
{
}

static void handler(int sig)
{
	if (__pthread_self()->tid != target_tid) return;

	int old_errno = errno;

	
	sem_post(&caller_sem);
	sem_wait(&target_sem);

	callback(context);

	
	sem_post(&caller_sem);
	sem_wait(&exit_sem);

	
	sem_post(&caller_sem);

	errno = old_errno;
}

void __synccall(void (*func)(void *), void *ctx)
{
	sigset_t oldmask;
	int cs, i, r;
	struct sigaction sa = { .sa_flags = SA_RESTART | SA_ONSTACK, .sa_handler = handler };
	pthread_t self = __pthread_self(), td;
	int count = 0;

	
	__block_app_sigs(&oldmask);
	__tl_lock();
	__block_all_sigs(0);
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);

	sem_init(&target_sem, 0, 0);
	sem_init(&caller_sem, 0, 0);
	sem_init(&exit_sem, 0, 0);

	if (!libc.threads_minus_1 || __syscall(SYS_gettid) != self->tid)
		goto single_threaded;

	callback = func;
	context = ctx;

	
	memset(&sa.sa_mask, -1, sizeof sa.sa_mask);
	__libc_sigaction(SIGSYNCCALL, &sa, 0);

	for (td=self->next; td!=self; td=td->next) {
		target_tid = td->tid;
		while ((r = -__syscall(SYS_tkill, td->tid, SIGSYNCCALL)) == EAGAIN);
		if (r) {
			
			callback = func = dummy;
			break;
		}
		sem_wait(&caller_sem);
		count++;
	}
	target_tid = 0;

	
	for (i=0; i<count; i++) {
		sem_post(&target_sem);
		sem_wait(&caller_sem);
	}

	sa.sa_handler = SIG_IGN;
	__libc_sigaction(SIGSYNCCALL, &sa, 0);

single_threaded:
	func(ctx);

	
	for (i=0; i<count; i++)
		sem_post(&exit_sem);
	for (i=0; i<count; i++)
		sem_wait(&caller_sem);

	sem_destroy(&caller_sem);
	sem_destroy(&target_sem);
	sem_destroy(&exit_sem);

	pthread_setcancelstate(cs, 0);
	__tl_unlock();
	__restore_sigs(&oldmask);
}
