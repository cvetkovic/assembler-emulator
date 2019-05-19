.data
.byte 0x00

.text

.global _start

_start:

#jmp 0x0fff
#jmp r0
#jmp r0[5]

#jmp skok_labela
jmp $skok_labela
mov r0, r1

add r0, r1
xchg r0, r1

ista_sekcija:
add r0, r1
halt

.section sekcija

skok_labela:
add r0, r1
halt