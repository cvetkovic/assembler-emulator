.data
n:
.byte 7

.text
.global _start
.extern print_int

# calculating factoriel
_start:
push n
call fact
pop r3

xor r3, r3

#u r0 je vec parametar
call print_int

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