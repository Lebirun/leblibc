#include "pthread_impl.h"

struct waiter {
	struct waiter *prev, *next;
	volatile int state, barrier;
	volatile int *notify;
};

static inline void lock(volatile int *l)
{
	if (a_cas(l, 0, 1)) {
		a_cas(l, 1, 2);
		do __wait(l, 0, 2, 1);
		while (a_cas(l, 0, 2));
	}
}

static inline void unlock(volatile int *l)
{
	if (a_swap(l, 0)==2)
		__wake(l, 1, 1);
}

static inline void unlock_requeue(volatile int *l, volatile int *r, int w)
{
	a_store(l, 0);
	if (w) __wake(l, 1, 1);
	else __syscall(SYS_futex, l, FUTEX_REQUEUE|FUTEX_PRIVATE, 0, 1, r) != -ENOSYS
		|| __syscall(SYS_futex, l, FUTEX_REQUEUE, 0, 1, r);
}

enum {
	WAITING,
	SIGNALED,
	LEAVING,
};

int __pthread_cond_timedwait(pthread_cond_t *restrict c, pthread_mutex_t *restrict m, const struct timespec *restrict ts)
{
	struct waiter node = { 0 };
	int e, seq, clock = c->_c_clock, cs, shared=0, oldstate, tmp;
	volatile int *fut;

	if ((m->_m_type&15) && (m->_m_lock&INT_MAX) != __pthread_self()->tid)
		return EPERM;

	if (ts && ts->tv_nsec >= 1000000000UL)
		return EINVAL;

	__pthread_testcancel();

	if (c->_c_shared) {
		shared = 1;
		fut = &c->_c_seq;
		seq = c->_c_seq;
		a_inc(&c->_c_waiters);
	} else {
		lock(&c->_c_lock);

		seq = node.barrier = 2;
		fut = &node.barrier;
		node.state = WAITING;
		node.next = c->_c_head;
		c->_c_head = &node;
		if (!c->_c_tail) c->_c_tail = &node;
		else node.next->prev = &node;

		unlock(&c->_c_lock);
	}

	__pthread_mutex_unlock(m);

	__pthread_setcancelstate(PTHREAD_CANCEL_MASKED, &cs);
	if (cs == PTHREAD_CANCEL_DISABLE) __pthread_setcancelstate(cs, 0);

	do e = __timedwait_cp(fut, seq, clock, ts, !shared);
	while (*fut==seq && (!e || e==EINTR));
	if (e == EINTR) e = 0;

	if (shared) {
		
		if (e == ECANCELED && c->_c_seq != seq) e = 0;
		if (a_fetch_add(&c->_c_waiters, -1) == -0x7fffffff)
			__wake(&c->_c_waiters, 1, 0);
		oldstate = WAITING;
		goto relock;
	}

	oldstate = a_cas(&node.state, WAITING, LEAVING);

	if (oldstate == WAITING) {
		

		lock(&c->_c_lock);
		
		if (c->_c_head == &node) c->_c_head = node.next;
		else if (node.prev) node.prev->next = node.next;
		if (c->_c_tail == &node) c->_c_tail = node.prev;
		else if (node.next) node.next->prev = node.prev;
		
		unlock(&c->_c_lock);

		if (node.notify) {
			if (a_fetch_add(node.notify, -1)==1)
				__wake(node.notify, 1, 1);
		}
	} else {
		
		lock(&node.barrier);
	}

relock:
	
	if ((tmp = pthread_mutex_lock(m))) e = tmp;

	if (oldstate == WAITING) goto done;

	if (!node.next && !(m->_m_type & 8))
		a_inc(&m->_m_waiters);

	
	if (node.prev) {
		int val = m->_m_lock;
		if (val>0) a_cas(&m->_m_lock, val, val|0x80000000);
		unlock_requeue(&node.prev->barrier, &m->_m_lock, m->_m_type & (8|128));
	} else if (!(m->_m_type & 8)) {
		a_dec(&m->_m_waiters);		
	}

	
	if (e == ECANCELED) e = 0;

done:
	__pthread_setcancelstate(cs, 0);

	if (e == ECANCELED) {
		__pthread_testcancel();
		__pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
	}

	return e;
}

int __private_cond_signal(pthread_cond_t *c, int n)
{
	struct waiter *p, *first=0;
	volatile int ref = 0;
	int cur;

	lock(&c->_c_lock);
	for (p=c->_c_tail; n && p; p=p->prev) {
		if (a_cas(&p->state, WAITING, SIGNALED) != WAITING) {
			ref++;
			p->notify = &ref;
		} else {
			n--;
			if (!first) first=p;
		}
	}
	
	if (p) {
		if (p->next) p->next->prev = 0;
		p->next = 0;
	} else {
		c->_c_head = 0;
	}
	c->_c_tail = p;
	unlock(&c->_c_lock);

	
	while ((cur = ref)) __wait(&ref, 0, cur, 1);

	
	if (first) unlock(&first->barrier);

	return 0;
}

weak_alias(__pthread_cond_timedwait, pthread_cond_timedwait);
