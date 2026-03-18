.text
.global __clone
.type __clone,@function
__clone:
	xor %eax,%eax
	mov $192 | 0x80000000,%eax
	mov %rdi,%r11
	mov %rdx,%rdi
	mov %rsi,%rsi
	mov %r8,%rdx
	mov %r9,%r8
	mov 8(%rsp),%r9
	int $0x80
	test %eax,%eax
	jnz 1f
	xor %ebp,%ebp
	call *%r11
	mov %eax,%edi
	xor %eax,%eax
	int $0x80
	hlt
1:	ret
