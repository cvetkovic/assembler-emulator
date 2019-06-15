.text
.global print_int

print_int:

petlja:
cmp r0, 0
jeq kraj

mov r1, r0
div r1, 10
mul r1, 10
mov r2, r0
sub r2, r1	# mod
div r0, 10

push r2
call print
pop r2

jmp petlja

kraj:
ret

print:
push r0

mov r0, r6[4]
add r0, 48
mov *0xFF00, r0

pop r0
ret