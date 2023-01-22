#ifndef lint
static	char	*sccsid = "@(#)trapquery.c	1.1	(ULTRIX)	1/8/85";
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

# include	"monitor.h"
# include	<trace.h>
# include	<resp.h>





/*
**  TRAPQUERY -- trap queries which succeeded.
**
**	This module traps the current query into the file
**	specified by Trapfile.  It will open Trapfile if
**	it is not already open.
**
**	Parameters:
**		retcode -- the return code of the query.
**		name -- the name of the file in which to dump the
**			query buffer.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Outputs the query buffer to Trapfile.
**
**	Called By:
**		go.
**
**	Files:
**		Trapfile -- the descriptor of the file in which to
**			trap the query.
**		Qbname -- the name of the query buffer.
**
**	Trace Flags:
**		none
**
**	Bugs:
**		If 'name' (that is, the {querytrap} macro) changes
**			during a run, the output file does not change.
*/

FILE	*Trapfile;

trapquery(resp, name)
struct resp	*resp;
char		*name;
{
	register FILE	*iop;
	static int	first;
	register char	*sp, c;
	int		timevec[2];
	extern		fgetc();
	extern char	*ctime();

	if (first < 0)
		return;
	if (Trapfile == NULL)
	{
		if ((Trapfile = fopen(name, "a")) == NULL)
		{
			printf("can't trap query in %s\n", name);
			first = -1;
			return;
		}
	}
	if (first == 0)
	{
		time(timevec);
		sp = ctime(timevec);
		while (*sp)
			putc(*sp++, Trapfile);
		first++;
	}

	if ((iop = fopen(Qbname, "r")) == NULL)
		syserr("go: open 1");
	macinit(fgetc, iop, 1);

	while ((c = macgetch()) > 0)
		putc(c, Trapfile);

	if (resp->resp_resp == 0)
	{
		sp = (char *) locv(resp->resp_tups);
		while (*sp)
			putc(*sp++, Trapfile);
		putc('\n', Trapfile);
	}

	fclose(iop);
}
