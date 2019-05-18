# to be used for interrupt testing
.text
.global _start

_start:
mov r0, 0xabcd
jmp _start