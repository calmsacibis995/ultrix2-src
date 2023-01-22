#ifndef lint
static	char	*sccsid = "@(#)IIpb_flush.c	1.1	(ULTRIX)	1/8/85";
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

# include	<pipes.h>



/*
**  IIPB_FLUSH -- flush a pipe buffer
**
**	This routine insures that all the data in a pipe buffer
**	is flushed out to the pipe.
**
**	We also handle input switching in this routine.  If the
**	message we are writing is not merely informational (such
**	as an error message, or some sort of meta message), we
**	change the input to be whatever pipe the named process
**	will write back on.
**
**	Parameters:
**		ppb -- a ptr to the pipe buffer to flush.
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

IIpb_flush(ppb)
register pb_t	*ppb;
{
	/* mark this as an EOF block and flush the buffer */
	ppb->pb_stat |= PB_EOF;
	IIpb_write(ppb);
}
