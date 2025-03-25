.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    call getchar@PLT
    movl %eax, -4(%rbp)
    movl -4(%rbp), %eax
    movl %eax, %edi
    call putchar@PLT
    movl $0, %eax
    jmp end
end:
    popq %rbp
    ret
