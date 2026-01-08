    .text
    .global _start
    .type   _start, @function
_start:
    call main@PLT
    movq %rax, %rdi
    movq $60, %rax
    syscall
