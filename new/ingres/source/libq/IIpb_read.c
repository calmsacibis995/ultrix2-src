#ifndef lint
static	char	*sccsid = "@(#)IIpb_read.c	1.1	(ULTRIX)	1/8/85";
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
# include	"IIglobals.h"



/*
**  IIPB_READ -- read a pipe block
**
**	This routine reads a pipe block into *ppb.  It also handles
**	all the low level protocol: RESET blocks, SYNC blocks, META
**	blocks, blocks intended for another process, etc.
**
**	When this routine returns, it returns with a block intended
**	for this process, which must be a REGULAR block, a RESPONSE
**	block, or an ERROR block.
**
**	Parameters:
**		ppb -- a pointer to the area which wants the block.
**
**	Returns:
**		none
**
**	Side Effects:
**		ppb is set to the named block.
**		Virtually any amount of stuff can happen, depending
**			on what is coming through the pipe.
**
**	Trace Flags:
**		12.4 - 12.9
*/


IIpb_read(ppb)
register pb_t	*ppb;
{
	register int	type;
	register int	from;

	/*
	**  Top Loop.
	**	Hang waiting for a normal block.  Other blocks are
	**		handled inside the loop.
	**	We multiplex 'from' in here temporarily.
	*/

	for (;;)
	{
		from = IIpb_rphys(ppb, IIinput);

		if (from != PB_IOSIZE)
			IIsyserr("pb_read: read error (%d)", from);

		/* set up buffer pointers, etc. */
		ppb->pb_xptr = ppb->pb_data;
		ppb->pb_nleft = ppb->pb_nused;
		type = ppb->pb_type;
		from = ppb->pb_from;

		/* do sync block processing */
		if (type == PB_SYNC)
		{
			IISyncs[from]--;
			continue;
		}

		/* see if we are ignoring from this process */
		if (IISyncs[from] > 0)
			continue;

		/*
		**  Block type dispatching.
		**	Regular, response, and error blocks return.
		**	Meta blocks are handled by calling other
		**		routines.
		*/

		switch (type)
		{
		  case PB_REG:
		  case PB_RESP:
		  case PB_ERR:
			/* handled by readinput() */
			return;

		  /* insert more meta handling before this line */

		  default:
			IIsyserr("pb_read: type %d", type);
		}
	}
}
