/*
 *	@(#)csboot.s	1.1 (ULTRIX) 6/28/85
 */

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 * Modification History:
 *
 * 17-JUN-85 - jaw
 * 	support for RX50 on scorpio.
 */
	.set	RELOC,0x50000
	.set	BOOTLAST,15		/* last boot sector */
	.set	RABPSECT,512		/* bytes per sector */

init:
	.word	0  			/* entry mask for dec monitor */
	nop;nop;nop;nop;nop;nop;nop;nop /* some no-ops for 750 boot to skip */
	nop;nop;
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

	.set	PROGSIZE,(BOOTLAST*RABPSECT)
done:
	tstl	(sp)+			/* pop address */
	popr	$0xffff			/* END FIREWALL */
	rotl	$8,r3,r10
	movl	$PROGSIZE,r3
clrcor:
	clrq	(r3)
	acbl	$RELOC,$8,r3,clrcor
/* start loaded program */
	addl2	$0x10,r10			/* major("/dev/cx0a") */
	movl 	r2,r9			/* boot device CSR */
	calls	$0,*$0
	brw	start2
end:


