#   Copyright (c) 2026 Nick Marino
#  
#   All runtime functions defined here assume the Linux x86 SystemV ABI.

    .text
    .global _start
    .type   _start, @function
_start:
    call    __rt_init
    call    main@PLT
    movq    %rax, %rdi
    movq    $60, %rax
    syscall
    ud2

__rt_init:
    call    __fmt_arena_init@PLT
    ret

__rt_shutdown:
    call    __fmt_arena_destroy@PLT
    ret

# __mem_copy :: (*void, *void, s64) -> void
__mem_copy:
.cond:
    cmpq    $0, %rcx
    jne     .body
    ret
.body:
    movb    (%rsi), %al
    movb    %al, (%rdi)
    incq    %rdi
    incq    %rsi
    decq    %rcx
    jmp     .cond

# __mem_set :: (*void, s8, s64) -> void
__mem_set:
.cond:
    cmpq    $0, %rcx
    jne     .body
    ret
.body:
    movb    %sil, (%rdi)
    decq    %rcx
    jmp     .cond

# exit :: (s64) -> void
    .text
    .global exit
    .type   exit, @function
exit:
    movq    $60, %rax
    syscall

# open :: (*char, s64, s64) -> s64
    .text
    .global open
    .type   open, @function
open: 
    movq    $2, %rax
    syscall
    ret

# close :: (s64) -> s64
    .text
    .global close
    .type   close, @function
close:
    movq    $3, %rax
    syscall
    ret

# read :: (s64, *char, s64) -> s64
    .text
    .global read
    .type   read, @function
read:
    movq    $0, %rax
    syscall
    ret

# write :: (s64, *mut char, s64) -> s64
    .text
    .global write
    .type   write, @function
write:
    movq    $1, %rax
    syscall
    ret

# brk :: (u64) -> *void
    .text
    .global brk
    .type   brk, @function
brk:
    movq    $12, %rax
    syscall
    ret
