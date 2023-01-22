#ifndef lint
static	char	*sccsid = "@(#)pb_dump.c	1.1	(ULTRIX)	1/8/85";
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

# include	"pipes.h"


/*
**  PB_DUMP -- dump pipe buffer for debugging
**
**	Parameters:
**		ppb -- pointer to the structure to dump.
**		full -- if set, dump everything, else just
**			dump the header.
**
**	Returns:
**		none
**
**	Side Effects:
**		none
**
**	Trace Flags:
**		none
*/

pb_dump(ppb, full)
register pb_t	*ppb;
int		full;
{
	printf("PB @ %x:\n", ppb);
	printf("\t    st %4d  proc %4d  resp %4d  from %4d\n",
	    ppb->pb_st, ppb->pb_proc, ppb->pb_resp, ppb->pb_from);
	printf("\t    type %2d  stat %4o  nused %3d  nleft %3d",
	    ppb->pb_type, ppb->pb_stat, ppb->pb_nused, ppb->pb_nleft);
	if (full)
	{
		register int	i;
		register char	*p;
		register char	c;

		p = ppb->pb_data;
		for (i = 0; i < ppb->pb_nused; i++)
		{
			c = *p++;
			if (i % 10 == 0)
				printf("\n\t%3d:", i);
			putchar(' ');
			if (c >= 040 && c < 0177)
				printf(" %c  ", c);
			else
				xputchar(c);
		}
	}
	putchar('\n');
}
