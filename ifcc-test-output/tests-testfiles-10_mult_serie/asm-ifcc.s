.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $3, %eax
    movl %eax, -20(%rbp)  
    movl $2, %eax
    imull -20(%rbp), %eax
    movl %eax, -4(%rbp)
    movl $3, %eax
    movl %eax, -24(%rbp)  
    movl -4(%rbp), %eax
    imull -24(%rbp), %eax
    movl %eax, -8(%rbp)
    movl $2, %eax
    movl %eax, -28(%rbp)  
    movl $4, %eax
    imull -28(%rbp), %eax
    movl %eax, -12(%rbp)
    movl -8(%rbp), %eax
    movl %eax, -32(%rbp)  
    movl -12(%rbp), %eax
    imull -32(%rbp), %eax
    movl %eax, -16(%rbp)
    movl -8(%rbp), %eax
    movl %eax, -36(%rbp)  
    movl -16(%rbp), %eax
    imull -36(%rbp), %eax
    popq %rbp
    ret
