.section .data
broj_a:
.byte 10
broj_b:
.byte 20

.global saberi

.section .text

saberi:
mov r0, broj_a
mov r1, broj_b
add r0, r1

ret