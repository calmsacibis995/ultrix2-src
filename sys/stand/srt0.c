/*	@(#)srt0.c	1.9	10/21/85*/

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

/* ------------------------------------------------------------------------
 * Modification History: /sys/stand/srt0.c
 *
 * 03-OCT-85 -- jaw
 *	sqeeze space from badwrite.
 *
 * 19-Sep-85 -- jaw
 *	fix so BI SST works properly.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 20-Mar-85 -jaw
 *	Changes for support of the VAX8200 were merged in.
 *
 * 14-Mar-85 -- darrell
 *	Added code so the MicroVAX II can be halted.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 *  5 Jul 84 -- rjl
 * 	Added support for passing through the qvss console flag
 *	from the restart parameter block
 *
 * 19 Mar 84 -- rjl
 *	Repeated changes in initial load point have prompted the conversion
 *	of the relocating code to position independant code.
 *
 * 29 Dec 83 --jmcg
 *	Added MicroVAX_1 support.  Needs to save unit info and initialize
 *	devtype, relocate from 0x3000 instead of 0, and has to accommodate
 *	new SID value when handling mchecks from probing memory.  Fixed
 *	some comments.
 *
 * 28 Dec 83 --jmcg
 *	Derived from 4.2BSD, labeled:
 *		srt0.c	6.1	83/07/29
 *
 * ------------------------------------------------------------------------
 */

#include "../vax/mtpr.h"
#define	LOCORE
#include "../vax/cpu.h"
#define MEMSIZEADDR	0xf0000
/*
 *  The MicroVAX II has a switch on the back that disables the front
 *  panel halt switch.  There is a Q-Bus register that is written with
 *  the proper action upon executing a halt instruction.  This register
 *  is the Q-Bus console program mailbox.
 */
#define QB_CPMBX  0x200B8014	/* MicroVAX II console program mailbox */

/*
 * Startup code for standalone system
 * Non-relocating version -- for programs which are loaded by boot
 * Relocating version for boot itself.
 * UV1 version for MicroVAX_1, for which this is the first code to
 * be executed outside boot rom.
 */

	.globl	_end
	.globl	_edata
	.globl	_main
	.globl	__rtt
	.globl	_configure
	.globl	_cpu
	.globl	_openfirst

	.set	HIGH,31		# mask for total disable

entry:	.globl	entry
#ifdef UV1
	nop;nop			# 0x0101 if treated as mask
#else UV1
	.word	0x0
#endif UV1
	mtpr	$HIGH,$IPL		# just in case
#ifdef UV1
/*
 *	These definitions are based on MicroVAX_1 Boot Prom sources.
 */

#define	RPB$L_BOOTR1	0x20
#define	RPB$L_BOOTR5	0x30
#define	RPB$W_UNIT	0x64
#define RPB$Q_PFNMAP	0x44
#define RPB$Q_PFNMAPADDR 0x48
#define RPB$Q_GOODPAGES	0x4c

/*
 * set up devtype for 'main' with unit booted from.
 */

	movl	$9,r10				# major( ra) = 9
	insv	RPB$W_UNIT(r11),$8,$8,r10	# get unit number
/*
 * Size memory
 */
	movl	RPB$Q_PFNMAPADDR(r11),r8	# get the pfn addr
	movl	RPB$Q_PFNMAP(r11),r7		# get the pfn size 
	clrl	r9				# init good page counter
0:	cmpb	(r8)+,$0xff			# is it 8 good pages ?
	bneq	1f				# if not we're done
	addl2	$8,r9				# 8 more good pages
	sobgtr	r7,0b
1:	ashl	$9,r9,*$MEMSIZEADDR		# convert to pages
	movl	RPB$L_BOOTR5(r11),r11		# Save the bootflags

#endif UV1
#ifdef REL
	movl	$RELOC,sp
#else REL
	movl	$RELOC-0x2400,sp
#endif REL
start:
/*
 * Clear the stack area.
 */
	movab	_edata,r0
clr:
	clrl	(r0)+
	cmpl	r0,sp
	jlss	clr
#ifdef REL
/*
 * Move to higher memory locations to make room for
 * for other programs.
 */
	movab	_edata,r0		# Compute the size
	movab	entry,r1
	subl2	r1,r0
	movc3	r0,entry,(sp)		# Move it
/*
 * Reclear bss segment separately from text and data
 * since movc3 can't move more than 64K bytes
 */
dclr:
	clrl	(r3)+
	cmpl	r3,$_end
	jlss	dclr
/* this loop shouldn't be necessary, but is when booting from an ra81 */
xclr:	
	clrl	(r3)+
	cmpl	r3,$MEMSIZEADDR
	jlss	xclr
	jmp	*abegin
begin:
#endif REL
	mtpr	$0,$SCBB
	calls	$0,_configure
	movl	$1,_openfirst
	calls	$0,_main
#ifndef TP
	jmp	start
#else TP
	ret
#endif TP

	.data
#ifdef REL
abegin:	.long	begin
#endif REL

/*
 *  __rtt is called from _stop, and will loop forever to simulate a
 *  halt.  A halt intruction cannot be used here (except MicroVAX II)
 *  because it will try to reboot if the auto-restart switch is on.
 *
 *  MicroVAX II must be halted, as the halt switch (break key) has
 *  been disabled.
 */

__rtt:
	.word	0x0
	pushab	2f
	calls	$1,_printf		# print the message
	cmpl	$MVAX_II,_cpu		# if MicroVAX II,
	bneq	1f			# then halt
	movb	$0x23,QB_CPMBX		# poke the qbus mailbox
	halt
1:
	brb	1b			# otherwise, loop here forever
/*	jmp	start			# jump to physical address c */
	.data
2:
	.asciz "THE PROCESSOR CAN NOW BE HALTED\n"
	.text

/*
 * badwrte(addr,len,data)
 */
	.globl	_badwrte
_badwrte:
	.word	0
	movl	$1,r0
	movl	4(ap),r3
	movl	$4,r2
	movab	9f,(r2)
	bisl2	$0xc00,(r3)
	movl $30000,r2
1:	sobgtr r2,1b
	clrl	r0			# made it w/o machine checks
2:	movl	$4,r2
	clrl	(r2)
	ret
	.align	2
9:
	mtpr	$0xf,$MCESR
	movl $30000,r2
1:	sobgtr r2,1b

	addl2	(sp)+,sp		# discard mchchk trash
	movab	2b,(sp)
	rei

/*
 * badloc(addr,len)
 */
	.globl	_badloc
_badloc:
	.word	0
	movl	$1,r0
	movl	4(ap),r3
	movl	$4,r2
	movab	9f,(r2)
	bbc	$0,8(ap),1f; tstb (r3)
1:	bbc	$1,8(ap),1f; tstw (r3)
1:	bbc	$2,8(ap),1f; tstl (r3)
1:	clrl	r0			# made it w/o machine checks
2:	movl	$4,r2
	clrl	(r2)
	ret
	.align	2
9:
	casel	_cpu,$1,$VAX_MAX
0:
	.word	8f-0b		# 1 is 780
	.word	5f-0b		# 2 is 750
	.word	5f-0b		# 3 is 730
	.word	6f-0b		# 4 is 8600
	.word	5f-0b		# 5 is 8200 
	.word	9f-0b		# 6 is undefined
	.word	5f-0b		# 7 is MicroVAX I
	.word	9f-0b		# 8 (MicroVAX chip)

				# 730, 750, MicroVAX I
5:
	mtpr	$0xf,$MCESR
	brb	1f

				# 780
8:
#if defined(VAX780)
	mtpr	$0,$SBIFS
	brb	1f
#endif
6:
#if defined(VAX8600)
	mtpr	$0,$EHSR
	brb	1f
#endif
9:
	# don't know how to clear machine checks on undefined processor 
	# just have to hope this works...
1:
	addl2	(sp)+,sp		# discard mchchk trash
	movab	2b,(sp)
	rei























