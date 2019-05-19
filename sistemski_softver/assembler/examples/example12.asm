.data

#.global diff
.equ diff, labela2-labela1 #a + b * ( c - d ) + ( e - f ) * g / h

labela1:
.word 0xabcd

.text

.word 0xffff
.word 0xbcde
.word 0xcdef
labela2:
.word 0xdef0

.extern a,b,c,d,e,f,g,h
