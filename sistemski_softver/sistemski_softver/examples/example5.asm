# addressing modes testing

.data
.skip 10
simbol:
.byte 0x45
simbol1:
.byte 0
simbol2:
.byte 0

.text

#immediately 
mov r7, 25
mov r0, &simbol
movb r7h, 25
movb r0l, &simbol
mov r7, 0xab
mov r7, 0xabcd
mov r7, 0xabcdef

#register direct
mov r0, r1
movb r0h, r1l
movw r7, r6

#register indirect
mov [sp], r0
mov [pc], r0

mov r5[8], r0
mov r5[0xcd], r0
mov r5[0xabcd], r0
mov r5[0x123456], r0
mov r5[0], r0
mov r5[simbol], r0
mov r0, r5[simbol]

movb r0l, r6[simbol]
movb r6[simbol], r0h

#pc relative with symbol
mov r0, $simbol
movb r0h, $simbol

#memory direct
mov *56, r0
mov r1, *0xabcd
mov r2, simbol1
mov simbol2, r3
movb r2l, simbol1
movb simbol2, r3l