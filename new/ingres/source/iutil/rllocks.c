#ifndef lint
static	char	*sccsid = "@(#)rllocks.c	1.1	(ULTRIX)	1/8/85";
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

# include	<ingres.h>
# include	<lock.h>
# include	<signal.h>


struct lockreq	Lock;
/*
/*
 *	setrll- set a relation lock
 */
setrll(act, rtid, mod)
char	act;
long	rtid;
char	mod;
{
	register char	*r;
	register int	i;
	auto	int	ret_val;

#	ifdef xATR1
	if ( tTf(28,4) )
	{
		printf(" setrll act=%d md=%o ", act, mod);
		dumptid(&rtid);
	}
#	endif
	if (Alockdes < 0)
		return(1);
	Lock.lract = act;	/* sleep (act = 2) or error return (act = 1)*/
	Lock.lrtype = T_REL;	/* relation lock */
	Lock.lrmod = mod;	/* exclusive (mod = 1) or shared (mod = 2)*/
	bmove(&rtid, Lock.lrel, 4);	/* copy relation id */
	r = Lock.lpage;

	/* zero out page id */
	for (i = 0; i < 4; i++)
		*r++ = 0;

	write(Alockdes, &Lock, KEYSIZE+3);
	read(Alockdes, &ret_val, sizeof (int));
	return (ret_val);
}
/*
 *	unlrel- unlock a relation lock
 */
unlrl(rtid)
long	rtid;
{
	register char	*r;
	register int	i;
	auto	int	ret_val;

#	ifdef xATR1
	if (tTf(28, 5))
	{
		printf(" unlrl ");
		dumptid(&rtid);
	}
#	endif
	if (Alockdes < 0)
		return (1);
	Lock.lract = A_RLS1;
	Lock.lrtype = T_REL;	/* relation lock */
	bmove(&rtid, Lock.lrel, 4);	/* copy relation id */
	r = Lock.lpage;
	for (i = 0; i < 4; i++)
			/* zero out pageid */
		*r++ = 0;
	i = write(Alockdes, &Lock, KEYSIZE + 3);
	read(Alockdes, &ret_val, sizeof (int));
	return (ret_val);
}
