#ifndef lint
static	char	*sccsid = "@(#)IIpb_put.c	1.1	(ULTRIX)	1/8/85";
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

# include	<useful.h>
# include	<pipes.h>



/*
**  IIPB_PUT -- buffered put on pipe
**
**	This routine puts the named data out onto the pipe
**	determined by ppb->pb_proc.
**
**	Parameters:
**		dp -- a pointer to the data to write.
**		len -- the length of the data to write.
**		ppb -- a pointer to the pipe block.
**
**	Returns:
**		none
**
**	Side Effects:
**		none
**
**	Trace Flags:
**		18.8 - 18.15
*/

IIpb_put(dp, len, ppb)
register char	*dp;
register int	len;
register pb_t	*ppb;
{
	register int	i;


	/*
	**  Top loop.
	**	Loop until we have run out of things to write.
	*/

	while (len > 0)
	{
		/* compute the length to move */
		i = min(ppb->pb_nleft, len);

		/* move data into buffer and adjust ptrs & counts */
		IIbmove(dp, ppb->pb_xptr, i);
		dp += i;
		len -= i;
		ppb->pb_xptr += i;
		ppb->pb_nleft -= i;
		ppb->pb_nused += i;

		/* flush block if full */
		if (ppb->pb_nleft <= 0)
			IIpb_write(ppb);
	}
}
