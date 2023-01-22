#ifndef lint
static	char	*sccsid = "@(#)newlino.c	1.1	(ULTRIX)	1/8/85";
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


/*
** newlino finds a free line number which it returns and adjusts
**	the next line counter (Nxtlino) by the length of the tuple (len).
**	This routine is used to recover unused sections of the
**	line number table (Acc_head->linetab).
*/

newlino(len)
int	len;
{
	register int	newlno, nextlno;
	register short	*lp;

	nextlno = Acc_head->nxtlino;
	lp = &Acc_head->linetab[0];
	for (newlno = 0; newlno < nextlno; newlno++)
	{
		if (*lp == 0)
		{
			/* found a free line number */
			*lp = Acc_head->linetab[-nextlno];
			Acc_head->linetab[-nextlno] += len;
			return (newlno);
		}
		lp--;
	}

	/* no free line numbers. use nxtlino */
	Acc_head->linetab[-(nextlno + 1)] = *lp + len;
	Acc_head->nxtlino++;
	return (nextlno);
}
