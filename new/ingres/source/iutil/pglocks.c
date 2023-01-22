#ifndef lint
static	char	*sccsid = "@(#)pglocks.c	1.1	(ULTRIX)	1/8/85";
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
 *	setpgl- sets a lock for the access buffer
 */
setpgl(buf)
struct	accbuf	*buf;
{
	register struct accbuf	*b;
	register int		i;
	auto	int	ret_val;

#	ifdef xATR1
	if ( tTf(28,2) )
	{
		printf(" setpgl pg=%ld rel", buf->thispage);
		dumptid(&buf->rel_tupid);
	}
#	endif
	if (Alockdes < 0)
		return(1);
	b = buf;
	Lock.lract = A_SLP;	/* wait for lock */
	Lock.lrtype = T_PAGE;	/* page lock */
	Lock.lrmod = M_EXCL;	/* exclusive lock */
	bmove(&b->rel_tupid, Lock.lrel, 4);	/* copy relation id */
	bmove(&b->thispage, Lock.lpage, 4);	/* copy page id */
	i = write(Alockdes,  &Lock,  KEYSIZE + 3);
	read(Alockdes, &ret_val, sizeof (int));
	b->bufstatus |= BUF_LOCKED;
	return (ret_val);
}
/*
 *	unlpg- releases a page lock
 */
unlpg(buf)
struct	accbuf	*buf;
{
	register struct	accbuf	*b;
	register int		i;
	auto	int	ret_val;

#	ifdef xATR1
	if (tTf(28, 3))
	{
		printf(" unlpg page %ld rel", buf->thispage);
		dumptid(&buf->rel_tupid);
	}
#	endif
	if (Alockdes < 0)
		return(1);
	b = buf;
	Lock.lract = A_RLS1;
	bmove(&b->rel_tupid, Lock.lrel, 4);	/* copy relation id */
	Lock.lrtype = T_PAGE;	/* page lock */
	bmove(&b->thispage, Lock.lpage, 4);	/* copy page id */
	b->bufstatus &= ~BUF_LOCKED;
	i = write(Alockdes,  &Lock,  KEYSIZE + 3);
	read(Alockdes, &ret_val, sizeof (int));
	return (ret_val);
}
/*
 *	unlall - release all locks held by this process
 */
unlall()
{
	register int	i;
	auto	int	ret_val;


#	ifdef xATR1
	if (tTf(28, 6))
		printf(" unlall\n");
#	endif

	Acclock = TRUE;	/* reset page lock flag just in case */
	if (Alockdes < 0)
		return(1);
	Lock.lract = A_RLSA;
	i = write(Alockdes, &Lock, KEYSIZE + 3);
	read(Alockdes, &ret_val, sizeof (int));
	return (ret_val);
}
