.section __TEXT,__text,regular,pure_instructions
.globl _main
_main:
    mov w0, #32   // Placer la valeur 42 dans le registre w0 (registre de retour pour ARM64)
    ret
