#include <features.h>

hidden void __restore()
{
	__asm__ volatile (
		"mov $0x800000DA, %%eax\n\t"
		"int $0x80\n\t"
		::: "eax"
	);
}

hidden void __restore_rt()
{
	__asm__ volatile (
		"mov $0x800000DA, %%eax\n\t"
		"int $0x80\n\t"
		::: "eax"
	);
}
