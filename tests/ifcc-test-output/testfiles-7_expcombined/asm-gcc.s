	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 15, 0	sdk_version 15, 1
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #16
	.cfi_def_cfa_offset 16
	str	wzr, [sp, #12]
	mov	w8, #5                          ; =0x5
	str	w8, [sp, #8]
	mov	w8, #9                          ; =0x9
	str	w8, [sp, #4]
	ldr	w9, [sp, #8]
	ldr	w10, [sp, #4]
	add	w9, w9, w10
	mul	w8, w8, w9
	str	w8, [sp]
	ldr	w0, [sp]
	add	sp, sp, #16
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
