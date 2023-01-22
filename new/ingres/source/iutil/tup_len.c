#ifndef lint
static	char	*sccsid = "@(#)tup_len.c	1.1	(ULTRIX)	1/8/85";
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
**	Tup_len finds the amount of space occupied by the
**	tuple specified by "tid"
*/

tup_len(tid)
TID	*tid;
{
	register short	*lp;
	register int	nextoff, off;
	int		lineoff, i;


	/* point to line number table */
	lp = (short *) Acc_head->linetab;

	/* find offset for tid */
	lineoff = lp[-(tid->line_id & I1MASK)];

	/* assume next line number follows lineoff */
	nextoff = lp[-Acc_head->nxtlino];

	/* look for the line offset following lineoff */
	for (i = 0; i < Acc_head->nxtlino; i++)
	{
		off = *lp--;

		if (off <= lineoff)
			continue;

		if (off < nextoff)
			nextoff = off;
	}
#	ifdef xATR3
	if (tTf(27, 8))
		printf("tup_len ret %d\n", nextoff - lineoff);
#	endif
	return (nextoff - lineoff);
}
