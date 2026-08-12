#include <stdarg.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include "lock.h"
#include "fork_impl.h"
#include "locale_impl.h"

static volatile int lock[1];
static char *log_ident;
static int log_opt;
static int log_facility = LOG_USER;
static int log_mask = 0xff;
static int log_fd = -1;
volatile int *const __syslog_lockptr = lock;

int setlogmask(int maskpri)
{
	int ret;

	LOCK(lock);
	ret = log_mask;
	if (maskpri) log_mask = maskpri;
	UNLOCK(lock);
	return ret;
}

static const struct {
	short sun_family;
	char sun_path[9];
} log_addr = {
	AF_UNIX,
	"/dev/log"
};

void closelog(void)
{
	int cs;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	LOCK(lock);
	close(log_fd);
	log_fd = -1;
	UNLOCK(lock);
	pthread_setcancelstate(cs, 0);
}

static void __openlog()
{
	log_fd = socket(AF_UNIX, SOCK_DGRAM|SOCK_CLOEXEC, 0);
	if (log_fd >= 0) connect(log_fd, (void *)&log_addr, sizeof log_addr);
}

void openlog(const char *ident, int opt, int facility)
{
	int cs;
	char *new_ident;

	new_ident = ident ? strdup(ident) : 0;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	LOCK(lock);

	if (!ident || new_ident) {
		free(log_ident);
		log_ident = new_ident;
	}
	log_opt = opt;
	log_facility = facility;

	if ((opt & LOG_NDELAY) && log_fd<0) __openlog();

	UNLOCK(lock);
	pthread_setcancelstate(cs, 0);
}

static int is_lost_conn(int e)
{
	return e==ECONNREFUSED || e==ECONNRESET || e==ENOTCONN || e==EPIPE;
}

static void _vsyslog(int priority, const char *message, va_list ap)
{
	char timebuf[16];
	char inline_buf[1024];
	time_t now;
	struct tm tm;
	char *buf;
	const char *ident;
	va_list aq;
	int errno_save = errno;
	int pid;
	int header_len;
	int message_len;
	int l;
	int hlen;
	int fd;
	size_t needed;
	int allocated;

	if (log_fd < 0) __openlog();

	if (!(priority & LOG_FACMASK)) priority |= log_facility;

	now = time(NULL);
	gmtime_r(&now, &tm);
	strftime_l(timebuf, sizeof timebuf, "%b %e %T", &tm, C_LOCALE);

	pid = (log_opt & LOG_PID) ? getpid() : 0;
	ident = log_ident ? log_ident : "";
	header_len = snprintf(0, 0, "<%d>%s %n%s%s%.0d%s: ",
		priority, timebuf, &hlen, ident, "["+!pid, pid, "]"+!pid);
	if (header_len < 0) return;
	errno = errno_save;
	va_copy(aq, ap);
	message_len = vsnprintf(0, 0, message, aq);
	va_end(aq);
	if (message_len >= 0) {
		if ((size_t)header_len > SIZE_MAX-(size_t)message_len-2) return;
		needed = (size_t)header_len+(size_t)message_len+2;
		allocated = needed > sizeof inline_buf;
		buf = allocated ? malloc(needed) : inline_buf;
		if (!buf) return;
		l = snprintf(buf, needed, "<%d>%s %n%s%s%.0d%s: ",
			priority, timebuf, &hlen, ident, "["+!pid, pid, "]"+!pid);
		errno = errno_save;
		va_copy(aq, ap);
		message_len = vsnprintf(buf+l, needed-(size_t)l, message, aq);
		va_end(aq);
		if (message_len < 0) {
			if (allocated) free(buf);
			return;
		}
		l += message_len;
		if (l == 0 || buf[l-1] != '\n') buf[l++] = '\n';
		if (send(log_fd, buf, l, 0) < 0 && (!is_lost_conn(errno)
		    || connect(log_fd, (void *)&log_addr, sizeof log_addr) < 0
		    || send(log_fd, buf, l, 0) < 0)
		    && (log_opt & LOG_CONS)) {
			fd = open("/dev/console", O_WRONLY|O_NOCTTY|O_CLOEXEC);
			if (fd >= 0) {
				dprintf(fd, "%.*s", l-hlen, buf+hlen);
				close(fd);
			}
		}
		if (log_opt & LOG_PERROR) dprintf(2, "%.*s", l-hlen, buf+hlen);
		if (allocated) free(buf);
	}
}

static void __vsyslog(int priority, const char *message, va_list ap)
{
	int cs;
	if (!(log_mask & LOG_MASK(priority&7)) || (priority&~0x3ff)) return;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	LOCK(lock);
	_vsyslog(priority, message, ap);
	UNLOCK(lock);
	pthread_setcancelstate(cs, 0);
}

void syslog(int priority, const char *message, ...)
{
	va_list ap;
	va_start(ap, message);
	__vsyslog(priority, message, ap);
	va_end(ap);
}

weak_alias(__vsyslog, vsyslog);
