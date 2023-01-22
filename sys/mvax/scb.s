/*
 * 	*sccsid = "@(#)scb.s	1.16	(ULTRIX)	4/18/86";
 */

/************************************************************************
 *									*
 *			Copyright (c) 1984,86 by			*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 * Modification History:
 *
 * 18-Apr-86 -- jaw	hooks for nmi faults and fixes to bierrors.
 *
 * 17-Apr-86  -- jaw     re-write scbprot so it protects ALL of the scb.  This
 *			 also fixes the "lost" instack page bug.
 *
 * 15-Apr-86 -- afd
 *	Added dummy definition of "vax$emulate" and "vax$emulate_fpd"
 *	if MVAX is not defined.
 *
 * 15-Apr-86 -- jf   add support for system processes. Uses IPL10 interrupt
 *
 * 02-Apr-86 -- jaw  add support for nautilus console and memory adapter
 *
 * 18-Mar-86 -- jaw  add routines to cpu switch for nexus/unibus addreses
 *		     also got rid of some globals like nexnum.
 *		     ka8800 cleanup.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 16-Apr-85 -- lp
 * 	Added interrupt entry points for vax8200 serial lines.
 *
 * 12-Mar-85 -- darrell
 *	Handle sbi interrupts.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 */

#include "uba.h"

/*
 * System control block
 */
	.set	INTSTK,1	# handle this interrupt on the interrupt stack
	.set	HALT,3		# halt if this interrupt occurs

/*
 * All unused or unexpected vectors in the SCB point to it's relative
 * position in a dummy scb called scb_stray.  The dummy block contains
 * PUSHR and a jsb to Xstray in locore.s.  Xstray calculates the offset
 * in the block and reports it as the vector.
 */

_scb:	.globl	_scb

/*
 * offset in the STRAY def is multiplied by two because the set of
 * instructions used in scb_stray occupys two longwords of storage.
 */

#define STRAY(offset)	.long	_scb_stray+(offset*2)+INTSTK
#define KS(a)	.long	_X/**/a
#define IS(a)	.long	_X/**/a+INTSTK
#define STOP(a) .long	_X/**/a+HALT

/* 000 */	IS(passrel);	IS(machcheck);	IS(kspnotval);	STOP(powfail);
/* 010 */	KS(privinflt);	KS(xfcflt);	KS(resopflt);	KS(resadflt);
/* 020 */	KS(protflt);	KS(transflt);	KS(tracep);	KS(bptflt);
/* 030 */	KS(compatflt);	KS(arithtrap);	STRAY(0x38);	STRAY(0x3c);
/* 040 */	KS(syscall);	KS(chme);	KS(chms);	KS(chmu);
/* 050 */	STRAY(0x50);	IS(cmrd);	IS(sbi0alert);	IS(sbi0flt);
/* 060 */	IS(wtime);	IS(sbi0fail);	STRAY(0x68);	STRAY(0x6c);
/* 070 */	STRAY(0x70);	STRAY(0x74);	STRAY(0x78);	STRAY(0x7c);
/* 080 */	IS(ipintr);	STRAY(0x84);	KS(astflt);	STRAY(0x8c);
/* 090 */	STRAY(0x90);	STRAY(0x94);	STRAY(0x98);	IS(intqueue);
/* 0a0 */	IS(softclock);  STRAY(0xa4);	IS(sysprocint);	STRAY(0xac);
/* 0b0 */	IS(netintr);	STRAY(0xb4);	STRAY(0xb8);	STRAY(0xbc);
/* 0c0 */	IS(hardclock);	STRAY(0xc4);	IS(cnrint1);	IS(cnxint1);
/* 0d0 */	IS(cnrint2);	IS(cnxint2);	IS(cnrint3);	IS(cnxint3);
/* 0e0 */	STRAY(0xe0);	STRAY(0xe4);	STRAY(0xe8);	STRAY(0xec);
/* 0f0 */	IS(consdin);	IS(consdout);	IS(cnrint);	IS(cnxint);
/* ipl 0x14, nexus 0-15 */
/* 100 */	STRAY(0x100);	STRAY(0x104);	STRAY(0x108);	STRAY(0x10c);
/* 110 */	STRAY(0x110);	STRAY(0x114);	STRAY(0x118);	STRAY(0x11c);
/* 120 */	STRAY(0x120);	STRAY(0x124);	STRAY(0x128);	STRAY(0x12c);
/* 130 */	STRAY(0x130);	STRAY(0x134);	STRAY(0x138);	STRAY(0x13c);
/* ipl 0x15, nexus 0-15 */
/* 140 */	STRAY(0x140);	STRAY(0x144);	IS(cmrd);	STRAY(0x14c);
/* 150 */	STRAY(0x150);	STRAY(0x154);	STRAY(0x158);	STRAY(0x15c);
/* 160 */	STRAY(0x160);	STRAY(0x164);	STRAY(0x168);	STRAY(0x16c);
/* 170 */	STRAY(0x170);	STRAY(0x174);	STRAY(0x178);	STRAY(0x17c);
/* ipl 0x16, nexus 0-15 */
/* 180 */	STRAY(0x180);	STRAY(0x184);	STRAY(0x188);	STRAY(0x18c);
/* 190 */	STRAY(0x190);	STRAY(0x194);	STRAY(0x198);	STRAY(0x19c);
/* 1a0 */	STRAY(0x1a0);	STRAY(0x1a4);	STRAY(0x1a8);	STRAY(0x1ac);
/* 1b0 */	STRAY(0x1b0);	STRAY(0x1b4);	STRAY(0x1b8);	STRAY(0x1bc);
/* ipl 0x17, nexus 0-15 */
/* 1c0 */	STRAY(0x1c0);	STRAY(0x1c4);	STRAY(0x1c8);	STRAY(0x1cc);
/* 1d0 */	STRAY(0x1d0);	STRAY(0x1d4);	STRAY(0x1d8);	STRAY(0x1dc);
/* 1e0 */	STRAY(0x1e0);	STRAY(0x1e4);	STRAY(0x1e8);	STRAY(0x1ec);
/* 1f0 */	STRAY(0x1f0);	STRAY(0x1f4);	STRAY(0x1f8);	STRAY(0x1fc);

	.globl	_UNIvec
_UNIvec:

#if defined(VAX8600)
	/*
	 *	extended SCB for second SBI on VENUS
	 */

/* 200 */	STRAY(0x200);	STRAY(0x204);	STRAY(0x208);	STRAY(0x20c);
/* 210 */	STRAY(0x210);	STRAY(0x214);	STRAY(0x218);	STRAY(0x21c);
/* 220 */	STRAY(0x220);	STRAY(0x224);	STRAY(0x228);	STRAY(0x22c);
/* 230 */	STRAY(0x230);	STRAY(0x234);	STRAY(0x238);	STRAY(0x23c);
/* 240 */	STRAY(0x240);	STRAY(0x244);	STRAY(0x248);	STRAY(0x24c);
/* 250 */	STRAY(0x250);	IS(sbi1fail);	IS(sbi1alert);	IS(sbi1flt);
/* 260 */	IS(sbi1error);	STRAY(0x264);	STRAY(0x268);	STRAY(0x26c);
/* 270 */	STRAY(0x270);	STRAY(0x274);	STRAY(0x278);	STRAY(0x27c);
/* 280 */	STRAY(0x280);	STRAY(0x284);	STRAY(0x288);	STRAY(0x28c);
/* 290 */	STRAY(0x290);	STRAY(0x294);	STRAY(0x298);	STRAY(0x29c);
/* 2a0 */	STRAY(0x2a0);	STRAY(0x2a4);	STRAY(0x2a8);	STRAY(0x2ac);
/* 2b0 */	STRAY(0x2b0);	STRAY(0x2b4);	STRAY(0x2b8);	STRAY(0x2bc);
/* 2c0 */	STRAY(0x2c0);	STRAY(0x2c4);	STRAY(0x2c8);	STRAY(0x2cc);
/* 2d0 */	STRAY(0x2d0);	STRAY(0x2d4);	STRAY(0x2d8);	STRAY(0x2dc);
/* 2e0 */	STRAY(0x2e0);	STRAY(0x2e4);	STRAY(0x2e8);	STRAY(0x2ec);
/* 2f0 */	STRAY(0x2f0);	STRAY(0x2f4);	STRAY(0x2f8);	STRAY(0x2fc);
/* ipl 0x14, SBIA1 nexus 0-15 */
/* 300 */	STRAY(0x300);	STRAY(0x304);	STRAY(0x308);	STRAY(0x30c);
/* 310 */	STRAY(0x310);	STRAY(0x314);	STRAY(0x318);	STRAY(0x31c);
/* 320 */	STRAY(0x320);	STRAY(0x324);	STRAY(0x328);	STRAY(0x32c);
/* 330 */	STRAY(0x330);	STRAY(0x334);	STRAY(0x338);	STRAY(0x33c);
/* ipl 0x15, SBIA1 nexus 0-15 */
/* 340 */	STRAY(0x340);	STRAY(0x344);	STRAY(0x348);	STRAY(0x34c);
/* 350 */	STRAY(0x350);	STRAY(0x354);	STRAY(0x358);	STRAY(0x35c);
/* 360 */	STRAY(0x360);	STRAY(0x364);	STRAY(0x368);	STRAY(0x36c);
/* 370 */	STRAY(0x370);	STRAY(0x374);	STRAY(0x378);	STRAY(0x37c);
/* ipl 0x16, SBIA1 nexus 0-15 */
/* 380 */	STRAY(0x380);	STRAY(0x384);	STRAY(0x388);	STRAY(0x38c);
/* 390 */	STRAY(0x390);	STRAY(0x394);	STRAY(0x398);	STRAY(0x39c);
/* 3a0 */	STRAY(0x3a0);	STRAY(0x3a4);	STRAY(0x3a8);	STRAY(0x3ac);
/* 3b0 */	STRAY(0x3b0);	STRAY(0x3b4);	STRAY(0x3b8);	STRAY(0x3bc);
/* ipl 0x17, SBIA1 nexus 0-15 */
/* 3c0 */	STRAY(0x3c0);	STRAY(0x3c4);	STRAY(0x3c8);	STRAY(0x3cc);
/* 3d0 */	STRAY(0x3d0);	STRAY(0x3d4);	STRAY(0x3d8);	STRAY(0x3dc);
/* 3e0 */	STRAY(0x3e0);	STRAY(0x3e4);	STRAY(0x3e8);	STRAY(0x3ec);
/* 3f0 */	STRAY(0x3f0);	STRAY(0x3f4);	STRAY(0x3f8);	STRAY(0x3fc);
#endif VAX8600

	.space (512*NUBA)

/* note that scorpio uses first page for BI vectors. */
/* VAX8800 needs extra BI page to align BI scb  pages. */
#if defined(VAX8800)
	.globl _vax8800bivec

_vax8800bivec:
	.space (5 * 512)

#endif
	.globl _scbend

_scbend:

/*
 * In order to merge MicroVAX support back in with other vaxen,
 * these 2 symbols need to be defined for non-MicroVAX vaxen.
 * If MVAX is defined, then we get the emulation code and hence
 * get the real definition of these.
 */
#ifndef MVAX
	.data
	.globl vax$emulate
	.globl vax$emulate_fpd
vax$emulate:
	.long 0
vax$emulate_fpd:
	.long 0
#endif
