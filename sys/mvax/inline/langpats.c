#ifndef lint
static char *sccsid = "@(#)langpats.c	1.6	(ULTRIX)	4/2/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984,86 by			*
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

/*-----------------------------------------------------------------------
 *
 * Modification History
 *
 * 02-Apr-86 -- jrs
 *	Changed lock pattern for efficiency.  Also only do cpuindex
 *	if number if active cpus greater than one.
 *
 * 18 Mar 86 -- jrs
 *	Changed patterns for cpuindex and cpuident to match new locore
 *
 * 16 Jul 85 -- jrs
 *	Added patterns for lock and unlock primitives
 *
 *	Based on 4.2bsd labeled:
 *		langpats.c	2.6 (Berkeley) 11/19/84
 *
 */

#include "inline.h"

/*
 * Pattern table for kernel specific routines.
 * These patterns are based on the old asm.sed script.
 */
struct pats language_ptab[] = {

#ifdef vax
	{ "0,_spl0\n",
"	mfpr	$18,r0\n\
	mtpr	$0,$18\n" },

	{ "0,_spl1\n",
"	mfpr	$18,r0\n\
	mtpr	$1,$18\n" },

	{ "0,_splsoftclock\n",
"	mfpr	$18,r0\n\
	mtpr	$0x8,$18\n" },

	{ "0,_splnet\n",
"	mfpr	$18,r0\n\
	mtpr	$0xc,$18\n" },

	{ "0,_splimp\n",
"	mfpr	$18,r0\n\
	mtpr	$0x16,$18\n" },

	{ "0,_spl4\n",
"	mfpr	$18,r0\n\
	mtpr	$0x14,$18\n" },

	{ "0,_splbio\n",
"	mfpr	$18,r0\n\
	mtpr	$0x15,$18\n" },

	{ "0,_spltty\n",
"	mfpr	$18,r0\n\
	mtpr	$0x15,$18\n" },

	{ "0,_spl5\n",
"	mfpr	$18,r0\n\
	mtpr	$0x15,$18\n" },

	{ "0,_splclock\n",
"	mfpr	$18,r0\n\
	mtpr	$0x18,$18\n" },

	{ "0,_spl6\n",
"	mfpr	$18,r0\n\
	mtpr	$0x18,$18\n" },

	{ "0,_splhigh\n",
"	mfpr	$18,r0\n\
	mtpr	$0x18,$18\n" },

	{ "0,_spl7\n",
"	mfpr	$18,r0\n\
	mtpr	$0x1f,$18\n" },

	{ "0,_splextreme\n",
"	mfpr	$18,r0\n\
	mtpr	$0x1f,$18\n" },

	{ "1,_splx\n",
"	movl	(sp)+,r1\n\
	mfpr	$18,r0\n\
	mtpr	r1,$18\n" },

	{ "1,_mfpr\n",
"	movl	(sp)+,r5\n\
	mfpr	r5,r0\n" },

	{ "2,_mtpr\n",
"	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	mtpr	r5,r4\n" },

	{ "0,_setintqueue\n",
"	mtpr	$0x7,$0x14\n" },

	{ "0,_setsoftclock\n",
"	mtpr	$0x8,$0x14\n" },

	{ "1,_resume\n",
"	movl	(sp)+,r5\n\
	ashl	$9,r5,r0\n\
	movpsl	-(sp)\n\
	jsb	_Resume\n" },

#ifdef THIS_IS_A_COMMENT
/*
 * Copy 1 relocation unit (NBPG bytes)
 * from user virtual address to physical address
 */
_copyseg:	.globl	_copyseg
	.word	0x0
	bisl3	$PG_V|PG_KW,8(ap),_CMAP2
	mtpr	$_CADDR2,$TBIS	# invalidate entry for copy
	movc3	$NBPG,*4(ap),_CADDR2
	ret
/*
 * zero out physical memory
 * specified in relocation units (NBPG bytes)
 */
_clearseg:	.globl	_clearseg
	.word	0x0
	bisl3	$PG_V|PG_KW,4(ap),_CMAP1
	mtpr	$_CADDR1,$TBIS
	movc5	$0,(sp),$0,$NBPG,_CADDR1
	ret
#endif THIS_IS_A_COMMENT
/*
 * The above two routines were in locore.s and were moved here
 * for efficiency reasons. Both are called heavily as "calls"
 * subroutines in the kernel. copyseg is called only in vmdup()
 * when forking. clearseg is called to zero out 512 byte pages
 * of memory in 5 routines (zero fill is the biggest hitter of this routine).
 * So we have moved them to inline where we don't have the macros
 * facility and we have to hard code some constants. These constants
 * won't be a problem until we get new hardware but then this had
 * to change anyway. The change in the code from what it was in
 * locore to what it is in inline is interesting to say the least. (rr).
 */
	{ "2,_copyseg\n",
"	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	bisl3	$0x90000000,r5,_CMAP2\n\
	mtpr	$_CADDR2,$0x3a\n\
	movc3	$0x200,(r4),_CADDR2\n" },

	{ "1,_clearseg\n",
"	movl	(sp)+,r5\n\
	bisl3	$0x90000000,r5,_CMAP1\n\
	mtpr	$_CADDR1,$0x3a\n\
	movc5	$0,(sp),$0,$0x200,_CADDR1\n" },

	{ "3,_strncmp\n",
"	movl	(sp)+,r1\n\
	movl	(sp)+,r3\n\
	movl	(sp)+,r5\n\
	cmpc3	r5,(r1),(r3)\n" },

	{ "3,_copyin\n",
"	movl	(sp)+,r1\n\
	movl	(sp)+,r3\n\
	movl	(sp)+,r5\n\
	jsb	_Copyin\n" },

	{ "3,_copyout\n",
"	movl	(sp)+,r1\n\
	movl	(sp)+,r3\n\
	movl	(sp)+,r5\n\
	jsb	_Copyout\n" },

	{ "1,_fubyte\n",
"	movl	(sp)+,r0\n\
	jsb	_Fubyte\n" },

	{ "1,_fuibyte\n",
"	movl	(sp)+,r0\n\
	jsb	_Fubyte\n" },

	{ "1,_fuword\n",
"	movl	(sp)+,r0\n\
	jsb	_Fuword\n" },

	{ "1,_fuiword\n",
"	movl	(sp)+,r0\n\
	jsb	_Fuword\n" },

	{ "2,_subyte\n",
"	movl	(sp)+,r0\n\
	movl	(sp)+,r1\n\
	jsb	_Subyte\n" },

	{ "2,_suibyte\n",
"	movl	(sp)+,r0\n\
	movl	(sp)+,r1\n\
	jsb	_Subyte\n" },

	{ "2,_suword\n",
"	movl	(sp)+,r0\n\
	movl	(sp)+,r1\n\
	jsb	_Suword\n" },

	{ "2,_suiword\n",
"	movl	(sp)+,r0\n\
	movl	(sp)+,r1\n\
	jsb	_Suword\n" },

	{ "1,_lock\n",
"	movl	(sp)+,r0\n\
1:\n\
	bbssi	r0,_lockword,2f\n\
	movab	3f,_lastlock\n\
	brb	3f\n\
2:\n\
	bbc	r0,_lockword,1b\n\
	brb	2b\n\
3:\n" },

	{ "1,_unlock\n",
"	bbcci	(sp)+,_lockword,1f\n\
1:\n" },

	{ "1,_setrq\n",
"	movl	(sp)+,r0\n\
	jsb	_Setrq\n" },

	{ "1,_remrq\n",
"	movl	(sp)+,r0\n\
	jsb	_Remrq\n" },

	{ "0,_swtch\n",
"	movpsl	-(sp)\n\
	jsb	_Swtch\n" },

	{ "1,_setjmp\n",
"	movl	(sp)+,r1\n\
	clrl	r0\n\
	movl	fp,(r1)+\n\
	moval	1(pc),(r1)\n" },

	{ "1,_longjmp\n",
"	movl	(sp)+,r0\n\
	jsb	_Longjmp\n" },

	{ "0,_cpuindex\n",
"	cmpl	$1,_activecpu\n\
	bneq	1f\n\
	clrl	r0\n\
	brb	2f\n\
1:\n\
	jsb	cpuindex\n\
2:\n" },

	{ "0,_cpuident\n",
"	jsb	cpuident\n" },

	{ "1,_ffs\n",
"	movl	(sp)+,r1\n\
	ffs	$0,$32,r1,r0\n\
	bneq	1f\n\
	mnegl	$1,r0\n\
1:\n\
	incl	r0\n" },

	{ "1,_htons\n",
"	movl	(sp)+,r5\n\
	rotl	$8,r5,r0\n\
	rotl	$-8,r5,r1\n\
	movb	r1,r0\n\
	movzwl	r0,r0\n" },

	{ "1,_ntohs\n",
"	movl	(sp)+,r5\n\
	rotl	$8,r5,r0\n\
	rotl	$-8,r5,r1\n\
	movb	r1,r0\n\
	movzwl	r0,r0\n" },

	{ "1,_htonl\n",
"	movl	(sp)+,r5\n\
	rotl	$-8,r5,r0\n\
	insv	r0,$16,$8,r0\n\
	rotl	$8,r5,r1\n\
	movb	r1,r0\n" },

	{ "1,_ntohl\n",
"	movl	(sp)+,r5\n\
	rotl	$-8,r5,r0\n\
	insv	r0,$16,$8,r0\n\
	rotl	$8,r5,r1\n\
	movb	r1,r0\n" },

	{ "2,__insque\n",
"	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	insque	(r4),(r5)\n" },

	{ "1,__remque\n",
"	movl	(sp)+,r5\n\
	remque	(r5),r0\n" },

	{ "2,__queue\n",
"	movl	(sp)+,r0\n\
	movl	(sp)+,r1\n\
	insque	(r1),*4(r0)\n" },

	{ "1,__dequeue\n",
"	movl	(sp)+,r0\n\
	remque	*(r0),r0\n" },

	{ "2,_imin\n",
"	movl	(sp)+,r0\n\
	movl	(sp)+,r5\n\
	cmpl	r0,r5\n\
	bleq	1f\n\
	movl	r5,r0\n\
1:\n" },

	{ "2,_imax\n",
"	movl	(sp)+,r0\n\
	movl	(sp)+,r5\n\
	cmpl	r0,r5\n\
	bgeq	1f\n\
	movl	r5,r0\n\
1:\n" },

	{ "2,_min\n",
"	movl	(sp)+,r0\n\
	movl	(sp)+,r5\n\
	cmpl	r0,r5\n\
	blequ	1f\n\
	movl	r5,r0\n\
1:\n" },

	{ "2,_max\n",
"	movl	(sp)+,r0\n\
	movl	(sp)+,r5\n\
	cmpl	r0,r5\n\
	bgequ	1f\n\
	movl	r5,r0\n\
1:\n" },
#endif vax

#ifdef mc68000
/* someday... */
#endif mc68000

	{ "", "" }
};
