	.arch armv7-a
	.eabi_attribute 28, 1
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 6
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.data
	.globl a
a:
	.word	0

	.globl b
b:
	.word	0

	.text
	.globl main
	.syntax unified
	.type    main, %function
main:
	push {r7, lr}
	sub    sp, sp, #20
	add    r7, sp, #0
	mov r3, #30
	str r3, [r7, #8]
	ldr r3, [r7, #8]
	mov r0, r3
	mov sp, r7
	add  sp, sp, #20
	pop {r7, pc}


