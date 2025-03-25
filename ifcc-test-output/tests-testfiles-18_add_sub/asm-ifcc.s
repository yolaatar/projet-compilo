.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $3, %eax
    movl %eax, -8(%rbp)
    movl $2, %eax
    subl %eax, -8(%rbp)
    movl -8(%rbp), %eax
    movl %eax, -12(%rbp)
    movl $5, %eax
    addl -12(%rbp), %eax
    movl %eax, -16(%rbp)
    movl $7, %eax
    subl %eax, -16(%rbp)
    movl -16(%rbp), %eax
    movl %eax, -20(%rbp)
    movl $2, %eax
    subl %eax, -20(%rbp)
    movl -20(%rbp), %eax
    movl %eax, -24(%rbp)
    movl $4, %eax
    addl -24(%rbp), %eax
    movl %eax, -28(%rbp)
    movl $6, %eax
    addl -28(%rbp), %eax
    movl %eax, -32(%rbp)
    movl $3, %eax
    addl -32(%rbp), %eax
    movl %eax, -36(%rbp)
    movl $2, %eax
    subl %eax, -36(%rbp)
    movl -36(%rbp), %eax
    movl %eax, -4(%rbp)
    movl -4(%rbp), %eax
    popq %rbp
    ret
