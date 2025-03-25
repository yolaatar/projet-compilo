.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $42, %eax
    jmp end
    movl $5, %eax
    movl %eax, -4(%rbp)
end:
    popq %rbp
    ret
