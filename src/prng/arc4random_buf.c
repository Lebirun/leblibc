#define _BSD_SOURCE
#include <stdlib.h>
#include <sys/random.h>
#include <errno.h>

void arc4random_buf(void *buffer, size_t length)
{
	unsigned char *position;
	ssize_t result;

	position = buffer;
	while (length > 0) {
		result = getrandom(position, length, 0);
		if (result < 0 && errno == EINTR)
			continue;
		if (result <= 0)
			abort();
		position += result;
		length -= (size_t)result;
	}
}
