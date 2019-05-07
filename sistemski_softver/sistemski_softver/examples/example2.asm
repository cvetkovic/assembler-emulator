.rodata
msg:
.long 1

.text
.global  main

main:
push ebp
mov   	ebp, esp
push   ebp
mov   	ebp, esp
push   ebp
mov   	ebp, esp
push   ebp
mov   	ebp, esp

skip:
mov    eax, 0
mov    esp, ebp
pop    ebp
ret