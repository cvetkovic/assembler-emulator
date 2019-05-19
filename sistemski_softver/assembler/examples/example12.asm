.data

.text

.global _start

_start:
mov r1, 5
mov r2, 6
xchg r1, r2
not r1
jmp kraj

add r1, r2
sub r2, r3

kraj:
int 1