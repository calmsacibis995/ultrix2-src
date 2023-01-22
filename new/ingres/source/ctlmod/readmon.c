#ifndef lint
static	char	*sccsid = "@(#)readmon.c	1.1	(ULTRIX)	1/8/85";
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
**  READMON -- Read bytes from tty monitor
**
**	This routine is a cludge.  It will exist only in version
**	6.3 with split monitor/parser.  The purpose is to get
**	around the problem caused by the monitor not outputting
**	text in one of the basic internal data types.  What it
**	can do is to output no parameters & terminate with a
**	PV_EOF.  This doesn't flush the pipe, so it can continue
**	with plain text.  This routine retrieves that text.
**	The name "readmon" is somewhat of a misnomer -- it is
**	not restricted to the monitor, & does not necessarily
**	read from the monitor; but this routine should only
**	be called from the parser on the monitor pipe.
**
**	Parameters:
**		buf -- a buffer to read into.
**		nbytes -- the max number of bytes to read.
**
**	Returns:
**		The actual number of bytes read.
**		zero on eof.
**
**	Side Effects:
**		none.
**
**	NOTE:
**		When this routine is eliminated, so should the
**		'cm_monppb' field of the Cm struct and the code
**		in proc_err to flush the monitor input pipe.
**
**	Trace Flags:
**		none.
*/

pb_t	*MonPpb;	/* pointer to ppb for monitor */

readmon(buf, nbytes)
char	*buf;
int	nbytes;
{
	Cm.cm_input = Cm.cm_rinput;
	return (pb_get(MonPpb, buf, nbytes));
}
