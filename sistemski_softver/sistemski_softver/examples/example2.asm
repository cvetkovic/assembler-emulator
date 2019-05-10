.section .text
test_label:
push ebp

.section .data
msg: .byte 1
.skip 4
.word 2
.equ EQU, 0xff

.text
.global  main

main:
push ebp
mov   	ebp, esp
push   ebp
.align 4
mov   	ebp, esp
push   ebp
mov   	ebp, esp
push   ebp
mov   	ebp, esp

skip:
mov    eax, 0
mov    esp, ebp
pop    ebp
ret