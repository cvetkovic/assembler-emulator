.bss 
.skip 12

.data

pocetak:
.word 0xabcd

.equ diff, kraj-_start

.text
.global _start

_start:
add r0, &diff
kraj:
jmp _start
