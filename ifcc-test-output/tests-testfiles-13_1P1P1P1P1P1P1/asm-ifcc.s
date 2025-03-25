.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $1, %eax
    movl %eax, -4(%rbp)
    movl $1, %eax
    movl %eax, -8(%rbp)
    movl $1, %eax
    movl %eax, -12(%rbp)
    movl $1, %eax
    movl %eax, -16(%rbp)
    movl $1, %eax
    movl %eax, -20(%rbp)
    movl $1, %eax
    movl %eax, -24(%rbp)
    movl $1, %eax
    movl %eax, -28(%rbp)
    movl $1, %eax
    movl %eax, -32(%rbp)
    movl $1, %eax
    movl %eax, -36(%rbp)
    movl $1, %eax
    movl %eax, -40(%rbp)
    movl $1, %eax
    movl %eax, -44(%rbp)
    movl $1, %eax
    movl %eax, -48(%rbp)
    movl $1, %eax
    movl %eax, -52(%rbp)
    movl $1, %eax
    movl %eax, -56(%rbp)
    movl $1, %eax
    movl %eax, -60(%rbp)
    movl $1, %eax
    addl -60(%rbp), %eax
    addl -56(%rbp), %eax
    addl -52(%rbp), %eax
    addl -48(%rbp), %eax
    addl -44(%rbp), %eax
    addl -40(%rbp), %eax
    addl -36(%rbp), %eax
    addl -32(%rbp), %eax
    addl -28(%rbp), %eax
    addl -24(%rbp), %eax
    addl -20(%rbp), %eax
    addl -16(%rbp), %eax
    addl -12(%rbp), %eax
    addl -8(%rbp), %eax
    addl -4(%rbp), %eax
    jmp end
end:
    popq %rbp
    ret
