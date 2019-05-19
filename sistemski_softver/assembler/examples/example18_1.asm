.section druga

.global greater_equal
.global not_equal
.global equal

greater_equal:
halt

not_equal:
add r0, r1
add r0, r2
halt

equal:
mov r1, 6
cmp r0, r1
jgt greater_equal
mov r2, 7
cmp r2, 3
jne not_equal
halt