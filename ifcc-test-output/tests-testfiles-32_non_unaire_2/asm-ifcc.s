.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $0, %eax
    movl %eax, -4(%rbp)
    movl -4(%rbp), %eax
    cmpl $0, %eax
    sete %al
    movzbl %al, %eax
    jmp end
end:
    popq %rbp
    ret
