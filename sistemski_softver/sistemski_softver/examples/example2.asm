.data
.byte 1
.extern f1,f2,f3,f4
.global labela1, labela2		# global before definition
.text
labela1:
add r0, r1
labela2:
add r2, r1

.bss

tekkt: .skip 10

.section text, "xr"
.byte 7

.global tekkt		# global after definition
.section tekst
.byte 8