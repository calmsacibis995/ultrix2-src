#ifndef lint
static	char	*sccsid = "@(#)IIpb_write.c	1.1	(ULTRIX)	1/8/85";
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
**  IIPB_WRITE -- Write pipe block
**
**	Writes the block specified by ppb down to the PARSER
**
**	Parameters:
**		ppb -- a ptr to the pipe block to write.
**
**	Returns:
**		none
**
**	Side Effects:
**		none
*/

IIpb_write(ppb)
register pb_t	*ppb;
{
	register int	i;
	register int	ofd;

	/* mark the block as coming from this process */
	ppb->pb_from = PB_FRONT;

	/* normal message */
	i = ppb->pb_proc;
	IIpb_wphys(ppb, IIw_down);


	/* reset some exciting pointers */
	ppb->pb_xptr = ppb->pb_data;
	ppb->pb_nleft = PB_DBSIZE;
	ppb->pb_nused = 0;
}
