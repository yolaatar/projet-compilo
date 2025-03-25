.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $4, %eax
    movl %eax, -16(%rbp)
    movl $3, %eax
    addl -16(%rbp), %eax
    movl %eax, -4(%rbp)
    movl $5, %eax
    movl %eax, -8(%rbp)
    movl -4(%rbp), %eax
    movl %eax, -20(%rbp)
    movl -8(%rbp), %eax
    addl -20(%rbp), %eax
    movl %eax, -24(%rbp)
    movl $3, %eax
    addl -24(%rbp), %eax
    movl %eax, -12(%rbp)
    movl -4(%rbp), %eax
    jmp end
end:
    popq %rbp
    ret
