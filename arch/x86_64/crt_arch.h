__asm__(
".text\n"
".weak _DYNAMIC \n"
".hidden _DYNAMIC \n"
".global " START "\n"
START ":\n"
"	xor %rbp,%rbp \n"
"	mov %rsp,%rdi \n"
"	lea _DYNAMIC(%rip),%rsi \n"
"	andq $-16,%rsp \n"
"	call " START "_c \n"
);
