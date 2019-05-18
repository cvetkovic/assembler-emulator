.data
.skip 16

.text
.global _start

_start:
movb r0l, 5
movb r0h, 1
addb r0l, r0h

halt