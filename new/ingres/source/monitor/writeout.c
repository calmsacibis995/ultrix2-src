#ifndef lint
static	char	*sccsid = "@(#)writeout.c	1.1	(ULTRIX)	1/8/85";
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
# include	<ingres.h>
# include	<aux.h>




/*
**  WRITE OUT QUERY BUFFER TO UNIX FILE
**
**	The logical buffer is written to a UNIX file, the name of which
**	must follow the \w command.
**
**	Uses trace flag 18
*/

writeout()
{
	register int	i;
	register char	*file;
	register int	source;
	int		dest;
	char		buf[512];
	extern char	*getfilenm();

	file = getfilenm();
	if (file[0] == 0 || file[0] == '-')
	{
		printf("Bad file name \"%s\"\n", file);
		return;
	}

	if ((dest = creat(file, 0644)) < 0)
	{
		printf("Cannot create \"%s\"\n", file);
		return;
	}

	if (!Nautoclear)
		Autoclear = 1;

	if ((source = open(Qbname, 0)) < 0)
		syserr("writeout: open(%s)\n", Qbname);

	fflush(Qryiop);

	while ((i = read(source, buf, sizeof buf)) > 0)
		write(dest, buf, i);

	close(source);
	close(dest);
}
