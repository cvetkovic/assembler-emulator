.data
podatak:
.byte 5

.global _start

.text
_start:
mov r0, 10
mov r0[0], 0xffff
mov r0, 4
mov r1, 0
mov r1, r0
mov r1, r0[6]

halt