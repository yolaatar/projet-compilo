.section __TEXT,__text,regular,pure_instructions
.globl _main
_main:
    sub sp, sp, #16
    str wzr, [sp, #12]
    mov w0, #42
    mov w8, w0
    mov w0, w8
    mov w8, w0
    mov w0, w8
    add sp, sp, #16
    ret
