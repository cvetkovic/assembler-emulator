.global a, c
.extern
.text

jz a
jz e
jz b
jz d

d: 
.long d
mov eax, b
mov c, eax
mov eax, e

.data
.skip 8
e: 
.long a-e+d
.long c
.long .bss
a:
.long b

.bss
c:
.skip 8

.end