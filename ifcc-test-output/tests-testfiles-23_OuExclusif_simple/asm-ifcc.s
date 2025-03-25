.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $27, %eax
    movl %eax, -8(%rbp)
    movl $38, %eax
    xorl -8(%rbp), %eax
    movl %eax, -4(%rbp)
    movl -4(%rbp), %eax
    popq %rbp
    ret
