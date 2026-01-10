#define _GNU_SOURCE
#include <sys/socket.h>
#include <limits.h>
#include <errno.h>
#include "syscall.h"

int sendmmsg(int fd, struct mmsghdr *msgvec, unsigned int vlen, unsigned int flags)
{
#if LONG_MAX > INT_MAX
	
	int i;
	if (vlen > IOV_MAX) vlen = IOV_MAX; 
	if (!vlen) return 0;
	for (i=0; i<vlen; i++) {
		
		ssize_t r = sendmsg(fd, &msgvec[i].msg_hdr, flags);
		if (r < 0) goto error;
		msgvec[i].msg_len = r;
	}
error:
	return i ? i : -1;
#else
	return syscall_cp(SYS_sendmmsg, fd, msgvec, vlen, flags);
#endif
}
