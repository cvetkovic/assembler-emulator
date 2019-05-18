# Univerzitet u Beogradu
# Elektrotehnicki fakultet
# Katedra za racunarsku tehniku i informatiku
# 
# Autor: Lazar Cvetkovic, 2016/0127
# Date:  18-May-2019

.section iv_table 
.word ivt_entry0
.word ivt_entry1
.word ivt_entry2
.word ivt_entry3
.skip 8

# read-only and executable section
.section interrupts, "rx"
.global ivt_entry0, ivt_entry1

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

ivt_entry1:
halt

ivt_entry2:
halt

ivt_entry3:
halt