#include "pthread_impl.h"
#include "fork_impl.h"
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#define PTHREAD_TSD_INLINE_KEYS 8

volatile size_t __pthread_tsd_size =
	sizeof(void *) * PTHREAD_TSD_INLINE_KEYS;
void *__pthread_tsd_main[PTHREAD_TSD_INLINE_KEYS] = { 0 };

struct extended_tsd {
	struct extended_tsd *next;
	pthread_t thread;
	pthread_key_t key;
	void *value;
};

static void (**keys)(void *);
static size_t keys_capacity;
static struct extended_tsd *extended_values;

static pthread_rwlock_t key_lock = PTHREAD_RWLOCK_INITIALIZER;

static pthread_key_t next_key;

static struct extended_tsd *find_extended(pthread_t thread,
		pthread_key_t key, struct extended_tsd **previous)
{
	struct extended_tsd *entry;
	struct extended_tsd *prev;

	prev = 0;
	for (entry = extended_values; entry; entry = entry->next) {
		if (entry->thread == thread && entry->key == key) break;
		prev = entry;
	}
	if (previous) *previous = prev;
	return entry;
}

static void *tsd_get_locked(pthread_t thread, pthread_key_t key)
{
	struct extended_tsd *entry;

	if (key < PTHREAD_TSD_INLINE_KEYS) return thread->tsd[key];
	entry = find_extended(thread, key, 0);
	return entry ? entry->value : 0;
}

static int tsd_set_locked(pthread_t thread, pthread_key_t key, void *value)
{
	struct extended_tsd *entry;
	struct extended_tsd *previous;

	if (key < PTHREAD_TSD_INLINE_KEYS) {
		thread->tsd[key] = value;
		return 0;
	}
	entry = find_extended(thread, key, &previous);
	if (entry) {
		if (value) {
			entry->value = value;
		} else {
			if (previous) previous->next = entry->next;
			else extended_values = entry->next;
			free(entry);
		}
		return 0;
	}
	if (!value) return 0;
	entry = malloc(sizeof(*entry));
	if (!entry) return ENOMEM;
	entry->thread = thread;
	entry->key = key;
	entry->value = value;
	entry->next = extended_values;
	extended_values = entry;
	return 0;
}

void *__pthread_tsd_get(pthread_t thread, pthread_key_t key)
{
	void *value;

	__pthread_rwlock_rdlock(&key_lock);
	value = key < keys_capacity && keys[key] ?
		tsd_get_locked(thread, key) : 0;
	__pthread_rwlock_unlock(&key_lock);
	return value;
}

int __pthread_tsd_set(pthread_t thread, pthread_key_t key, void *value)
{
	int result;

	__pthread_rwlock_wrlock(&key_lock);
	if (key >= keys_capacity || !keys[key]) {
		__pthread_rwlock_unlock(&key_lock);
		return EINVAL;
	}
	result = tsd_set_locked(thread, key, value);
	if (result == 0 && value) thread->tsd_used = 1;
	__pthread_rwlock_unlock(&key_lock);
	return result;
}

static void nodtor(void *dummy)
{
}

static void dummy_0(void)
{
}

weak_alias(dummy_0, __tl_lock);
weak_alias(dummy_0, __tl_unlock);

void __pthread_key_atfork(int who)
{
	struct extended_tsd *entry;
	struct extended_tsd *next;
	struct extended_tsd *previous;
	pthread_t self;

	if (who<0) {
		__pthread_rwlock_rdlock(&key_lock);
	} else if (!who) {
		__pthread_rwlock_unlock(&key_lock);
	} else {
		key_lock = (pthread_rwlock_t)PTHREAD_RWLOCK_INITIALIZER;
		self = __pthread_self();
		previous = 0;
		entry = extended_values;
		while (entry) {
			next = entry->next;
			if (entry->thread != self) {
				if (previous) previous->next = next;
				else extended_values = next;
				free(entry);
			} else {
				previous = entry;
			}
			entry = next;
		}
	}
}

int __pthread_key_create(pthread_key_t *k, void (*dtor)(void *))
{
	pthread_t self;
	void (**resized)(void *);
	size_t capacity;
	size_t start;
	size_t index;

	self = __pthread_self();
	if (!self->tsd) self->tsd = __pthread_tsd_main;
	if (!dtor) dtor = nodtor;

	__pthread_rwlock_wrlock(&key_lock);
	if (!keys_capacity) {
		keys_capacity = 1;
		keys = calloc(keys_capacity, sizeof(*keys));
		if (!keys) {
			__pthread_rwlock_unlock(&key_lock);
			return EAGAIN;
		}
	}
	start = next_key < keys_capacity ? next_key : 0;
	for (index = start; index < keys_capacity; index++) {
		if (!keys[index]) break;
	}
	if (index == keys_capacity) {
		for (index = 0; index < start; index++) {
			if (!keys[index]) break;
		}
	}
	if (index == start && keys[index]) index = keys_capacity;
	if (index == keys_capacity) {
		if (keys_capacity > UINT32_MAX / 2 ||
		    keys_capacity * 2 > SIZE_MAX / sizeof(*keys)) {
			__pthread_rwlock_unlock(&key_lock);
			return EAGAIN;
		}
		capacity = keys_capacity * 2;
		resized = realloc(keys, capacity * sizeof(*keys));
		if (!resized) {
			__pthread_rwlock_unlock(&key_lock);
			return EAGAIN;
		}
		memset(resized + keys_capacity, 0,
		       (capacity - keys_capacity) * sizeof(*keys));
		index = keys_capacity;
		keys = resized;
		keys_capacity = capacity;
	}
	keys[index] = dtor;
	*k = (pthread_key_t)index;
	next_key = (pthread_key_t)(index + 1 < keys_capacity ? index + 1 : 0);

	__pthread_rwlock_unlock(&key_lock);
	return 0;
}

int __pthread_key_delete(pthread_key_t k)
{
	sigset_t set;
	pthread_t self;
	pthread_t td;
	struct extended_tsd *entry;
	struct extended_tsd *next;
	struct extended_tsd *previous;
	void (**released)(void *);
	size_t index;
	int active;

	self = __pthread_self();
	td = self;
	released = 0;

	__block_app_sigs(&set);
	__pthread_rwlock_wrlock(&key_lock);

	if (k >= keys_capacity || !keys[k]) {
		__pthread_rwlock_unlock(&key_lock);
		__restore_sigs(&set);
		return EINVAL;
	}
	if (k < PTHREAD_TSD_INLINE_KEYS) {
		__tl_lock();
		do td->tsd[k] = 0;
		while ((td=td->next)!=self);
		__tl_unlock();
	} else {
		previous = 0;
		entry = extended_values;
		while (entry) {
			next = entry->next;
			if (entry->key == k) {
				if (previous) previous->next = next;
				else extended_values = next;
				free(entry);
			} else {
				previous = entry;
			}
			entry = next;
		}
	}
	keys[k] = 0;
	active = 0;
	for (index = 0; index < keys_capacity; index++) {
		if (keys[index]) {
			active = 1;
			break;
		}
	}
	if (!active) {
		released = keys;
		keys = 0;
		keys_capacity = 0;
		next_key = 0;
	}

	__pthread_rwlock_unlock(&key_lock);
	__restore_sigs(&set);
	free(released);

	return 0;
}

void __pthread_tsd_run_dtors()
{
	pthread_t self;
	size_t i;
	int j;
	void *val;
	void (*dtor)(void *);

	self = __pthread_self();

	for (j=0; self->tsd_used && j<PTHREAD_DESTRUCTOR_ITERATIONS; j++) {
		self->tsd_used = 0;
		i = 0;
		for (;;) {
			__pthread_rwlock_wrlock(&key_lock);
			if (i >= keys_capacity) {
				__pthread_rwlock_unlock(&key_lock);
				break;
			}
			val = keys[i] ? tsd_get_locked(self, (pthread_key_t)i) : 0;
			dtor = keys[i];
			tsd_set_locked(self, (pthread_key_t)i, 0);
			__pthread_rwlock_unlock(&key_lock);
			if (val && dtor && dtor != nodtor) {
				dtor(val);
			}
			i++;
		}
	}
}

weak_alias(__pthread_key_create, pthread_key_create);
weak_alias(__pthread_key_delete, pthread_key_delete);
