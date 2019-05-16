.data
.byte 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,255

.extern labela3
.global procedura

.text
procedura:
mov r4, r5
mov r2, labela3
mov r1, r2
mov r0, r1
ret