# Test za direktive align, skip, byte, word, end

.section .data
.skip 0xa
.byte 0x5, 6, 0x7
.skip 1
.byte 4
.align 4
broj_a:
.byte 0xcb
broj_b:
.word 0x12, 34

.global saberi

.section .text

saberi:
mov r0, broj_a
mov r1, broj_b
add r0, r1

ret

.end
ret
ret
ret