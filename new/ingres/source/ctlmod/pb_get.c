#ifndef lint
static	char	*sccsid = "@(#)pb_get.c	1.1	(ULTRIX)	1/8/85";
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

# include	"ctlmod.h"
# include	"pipes.h"


/*
**  PB_GET -- buffered get from pipe
**
**	This routine just gets a record from the pipe block, reading
**	if necessary.  It tries to do things in big chunks to keep
**	the overhead down.
**
**	Parameters:
**		ppb -- a pointer to the pipe block to unbuffer.
**		dp -- a pointer to the data area.
**		len -- the number of bytes to read.
**
**	Returns:
**		The number of bytes actually read.
**		Zero on end of file.
**
**	Side Effects:
**		none.
**
**	Trace Flags:
**		18.1 - 18.7
*/

pb_get(ppb, dp, len)
register pb_t	*ppb;
char		*dp;
register int	len;
{
	int		rct;
	register int	i;

	rct = 0;
# ifdef xCTR2
	if (tTf(18, 1))
		lprintf("pb_get: ");
# endif

	/*
	**  Top Loop.
	**	As long as we still want more, keep buffering out.
	*/

	while (len > 0)
	{
		/* if we have no data, read another block */
		while (ppb->pb_nleft <= 0)
		{
			if (bitset(PB_EOF, ppb->pb_stat))
			{
# ifdef xCTR2
				if (tTf(18, 1))
					printf("EOF\n");
# endif
				if (rct != 0)
					syserr("pb_get: EOF");
				return (rct);
			}
			pb_read(ppb);
		}

		/*
		**  Compute the length to move.
		**	This is the min of the amount we want and the
		**	amount we have available to us in the buffer.
		*/

		i = min(ppb->pb_nleft, len);
		bmove(ppb->pb_xptr, dp, i);
		ppb->pb_xptr += i;
		ppb->pb_nleft -= i;
		dp += i;
		len -= i;
		rct += i;
	}

# ifdef xCTR2
	if (tTf(18, 1))
		printf("%d\n", rct);
# endif
	return (rct);
}
