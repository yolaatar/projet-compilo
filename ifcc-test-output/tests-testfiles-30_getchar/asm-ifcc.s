.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    call getchar@PLT
    movl %eax, -4(%rbp)
    movl -4(%rbp), %eax
    jmp end
end:
    popq %rbp
    ret
