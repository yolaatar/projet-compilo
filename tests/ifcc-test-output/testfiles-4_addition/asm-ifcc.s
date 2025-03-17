.section __TEXT,__text,regular,pure_instructions
.globl _main
_main:
    sub sp, sp, #16
    str wzr, [sp, #12]
    mov w0, #4
    mov w8, w0
    mov w0, w8
    mov w8, w0
    mov w0, #3
    mov w8, w0
    mov w0, w8
    add w8, w8, w0
    mov w0, w8
    str w0, [sp, #8]
    mov w0, #5
    mov w8, w0
    mov w0, w8
    mov w8, w0
    mov w0, w8
    str w0, [sp, #12]
    ldr w0, [sp, #8]
    mov w8, w0
    mov w0, w8
    mov w8, w0
    ldr w0, [sp, #12]
    mov w8, w0
    mov w0, w8
    add w8, w8, w0
    mov w0, #3
    mov w8, w0
    mov w0, w8
    add w8, w8, w0
    mov w0, w8
    str w0, [sp, #16]
    ldr w0, [sp, #8]
    mov w8, w0
    mov w0, w8
    mov w8, w0
    mov w0, w8
    add sp, sp, #16
    ret
