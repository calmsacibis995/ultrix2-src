#ifndef lint
static	char	*sccsid = "@(#)cslocks.c	1.1	(ULTRIX)	1/8/85";
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
# include	<access.h>
# include	<lock.h>


struct lockreq	Lock;
/*
 *	setcsl- set a critical section lock
 */
setcsl(rtid)
long	rtid;
{
	register char	*r;
	register int	i;
	auto	int	ret_val;		/* value returned from the lock driver */

#	ifdef xATR1
	if ( tTf(28,0) )
	{
		printf(" setcsl ");
		dumptid(&rtid);
	}
#	endif

	if (Alockdes < 0)
		return (1);
	Lock.lract = A_SLP;	/* sleep while waiting on lock */
	Lock.lrtype = T_CS;	/* critical section lock */
	Lock.lrmod = M_EXCL;	/* exclusive access */
	bmove(&rtid, Lock.lrel, 4);	/* copy relid */ 
	r = Lock.lpage;
	for (i = 0; i < 4; i++)
			/* zero out pageid */
		*r++ = 0;
	i = write(Alockdes, &Lock, KEYSIZE+3);
	read(Alockdes,&ret_val, sizeof (int));
	return (ret_val);
}



/*
 *	unlcs- unlock a critical section
 */
unlcs(rtid)
long	rtid;
{
	register char	*r;
	register int	i;
	auto	int	ret_val;

#	ifdef xATR1
	if (tTf(28, 1))
	{
		printf(" unlcs ");
		dumptid(rtid);
	}
#	endif

	if (Alockdes < 0)
		return (1);
	Lock.lract = A_RLS1;
	Lock.lrtype = T_CS;
	bmove(&rtid, Lock.lrel, 4);	/* copy relation identifier */
	r = Lock.lpage;
	for (i = 0; i < 4; i++)
			/* zero out page id */
		*r++ = 0;
	i = write(Alockdes, &Lock, KEYSIZE + 3);
	read(Alockdes,&ret_val, sizeof (int) );
	return (ret_val);
}
