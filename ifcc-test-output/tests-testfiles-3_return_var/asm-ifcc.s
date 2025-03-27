.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $5, %eax
    movl %eax, -4(%rbp)
    movl -4(%rbp), %eax
    movl %eax, -8(%rbp)
    movl -4(%rbp), %eax
    jmp end
end:
    popq %rbp
    ret
