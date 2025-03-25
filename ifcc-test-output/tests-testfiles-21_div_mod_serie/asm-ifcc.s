.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $73, %eax
    movl %eax, -8(%rbp)  
    movl $5, %eax
    movl %eax, -12(%rbp)  
    movl -8(%rbp), %eax  
    cltd
    idivl -12(%rbp)
    movl %eax, -16(%rbp)  
    movl $3, %eax
    movl %eax, -20(%rbp)  
    movl -16(%rbp), %eax  
    cltd
    idivl -20(%rbp)
    movl %edx, %eax  
    movl %eax, -24(%rbp)  
    movl $4, %eax
    movl %eax, -28(%rbp)  
    movl -24(%rbp), %eax  
    cltd
    idivl -28(%rbp)
    movl %eax, -32(%rbp)  
    movl $2, %eax
    movl %eax, -36(%rbp)  
    movl -32(%rbp), %eax  
    cltd
    idivl -36(%rbp)
    movl %eax, -40(%rbp)  
    movl $2, %eax
    movl %eax, -44(%rbp)  
    movl -40(%rbp), %eax  
    cltd
    idivl -44(%rbp)
    movl %eax, -48(%rbp)  
    movl $3, %eax
    movl %eax, -52(%rbp)  
    movl -48(%rbp), %eax  
    cltd
    idivl -52(%rbp)
    movl %edx, %eax  
    movl %eax, -56(%rbp)  
    movl $7, %eax
    movl %eax, -60(%rbp)  
    movl -56(%rbp), %eax  
    cltd
    idivl -60(%rbp)
    movl %edx, %eax  
    movl %eax, -4(%rbp)
    movl -4(%rbp), %eax
    popq %rbp
    ret
