.data

.text
.global _start
.extern greater_equal
.extern not_equal
.extern equal

_start:

mov r0, 5
cmp r0, 5
jeq equal
halt