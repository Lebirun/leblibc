.text
.global __syscall_cp_asm
.hidden __syscall_cp_asm
.type __syscall_cp_asm,@function
__syscall_cp_asm:
	push %rbx
	push %rbp
.global __cp_begin
.hidden __cp_begin
__cp_begin:
	mov (%rdi),%eax
	test %eax,%eax
	jnz __cp_cancel
	mov %rsi,%rax
	mov %rdx,%rbx
	mov %r8,%rdx
	mov %r9,%rsi
	mov 24(%rsp),%rdi
	mov 32(%rsp),%rbp
	int $0x80
.global __cp_end
.hidden __cp_end
__cp_end:
	pop %rbp
	pop %rbx
	ret
.global __cp_cancel
.hidden __cp_cancel
__cp_cancel:
	pop %rbp
	pop %rbx
	jmp __cancel
