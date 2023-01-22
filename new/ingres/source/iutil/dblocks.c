#ifndef lint
static	char	*sccsid = "@(#)dblocks.c	1.1	(ULTRIX)	1/8/85";
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


struct	lockreq	Lock;
/*
 *	setdbl - set data base lock for either exclusive or shared
 *		 access.
 */
setdbl(act, mod)
char	act;			/* type request */
char	mod;			/* mod of lock: = 1 for exclusive, = 2 for shared*/
{
	register char	*r;
	register int	i;
	auto	int	ret_val;

#	ifdef xATR1
	if ( tTf(28,6) )
		printf(" setdbl act=%o md=%o\n", act, mod);
#	endif
	if (Alockdes < 0)
		return (1);
	Lock.lract = act;		/* type of request */
	Lock.lrtype = T_DB;		/* data base lock */
	Lock.lrmod = mod;		/* exclusive or shared */
					/* zero out rest of key */
	r = Lock.lrel;
	for (i = 0; i < 8; i++)
		*r++ = 0;
	i = write(Alockdes, &Lock, KEYSIZE + 3);
	read(Alockdes, &ret_val, sizeof (int));
	return (ret_val);
}
/*
 *	unldb	- release the data base lock
 */
unldb()
{
	register char	*r;
	register int	i;
	auto	int	ret_val;

#	ifdef xATR1
	if (tTf(28, 7))
		printf(" unldb\n");
#	endif
	if (Alockdes < 0)
		return (1);
	Lock.lract = A_RLS1;		/* release 1 lock */
	Lock.lrtype = T_DB;		/* a data base lock*/
	r = Lock.lrel;
	for (i = 0; i < 8; i++)		/* zero out part of key*/
		*r++ = 0;
	i = write(Alockdes, &Lock, KEYSIZE + 3);
	read(Alockdes, &ret_val, sizeof (int));
	return (ret_val);
}
