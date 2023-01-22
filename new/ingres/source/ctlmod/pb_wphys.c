#ifndef lint
static	char	*sccsid = "@(#)pb_wphys.c	1.1	(ULTRIX)	1/8/85";
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
# include	"pipes.h"


/*
**  PB_WPHYS -- physical write on pipe
**
**	Parameters:
**		ppb -- a pointer to the data area.
**		fd -- the file descriptor.
**
**	Returns:
**		none
**
**	Side Effects:
**		none
**
**	Called By:
**		pb_write
**
**	Trace Flags:
**		none
*/

pb_wphys(ppb, fd)
register pb_t	*ppb;
register int	fd;
{
	register int	i;

	i = write(fd, (char *) ppb, PB_IOSIZE);
	if (i != PB_IOSIZE)
	{
		pb_dump(ppb, TRUE);
		syserr("pb_wphys: write error: fd=%d, i=%d", fd, i);
	}
}
