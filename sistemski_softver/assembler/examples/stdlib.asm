# pozvati sa push cifra, call print_digit, pop ...
.text
.global print_digit
.global putchar

# -------------------------------------------
print_digit:
push r0

mov r0, r6[4]
add r0, 48
mov *0xFF00, r0

pop r0
ret

# -------------------------------------------
putchar:
push r0

mov r0, r6[4]
mov *0xFF00, r0

pop r0
ret