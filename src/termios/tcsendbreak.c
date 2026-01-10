#include <termios.h>
#include <sys/ioctl.h>

int tcsendbreak(int fd, int dur)
{
	
	return ioctl(fd, TCSBRK, 0);
}
