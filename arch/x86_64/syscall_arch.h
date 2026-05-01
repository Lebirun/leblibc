#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

static inline long __syscall0(long n)
{
	unsigned long __ret;
	__asm__ __volatile__ ("int $0x80" : "=a"(__ret) : "a"(n) : "memory");
	return __ret;
}

static inline long __syscall1(long n, long a1)
{
	unsigned long __ret;
	__asm__ __volatile__ (
		"pushq %%rbx\n\t"
		"movq %2, %%rbx\n\t"
		"int $0x80\n\t"
		"popq %%rbx"
		: "=a"(__ret) : "a"(n), "r"(a1) : "memory");
	return __ret;
}

static inline long __syscall2(long n, long a1, long a2)
{
	unsigned long __ret;
	__asm__ __volatile__ (
		"pushq %%rbx\n\t"
		"movq %2, %%rbx\n\t"
		"int $0x80\n\t"
		"popq %%rbx"
		: "=a"(__ret) : "a"(n), "r"(a1), "c"(a2) : "memory");
	return __ret;
}

static inline long __syscall3(long n, long a1, long a2, long a3)
{
	unsigned long __ret;
	__asm__ __volatile__ (
		"pushq %%rbx\n\t"
		"movq %2, %%rbx\n\t"
		"int $0x80\n\t"
		"popq %%rbx"
		: "=a"(__ret) : "a"(n), "r"(a1), "c"(a2), "d"(a3) : "memory");
	return __ret;
}

static inline long __syscall4(long n, long a1, long a2, long a3, long a4)
{
	unsigned long __ret;
	__asm__ __volatile__ (
		"pushq %%rbx\n\t"
		"movq %2, %%rbx\n\t"
		"int $0x80\n\t"
		"popq %%rbx"
		: "=a"(__ret) : "a"(n), "r"(a1), "c"(a2), "d"(a3), "S"(a4) : "memory");
	return __ret;
}

static inline long __syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
	unsigned long __ret;
	__asm__ __volatile__ (
		"pushq %%rbx\n\t"
		"movq %2, %%rbx\n\t"
		"int $0x80\n\t"
		"popq %%rbx"
		: "=a"(__ret) : "a"(n), "r"(a1), "c"(a2), "d"(a3), "S"(a4), "D"(a5) : "memory");
	return __ret;
}

static inline long __syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
	unsigned long __ret;
	__asm__ __volatile__ (
		"pushq %%rbx\n\t"
		"pushq %%rbp\n\t"
		"movq %2, %%rbx\n\t"
		"movq %7, %%rbp\n\t"
		"int $0x80\n\t"
		"popq %%rbp\n\t"
		"popq %%rbx"
		: "=a"(__ret)
		: "a"(n), "r"(a1), "c"(a2), "d"(a3), "S"(a4), "D"(a5), "r"(a6)
		: "memory");
	return __ret;
}
