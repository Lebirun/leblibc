.text
.global __clone
.type __clone,@function
__clone:
	and $-16,%rsi
	sub $16,%rsi
	mov %rdi,0(%rsi)
	mov %rcx,8(%rsi)
	mov 8(%rsp),%r10
	push %rbx
	mov $192 | 0x80000000,%eax
	mov %rdx,%rbx
	mov %rsi,%rcx
	mov %r8,%rdx
	mov %r9,%rsi
	mov %r10,%rdi
	int $0x80
	test %eax,%eax
	jnz 1f
	xor %ebp,%ebp
	pop %r11
	pop %rdi
	call *%r11
	mov %eax,%edi
	xor %eax,%eax
	int $0x80
	hlt
1:	pop %rbx
	ret
