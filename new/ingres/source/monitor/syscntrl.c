#ifndef lint
static	char	*sccsid = "@(#)syscntrl.c	1.1	(ULTRIX)	1/8/85";
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

# include "monitor.h"
# include <func.h>
# include <pipes.h>




/*
**  TRACE -- set/clear trace information dynamically.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Sends the rest of the input line to all processes
**		and calls tTamper on the rest of the input line.
**
**	Trace Flags:
**		32
*/

trace()
{
	pb_t	pb;
	char	buf[120];

	/* get rest of trace command */
	pb_prime(&pb, PB_TRACE);
	pb.pb_proc = PB_WILD;
	if (fgets(buf, sizeof buf, Input) == NULL)
		syserr("syscntrl: bad read");
	Prompt = TRUE;
	tTamper(buf, FuncVect[0]->fn_tflag, FuncVect[0]->fn_tvect, FuncVect[0]->fn_tsize);
	pb_put(buf, length(buf), &pb);

	pb.pb_stat |= PB_INFO;
	pb_flush(&pb);
}
/*
**  RESET -- do a system reset.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Sends a reset block to all processes
**		and calls cm_reset in the monitor.
*/

reset()
{
	pb_t 	pb;

	pb_prime(&pb, PB_RESET);
	pb.pb_proc = PB_WILD;
	pb.pb_stat |= PB_INFO;
	pb_flush(&pb);
	cm_reset();
}
