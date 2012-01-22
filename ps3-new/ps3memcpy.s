	.section	".toc","aw"
	.section	".text"
	.align 2
	.globl ps3memcpy
	.section	".opd","aw"

	.align 2
ps3memcpy:
	.long	.ps3memcpy,.TOC.@tocbase32

	.previous
#	.size	ps3memcpy,.-ps3memcpy
	.type	.ps3memcpy,@function
	.globl	.ps3memcpy
.ps3memcpy:
	
	extrdi 6,5,3,57 # 128 > ... >= 16
	srdi   5,5,7    # size >> 7
	cmpdi  0,6,0
	rldicl 4,4,0,32
	rldicl 3,3,0,32
	li 0, 0
	beq    0, .L190 # no remained data

.L100:	# less than 128
	mtctr 6
.L110:		
	lvx 0,0,4
	stvx 0,0,3
	addi 4,4,16
	addi 3,3,16
	bdnz .L110

.L190:	# size is aligned 128
	cmpdi  0,5,0
	beq    0, .L300
	mtctr 5
	li 8, 16
	li 9, 32
	li 10,48
	li 11,64
	li 7, 80
	li 12,96
	li 6,112
.L200:
	lvx 0,0,4
	lvx 1,8,4
	lvx 2,9,4
	lvx 3,10,4
	lvx 4,11,4
	lvx 5,7,4
	lvx 6,12,4
	lvx 7,6,4

	stvx 0,0,3
	stvx 1,8,3
	stvx 2,9,3
	stvx 3,10,3
	stvx 4,11,3
	addi 5,4,1024
	stvx 5,7,3
	addi 4,4,128
	stvx 6,12,3
	stvx 7,6,3

	addi 3,3,128
	dcbt 0, 5
	bdnz .L200
.L300:	
	blr
	
	.long 0
	.byte 0,0,0,0,0,0,0,0
	.size	.ps3memcpy,.-.ps3memcpy

