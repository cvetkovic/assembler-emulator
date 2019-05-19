# Univerzitet u Beogradu
# Elektrotehnicki fakultet
# Katedra za racunarsku tehniku i informatiku
# 
# Autor: Lazar Cvetkovic, 2016/0127
# Date:  18-May-2019

# interrupt vector table
.section iv_table 
.word ivt_entry0
.word ivt_entry1
.word ivt_entry2
.word ivt_entry3
.skip 8

# read-only and executable section
.section interrupts, "rx"
.global ivt_entry0, ivt_entry1
.global ivt_entry2, ivt_entry3

# cpu initialization
ivt_entry0:
mov r0, 0
mov r1, 0
mov r2, 0
mov r3, 0
mov r4, 0
mov r5, 0
mov r6, 0xFF00		# initial stack pointer
mov *0xFF00, 0		# data_out - for displaying characters
mov *0xFF10, 0		# timer_cfg
halt

# bad_instruction_format
ivt_entry1:
mov *0xFF00, 73		# I
mov *0xFF00, 110	# n
mov *0xFF00, 116	# t
mov *0xFF00, 101	# e
mov *0xFF00, 114	# r
mov *0xFF00, 114	# r
mov *0xFF00, 117	# u
mov *0xFF00, 112	# p
mov *0xFF00, 116	# t
mov *0xFF00, 032	# 
mov *0xFF00, 049	# 1
mov *0xFF00, 032	# 
mov *0xFF00, 105	# i
mov *0xFF00, 110	# n
mov *0xFF00, 118	# v
mov *0xFF00, 111	# o
mov *0xFF00, 107	# k
mov *0xFF00, 101	# e
mov *0xFF00, 100	# d
mov *0xFF00, 033	# !
mov *0xFF00, 010	# [\n]
halt

# timer interrupt routine
ivt_entry2:
# do nothing here
iret

# terminal interrupt routine
ivt_entry3:
push r0
mov r0, *0xFF02
mov *0xFF00, r0	# just print typed character
pop r0
iret