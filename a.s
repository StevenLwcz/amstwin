	.arch armv8-a
	.file	"a.c"
	.text
	.align	2
	.global	main
	.type	main, %function
main:
.LFB6:
	.cfi_startproc
	stp	x29, x30, [sp, -32]!
	.cfi_def_cfa_offset 32
	.cfi_offset 29, -32
	.cfi_offset 30, -24
	mov	x29, sp
	mov	w0, 10
	str	w0, [sp, 28]
	mov	w0, 10
	str	w0, [sp, 24]
	mov	x0, 20
	bl	malloc
	str	x0, [sp, 16]
	ldr	x0, [sp, 16]
	add	x0, x0, 2
	str	x0, [sp, 16]
	ldr	w1, [sp, 28]
	ldr	w0, [sp, 24]
	add	w0, w1, w0
	sxtw	x0, w0
	lsl	x0, x0, 1
	ldr	x1, [sp, 16]
	add	x0, x1, x0
	str	x0, [sp, 16]
	nop
	ldp	x29, x30, [sp], 32
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE6:
	.size	main, .-main
	.ident	"GCC: (Debian 10.2.1-6) 10.2.1 20210110"
	.section	.note.GNU-stack,"",@progbits
