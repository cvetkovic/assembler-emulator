.data
.skip 10

.text

.global _start

_start:
mov r0, 5
call potprogram
add r0, r1
halt

potprogram:
push r0
add r0, 0xffff
pop r0
ret