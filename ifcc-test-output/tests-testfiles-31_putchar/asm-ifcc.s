.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $97, %eax
    movl %eax, -4(%rbp)
    movl -4(%rbp), %eax
    movl %eax, -12(%rbp)
    movl $1, %eax
    addl -12(%rbp), %eax
    movl %eax, %edi
    call putchar@PLT
    movl %eax, -8(%rbp)
    movl -8(%rbp), %eax
    popq %rbp
    ret
