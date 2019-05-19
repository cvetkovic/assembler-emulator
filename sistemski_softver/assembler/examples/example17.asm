mov r0, 1
mov r1, r0
mov r2, memorijsko
mov r3, &memorijsko
mov r4, r3[2]
mov r3, &memorijsko2
mov r5, r3[-2]

add r0, r1
sub r0, r2
mul r0, r1
div r0, 5
cmp r0, r0
and r1, 5
or r2, 0xffff
xor r2, r2
test r0, r1
shl r2, 2
shr r2, 1
push r0
pop r1