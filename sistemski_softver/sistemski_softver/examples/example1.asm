.data
.asciz "Zdravo\n"

.text
.global saberi

saberi:
enter 0, 0

mov eax, [ebp + 8]			# eax = a;
mov ecx, [ebp + 12]			# ecx = b;

add eax, ecx				# eax = a + b;

leave
ret