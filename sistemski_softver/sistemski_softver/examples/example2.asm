.global b,k
.text
a:
add r1,r2
   jmp 20

   jmp *20

   jmp b

   jmp b

   jmp r1

   jmp r1[2]

   jmp r2[b]
   ret