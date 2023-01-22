/*
 * machdep.c
 */

#ifndef lint
static char	*sccsid = "@(#)machdep.c	1.4	Ultrix	12/7/84";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984 by				*
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
/*	machdep.c	6.1	83/07/29	*/

#include "../h/param.h"

#include "../vax/mtpr.h"
#include "../vax/clock.h"
#include "../vax/cpu.h"

/*ARGSUSED*/
/*VARARGS1*/
mtpr(regno, value)
{

	asm("	mtpr	8(ap),4(ap)");
}

/*ARGSUSED*/
mfpr(regno)
{

	asm("	mfpr	4(ap),r0");
#ifdef lint
	return (0);
#endif
}

/*
 * Copy bytes within kernel
 */
/*ARGSUSED*/
bcopy(from, to, count)
	caddr_t from, to;
	unsigned count;
{

	asm("	movc3	12(ap),*4(ap),*8(ap)");
}

/*
 * delay for n microseconds, limited to somewhat over 2000 seconds
 */

microdelay(n)
register int n;
{
	union cpusid sid;
	int cpu;

	sid.cpusid = mfpr(SID);
	cpu = sid.cpuany.cp_type;
	if( cpu == MVAX_I || cpu == MVAX_II ) {
		n /= 4;
		while (n-- > 0)		/* sorry, no ICR */
			;
	} else {
		if ( !(mfpr(ICCS) & ICCS_RUN)) {  /* is the clock enabled */
			mtpr(NICR, -n);		/* load neg n */
			mtpr(ICCS, ICCS_RUN+ICCS_TRANS+ICCS_INT+ICCS_ERR);
			while ( !(mfpr(ICCS) & ICCS_INT));	/* wait */
			mtpr(ICCS, ICCS_INT+ICCS_ERR);	/* turn clock off */
		}
	}
}	
