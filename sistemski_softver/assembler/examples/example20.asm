.data
string:
.byte 76, 97, 122, 97, 114, 32, 77, 46, 32, 67, 118, 101, 116, 107, 111, 118, 105, 99, 10, 0



.text
.extern putchar
.global _start

_start:

xor r0, r0
xor r1, r1

mov r0, &string

petlja:
movb r1l, r0[0]
cmpb r1l, 0
jeq kraj

push r1
call putchar
pop r1

add r0, 1
jmp petlja

kraj:
halt