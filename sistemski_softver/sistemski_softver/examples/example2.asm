.section .text
.extern eksterni_simbol

test_label:
mov r7, r2[5]
mov r7, r2[0xfa]
mov r7, r2[symbol]
mov r7, r2[-4]

.global test_label

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