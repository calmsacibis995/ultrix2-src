

# 	@(#)move.s	1.1	(ULTRIX)	12/9/84

.globl _move
_move:	.word 0x0000
	movc3 12(ap),*4(ap),*8(ap)
	ret
