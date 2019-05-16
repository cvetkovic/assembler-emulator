.data
.byte 0,1,2,3,4,5,6,7,
labela1:
.byte 8
labela2:
.byte 9
.byte 10,11,12,13,14,15
labela3:
.word 0xffee

.global labela3

.extern procedura

.text
mov r0, r1
_start:
mov r1, r2
mov r2, r3
mov r4, labela2
call procedura

mov r5, 5