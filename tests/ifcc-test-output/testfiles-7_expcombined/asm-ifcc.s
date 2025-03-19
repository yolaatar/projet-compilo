.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $5, %eax
    movl %eax, -4(%rbp)
    movl $9, %eax
    movl %eax, -8(%rbp)
    movl $9, %eax
    movl %eax, -16(%rbp)
    movl -4(%rbp), %eax
    movl %eax, -20(%rbp)
    movl -8(%rbp), %eax
    addl -20(%rbp), %eax
    imul -16(%rbp), %eax
    movl %eax, -12(%rbp)
    movl -12(%rbp), %eax
    popq %rbp
    ret
