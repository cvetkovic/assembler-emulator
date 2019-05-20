.data

pocetak:
.word 0xabcd

.equ diff, 2+5

.text
.global _start

_start:
kraj:
add r0, &diff
jmp _start


.section sekcija
halt