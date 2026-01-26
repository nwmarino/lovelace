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
    movq    $60, %rax   # exit syscall
    syscall
    ud2                 # unreachable

    .text
    .type   __rt_init, @function
__rt_init:
#   call    __fmt_arena_init@PLT
    ret

    .text
    .type   __rt_shutdown, @function
__rt_shutdown:
#   call    __fmt_arena_destroy@PLT
    ret

# __copy :: (*void, *void, s64) -> void
    .text
    .global __copy
    .type   __copy, @function
__copy:
.__copy_cnd:
    cmpq    $0, %rdx
    jne     .__copy_bdy
    ret
.__copy_bdy:
    movb    (%rsi), %al
    movb    %al, (%rdi)
    incq    %rdi
    incq    %rsi
    decq    %rdx
    jmp     .__copy_cnd

# __set :: (*void, s8, s64) -> void
    .text
    .global __set
    .type   __set, @function
__set:
.__set_cnd:
    cmpq    $0, %rdx
    jne     .__set_bdy
    ret
.__set_bdy:
    movb    %sil, (%rdi)
    decq    %rdx
    jmp     .__set_cnd

# __zero :: (*void, s64) -> void
    .text
    .global __zero
    .type   __zero, @function
__zero:
.__zero_cnd:
    cmpq    $0, %rdx
    jne     .__zero_bdy
    ret
.__zero_bdy:
    movb    $0, (%rdi)
    decq    %rdx
    jmp     .__zero_cnd

# __abort :: () -> void
    .text
    .global __abort
    .type   __abort, @function
__abort:
    movq    $62, %rax
    movq    $0, %rdi    # pid 0
    movq    $0, %rsi    # signal 6 (SIGABRT)
    syscall             # kill syscall
    call    __unreachable 

# __unreachable :: () -> void
    .text
    .global __unreachable
    .type   __unreachable, @function
__unreachable:
    ud2

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
