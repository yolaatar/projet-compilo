.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $3, %eax
    movl %eax, -4(%rbp)
    movl -4(%rbp), %eax
    movl %eax, -8(%rbp)
    movl $2, %eax
    cmpl -8(%rbp), %eax
    setl %al
    movzbl %al, %eax
    popq %rbp
    ret
