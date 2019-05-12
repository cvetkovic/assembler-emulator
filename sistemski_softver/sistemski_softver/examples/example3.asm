.global a, c
.extern
.text

jz a
jz e
jz b
jz d

d: 
.long d
mov eax, b
mov c, eax
mov eax, e

.data
.skip 8
e: 
.long a-e+d
.long c
.long .bss
a:
.long b

.bss
c:
.skip 8

.end

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