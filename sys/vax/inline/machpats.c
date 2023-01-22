
#ifndef lint
static char *sccsid = "@(#)machpats.c	1.4	ULTRIX	10/3/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
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
 *
 *   Modification History:
 *
 * 16 Jul 86 -- Todd M. Katz
 *	Add comments to insqti/remqhi.  When lp checked in my
 *	insqti/remqhi inline patterns he neglected to include comments.
 *	These comments are essential to understanding how to make use of
 *	the insqti/remqhi patterns.
 *
 * 9 Apr 86 -- lp
 *	Added insqti/remqhi.
 *
 * 26 Feb 86 -- depp
 *	Added "movpsl".
 *
 */

#include "inline.h"

/*
 * Pattern table for special VAX instructions.
 */
struct pats machine_ptab[] = {

#ifdef vax
	{ "3,_blkcpy\n",
"	movl	(sp)+,r1\n\
	movl	(sp)+,r3\n\
	jbr	2f\n\
1:\n\
	subl2	r0,(sp)\n\
	movc3	r0,(r1),(r3)\n\
2:\n\
	movzwl	$65535,r0\n\
	cmpl	(sp),r0\n\
	jgtr	1b\n\
	movl	(sp)+,r0\n\
	movc3	r0,(r1),(r3)\n" },

	{ "3,_bcopy\n",
"	movl	(sp)+,r1\n\
	movl	(sp)+,r3\n\
	movl	(sp)+,r5\n\
	movc3	r5,(r1),(r3)\n" },

	{ "3,_ovbcopy\n",
"	movl	(sp)+,r3\n\
	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	movc3	r5,(r3),(r4)\n" },

	{ "3,_blkcmp\n",
"	movl	(sp)+,r1\n\
	movl	(sp)+,r3\n\
	jbr	2f\n\
1:\n\
	subl2	r0,(sp)\n\
	cmpc3	r0,(r1),(r3)\n\
	bneq	3f\n\
2:\n\
	movzwl	$65535,r0\n\
	cmpl	(sp),r0\n\
	jgtr	1b\n\
	movl	(sp)+,r0\n\
	cmpc3	r0,(r1),(r3)\n\
3:\n" },

	{ "3,_bcmp\n",
"	movl	(sp)+,r1\n\
	movl	(sp)+,r3\n\
	movl	(sp)+,r5\n\
	cmpc3	r5,(r1),(r3)\n" },

	{ "2,_blkclr\n",
"	movl	(sp)+,r3\n\
	jbr	2f\n\
1:\n\
	subl2	r0,(sp)\n\
	movc5	$0,(r3),$0,r0,(r3)\n\
2:\n\
	movzwl	$65535,r0\n\
	cmpl	(sp),r0\n\
	jgtr	1b\n\
	movl	(sp)+,r0\n\
	movc5	$0,(r3),$0,r0,(r3)\n" },

	{ "2,_bzero\n",
"	movl	(sp)+,r3\n\
	movl	(sp)+,r5\n\
	movc5	$0,(r3),$0,r5,(r3)\n" },

	{ "3,_llocc\n",
"	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	movl	(sp)+,r1\n\
1:\n\
	movzwl	$65535,r0\n\
	cmpl	r5,r0\n\
	jleq	1f\n\
	subl2	r0,r5\n\
	locc	r4,r0,(r1)\n\
	jeql	1b\n\
	addl2	r5,r0\n\
	jbr	2f\n\
1:\n\
	locc	r4,r5,(r1)\n\
2:\n" },

	{ "3,_locc\n",
"	movl	(sp)+,r3\n\
	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	locc	r3,r4,(r5)\n" },

	{ "4,_scanc\n",
"	movl	(sp)+,r2\n\
	movl	(sp)+,r3\n\
	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	scanc	r2,(r3),(r4),r5\n" },

	{ "3,_skpc\n",
"	movl	(sp)+,r3\n\
	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	skpc	r3,r4,(r5)\n" },

	{ "2,_insque\n",
"	movl	(sp)+,r4\n\
	movl	(sp)+,r5\n\
	insque	(r4),(r5)\n" },

	{ "1,_remque\n",
"	movl	(sp)+,r5\n\
	remque	(r5),r0\n" },

	{ "0,_movpsl\n",
"	movpsl	r0\n" },

/* This pattern matches the following function call:
 *
 *	u_long	retry_count,	- Number attempts to obtain queue interlock
 *		status;		- Status of interlocked queue insertion
 *	struct	qptr	{
 *		u_long	*flink;
 *		u_long	*blink;
 *		}
 *		*pktptr,	- Address of packet's queue pointers
 *		*qhptr;		- Address of head of interlocked queue
 *
 *		.....
 *
 *	status = insqti( pktptr, qhptr, retry_count );
 *
 *	where:
 *		status < 0	- Successfully inserted packet onto queue
 *		status = 0	- Successfully inserted packet onto
 *				   previously empty queue
 *		status > 0	- Retry_count attempts failed to obtain queue
 *				   interlock, packet not inserted onto queue
 */
	{ "3,_insqti\n",
"		movl	(sp)+,r1\n\
		movl	(sp)+,r2\n\
		movl	(sp)+,r3\n\
		mnegl	$1,r0\n\
	0:	insqti	0(r1),0(r2)\n\
		bcc	1f\n\
		sobgeq	r3,0b\n\
		movzbl	$1,r0\n\
	 	brb	2f\n\
	1:	bneq	2f\n\
		clrf	r0\n\
 	2:	nop\n" },

/* This pattern matches the following function call:
 *
 *	u_long	retry_count;	- Number attempts to obtain queue interlock
 *	struct  qptr	{
 *		u_long	*flink;
 *		u_long	*blink;
 *		}
 *		*pktptr,	- System virtual addr of packet's queue ptrs
 *				-  pointers
 *		*qhptr;		- Address of head of interlocked queue
 *
 *		.....
 *
 *	pktptr = ( struct qptr * )remqhi( qhptr, retry_count );
 *
 *	where:
 *		pktptr < 0	- Successfully removed packet from queue
 *		pktptr = 0	- Queue empty, no packet removed
 *		pktptr > 0	- Retry_count attempts failed to obtain queue
 *				   interlock,  packet not removed from queue
 */
	{ "2,_remqhi\n",
"		movl	(sp)+,r1\n\
		movl	(sp)+,r2\n\
	0:	remqhi	0(r1),r0\n\
		bvc	2f\n\
		bcc	1f\n\
		sobgeq	r2,0b\n\
		movzbl	$1,r0\n\
		brb	2f\n\
	1:	clrf	r0\n\
	2:	nop\n" },
#endif vax

#ifdef mc68000
/* someday... */
#endif mc68000

	{ "", "" }
};
