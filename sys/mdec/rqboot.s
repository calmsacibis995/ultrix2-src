/*************************************************************************
 *
 *		Copyright (c) 1983 by
 *	Digital Equipment Corporation, Maynard, MA
 *		All rights reserved.
 *
 * This software is furnished under a license and may be used and copied
 * only in accordance with the terms of such license and with the
 * inclusion of the above copyright notice.  This software or any other
 * copies thereof may not be provided or otherwise made available to any
 * other person.  No title to and ownership of the software is hereby
 * transferred.
 *
 * This software is derived from software received from the University of
 * California, Berkeley, and from Bell Laboratories.  Use, duplication, or
 * disclosure is subject to restrictions under license agreements with
 * University of California and with AT&T.
 *
 * The information in this software is subject to change without notice
 * and should not be construed as a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or reliability of its
 * software on equipment which is not supplied by Digital.
 *
 *************************************************************************/

	/* "@(#)rqboot.s	1.4 (decvax!jmcg) 11/10/83 */

/* ------------------------------------------------------------------------
 * Modification History:
 *
 *  8 Mar 84 -- rjl
 *	Changed to new boot block format for the production units.
 *
 * 10 Oct 83 --jmcg
 *	Some counting errors were fixed.  These were serious enough to
 *	prevent the boot from working.
 *
 *  6 Oct 83 --jmcg
 *	The proposed bootblock standard for MicroVAX floppies has been
 *	followed.
 *
 *  6 Oct 83 --jmcg
 *	Derived from 4.2BSD pre-release, labeled:
 *		raboot.s	6.1	83/07/29
 *	It must be RE-MASTERED when 4.2BSD final release arrives.
 * ------------------------------------------------------------------------
 */

init:
/*00*/	.byte	0x01	/* nop */ /* pdp-11 branch to 04 */
	.byte	0x01	/* nop */
/*02*/	.byte	0x11	/* brb  0x05 (when starting at byte 2) */
	.byte	0x01	/* needed to make second word a PDP-11 branch */
/*04*/	.word	0x00	/* High 16 bits of secondary boot lbn */
/*06*/	.word	0x01	/* Low 16 bits of secondary boot lbn */
/*08*/	.long	0	/* "nobody cares" */
/*0c*/	brb	start	/* jump to vax part of bootstrap */
/*0e*/	.long	0	/* "nobody cares" */
/*12*/	.long	0	/* "nobody cares" */
/*16*/	.long	0	/* "nobody cares" */
/*1a*/	.long	0	/* "nobody cares" */
/*1e*/	.long	0	/* "nobody cares" */

_ident:			/* beginning of identification region */
/*22*/	.byte	0x18	/* signifies "VAX instruction set" */
	.byte	0	/* signifies "unspecified controller type" */
/*24*/	.byte	0x54	/* signifies "VAX 4.2BSD file system format" */
	.byte	~(0x18+0x54)	/* complement of sum of previous 3 bytes */
/*26*/	.byte	0	/* this word is the target of the pdp-11 branch at 02 */
	.byte	1	/* or 0x81 for two-sided floppy */
			/* the word at 26 is a nop PDP-11 branch instruction */
			/* it also encodes the version number of the standard */
/*28*/	.byte	_pdp11-0f	/* word displacement to pdp-11 part of bootblock */
	.byte 	1	/* makes this word a br to pdp-11 part of bootblock */
			/* if there's no room for pdp-11 part, this word is 0 */
0:

/*2a*/	.long	0x0f	/* size (512-byte blocks) of secondary boot image */
/*2e*/	.long	0x00	/* Load offset into good memory */
/*32*/	.long	0x02	/* transfer address of secondary boot image */
/*36*/	.long	0x11	/* longword sum of previous three longwords */
_vaxb:
/*3a*/
	/* vax boot begins here */
	brb	start
	.align 1
_pdp11:
	.word	0004067	/* jsr r0,$1 */
	.word	1f-0f
0:
	.asciz	"boot volume for VAX only\r\n"
	.align 1
1:
	.word	0105737	/* tstb *$ttps */
	.word	0177564
	.word	0100374	/* bpl 1b */
	.word	0112037	/* movb (r0)+,*$ttpb */
	.word	0177566
	.word	0003371	/* bgt 1b */
2:
	.word	0000777	/* br 2b (i.e. stall here) */


/*
 * UDA50 1st level boot program: loads next 7.5Kbytes from
 * boot sector of file system and sets it up to run.
 */
	.set	RELOC,0x50000
	.set	BOOTLAST,15		/* last boot sector */
	.set	RABPSECT,512		/* bytes per sector */

start:
	movl	r1,r7			/* UNIBUS I/O page address */
	movl	r2,r8			/* boot device CSR */
	movl	r3,r9			/* unit number */
	movl	r5,r11			/* boot flags */
	movl	$RELOC,sp
	moval	init,r10
	movc3	$end,(r10),(sp)
	movl	r7,r1			/* UNIBUS I/O page address */
	movl	r8,r2			/* boot device CSR */
	movl	r9,r3			/* unit number */
	jmp	*$RELOC+start2
/* now running relocated */
/* bring in the boot program */
start2:					/* running relocated */
	pushr	$0xffff			/* BEGIN FIREWALL */
	movl	$1,r4			/* first boot sector */
	clrl	r5			/* transfer address */
	clrl	-(sp)			/* transfer address */
1:
	movl	r4,r8			/* requested sector # */
	jsb	(r6)			/* call ROM-based driver */
	blbs	r0,2f
	halt				/* read error */
2:
	addl2	$RABPSECT,r5		/* bump address */
	movl	r5,(sp)
	aobleq	$BOOTLAST,r4,1b

	.set	PROGSIZE,((BOOTLAST-1)*RABPSECT)
done:
	tstl	(sp)+			/* pop address */
	popr	$0xffff			/* END FIREWALL */
	movl	$PROGSIZE,r3
clrcor:
	clrq	(r3)
	acbl	$RELOC,$8,r3,clrcor
/* start loaded program */
	movl	$9,r10			/* major("/dev/ra0a") */
	calls	$0,*$0
	brw	start2
end:
