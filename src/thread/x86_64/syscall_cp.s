.text
.global __syscall_cp_asm
.hidden __syscall_cp_asm
.type __syscall_cp_asm,@function
__syscall_cp_asm:
	mov (%rdi),%eax
	test %eax,%eax
	jnz __cp_cancel
	mov %rsi,%rax
	mov %rdx,%rbx
	mov %rcx,%rcx
	mov %r8,%rdx
	mov %r9,%rsi
	mov 8(%rsp),%rdi
	mov 16(%rsp),%rbp
.global __cp_begin
.hidden __cp_begin
__cp_begin:
	int $0x80
.global __cp_end
.hidden __cp_end
__cp_end:
	ret
.global __cp_cancel
.hidden __cp_cancel
__cp_cancel:
	jmp __cancel
