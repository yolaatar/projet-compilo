.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $3, %eax
    movl %eax, -4(%rbp)
    movl -4(%rbp), %eax
    cmpl $0, %eax
    sete %al
    movzbl %al, %eax
    cmpl $0, %eax
    sete %al
    movzbl %al, %eax
    cmpl $0, %eax
    sete %al
    movzbl %al, %eax
    cmpl $0, %eax
    sete %al
    movzbl %al, %eax
    cmpl $0, %eax
    sete %al
    movzbl %al, %eax
    cmpl $0, %eax
    sete %al
    movzbl %al, %eax
    cmpl $0, %eax
    sete %al
    movzbl %al, %eax
    cmpl $0, %eax
    sete %al
    movzbl %al, %eax
    cmpl $0, %eax
    sete %al
    movzbl %al, %eax
    jmp end
end:
    popq %rbp
    ret
