struct __dirstream
{
	off_t tell;
	int fd;
	int buf_pos;
	int buf_end;
	volatile int lock[1];
	size_t buf_capacity;
	char *buf;
};
