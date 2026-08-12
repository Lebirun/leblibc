#define _GNU_SOURCE
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

char *getpass(const char *prompt)
{
	int fd;
	struct termios s, t;
	ssize_t l;
	ssize_t received;
	size_t length;
	size_t capacity;
	char *resized;
	static char *password;

	if ((fd = open("/dev/tty", O_RDWR|O_NOCTTY|O_CLOEXEC)) < 0) return 0;

	tcgetattr(fd, &t);
	s = t;
	t.c_lflag &= ~(ECHO|ISIG);
	t.c_lflag |= ICANON;
	t.c_iflag &= ~(INLCR|IGNCR);
	t.c_iflag |= ICRNL;
	tcsetattr(fd, TCSAFLUSH, &t);
	tcdrain(fd);

	dprintf(fd, "%s", prompt);

	free(password);
	password = 0;
	capacity = 64;
	length = 0;
	password = malloc(capacity);
	if (!password) {
		l = -1;
	} else {
		l = 0;
		for (;;) {
			if (capacity - length < 2) {
				if (capacity > SIZE_MAX / 2) {
					l = -1;
					break;
				}
				resized = realloc(password, capacity * 2);
				if (!resized) {
					l = -1;
					break;
				}
				password = resized;
				capacity *= 2;
			}
			received = read(fd, password + length, capacity - length - 1);
			if (received <= 0) {
				if (received < 0) l = -1;
				break;
			}
			length += (size_t)received;
			if (password[length - 1] == '\n') {
				length--;
				break;
			}
		}
		password[length] = 0;
	}

	tcsetattr(fd, TCSAFLUSH, &s);

	dprintf(fd, "\n");
	close(fd);

	if (l < 0) {
		free(password);
		password = 0;
	}
	return password;
}
