.data
n:
.byte 6

.text
.global _start

# calculating factoriel
_start:
push n
call fact
pop r3
halt

fact:
mov r1, sp[2]
cmp r1, 0
jne else

then:
mov r0, 1
ret

else:
mov r0, r1
sub r0, 1
push r0
call fact
pop r1
mov r1, sp[2]
mul r0, r1
ret