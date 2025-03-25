.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $3, %eax
    movl %eax, -20(%rbp)
    movl $2, %eax
    movl %eax, -24(%rbp)  
    movl $4, %eax
    imull -24(%rbp), %eax
    subl %eax, -20(%rbp)
    movl -20(%rbp), %eax
    movl %eax, -28(%rbp)
    movl $5, %eax
    addl -28(%rbp), %eax
    movl %eax, -32(%rbp)
    movl $7, %eax
    movl %eax, -36(%rbp)  
    movl $7, %eax
    imull -36(%rbp), %eax
    movl %eax, -40(%rbp)  
    movl $3, %eax
    movl %eax, -44(%rbp)  
    movl -40(%rbp), %eax  
    cltd
    idivl -44(%rbp)
    movl %edx, %eax  
    movl %eax, -48(%rbp)
    movl $4, %eax
    movl %eax, -52(%rbp)
    movl $2, %eax
    addl -52(%rbp), %eax
    movl %eax, -56(%rbp)  
    movl $2, %eax
    movl %eax, -60(%rbp)  
    movl -56(%rbp), %eax  
    cltd
    idivl -60(%rbp)
    addl -48(%rbp), %eax
    addl -32(%rbp), %eax
    movl %eax, -4(%rbp)
    movl $2, %eax
    movl %eax, -64(%rbp)  
    movl $2, %eax
    imull -64(%rbp), %eax
    movl %eax, -68(%rbp)  
    movl $4, %eax
    imull -68(%rbp), %eax
    movl %eax, -72(%rbp)  
    movl $2, %eax
    movl %eax, -76(%rbp)  
    movl -72(%rbp), %eax  
    cltd
    idivl -76(%rbp)
    movl %eax, -80(%rbp)  
    movl $4, %eax
    movl %eax, -84(%rbp)  
    movl -80(%rbp), %eax  
    cltd
    idivl -84(%rbp)
    movl %edx, %eax  
    movl %eax, -88(%rbp)  
    movl $2, %eax
    movl %eax, -92(%rbp)  
    movl -88(%rbp), %eax  
    cltd
    idivl -92(%rbp)
    movl %eax, -96(%rbp)
    movl $67, %eax
    movl %eax, -100(%rbp)
    movl $4, %eax
    addl -100(%rbp), %eax
    movl %eax, -104(%rbp)
    movl $356, %eax
    andl -104(%rbp), %eax
    movl %eax, -108(%rbp)
    movl $37, %eax
    andl -108(%rbp), %eax
    orl -96(%rbp), %eax
    movl %eax, -8(%rbp)
    movl $3, %eax
    movl %eax, -112(%rbp)  
    movl -8(%rbp), %eax
    imull -112(%rbp), %eax
    movl %eax, -116(%rbp)
    movl $7, %eax
    addl -116(%rbp), %eax
    movl %eax, -120(%rbp)
    movl $8, %eax
    movl %eax, -124(%rbp)  
    movl $4, %eax
    movl %eax, -128(%rbp)  
    movl -124(%rbp), %eax  
    cltd
    idivl -128(%rbp)
    movl %edx, %eax  
    subl %eax, -120(%rbp)
    movl -120(%rbp), %eax
    movl %eax, -132(%rbp)
    movl $8, %eax
    movl %eax, -136(%rbp)  
    movl -8(%rbp), %eax
    imull -136(%rbp), %eax
    addl -132(%rbp), %eax
    movl %eax, -140(%rbp)
    movl $47, %eax
    xorl -140(%rbp), %eax
    movl %eax, -144(%rbp)
    movl $52, %eax
    xorl -144(%rbp), %eax
    movl %eax, -12(%rbp)
    movl -4(%rbp), %eax
    movl %eax, -148(%rbp)
    movl -8(%rbp), %eax
    movl %eax, -152(%rbp)  
    movl -8(%rbp), %eax
    imull -152(%rbp), %eax
    subl %eax, -148(%rbp)
    movl -148(%rbp), %eax
    movl %eax, -156(%rbp)
    movl -12(%rbp), %eax
    movl %eax, -160(%rbp)  
    movl -4(%rbp), %eax
    movl %eax, -164(%rbp)  
    movl -160(%rbp), %eax  
    cltd
    idivl -164(%rbp)
    addl -156(%rbp), %eax
    movl %eax, -16(%rbp)
    movl -16(%rbp), %eax
    popq %rbp
    ret
