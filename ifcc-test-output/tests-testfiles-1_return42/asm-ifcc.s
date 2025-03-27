.globl main
 main: 
    pushq %rbp
    movq %rsp, %rbp
    movl $42, %eax
    jmp end
end:
    popq %rbp
    ret
