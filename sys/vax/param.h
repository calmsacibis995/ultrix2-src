/*
 *	@(#)param.h	1.8	(ULTRIX)	3/18/87
 */

/************************************************************************
 *									*
 *			Copyright (c) 1983,86 by			*
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
 * 12-Feb-86 -- jrs
 *	Changed BASEPRI defn to handle new idle loop
 *
 *	Derived from 4.2 BSD labelled:
 *		param.h	6.1	83/07/29
 *
 *-----------------------------------------------------------------------
 */

/*
 * Machine dependent constants for vax.
 */
#define	NBPG	512		/* bytes/page */
#define	PGOFSET	(NBPG-1)	/* byte offset into page */
#define	PGSHIFT	9		/* LOG2(NBPG) */

#define	CLSIZE		2
#define	CLSIZELOG2	1

#define	SSIZE	4		/* initial stack size/NBPG */
#define	SINCR	4		/* increment of stack/NBPG */

#define	UPAGES	14		/* pages of u-area */
#define	NISP	5		/* pages of interrupt stack */

/*
 * Some macros for units conversion
 */
/* Core clicks (512 bytes) to segments and vice versa */
#define	ctos(x)	(x)
#define	stoc(x)	(x)

/* Core clicks (512 bytes) to disk blocks */
#define	ctod(x)	(x)
#define	dtoc(x)	(x)
#define	dtob(x)	((x)<<9)

/* clicks to bytes */
#define	ctob(x)	((x)<<9)

/* bytes to clicks */
#define	btoc(x)	((((unsigned)(x)+511)>>9))

/*
 * Macros to decode processor status word.
 */
#define	USERMODE(ps)	(((ps) & PSL_CURMOD) == PSL_CURMOD)
#define	BASEPRI(ps)	(((ps) & PSL_IPL) <= PSL_IPL_LOW)

#define DELAY(n)	{ microdelay(n); }
