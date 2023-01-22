#ifndef lint
static	char	*sccsid = "@(#)IIsyserr.c	1.1	(ULTRIX)	1/8/85";
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

# include	<stdio.h>



/*
**  SYSERR -- SYStem ERRor message print and abort
**
**	Syserr acts like a printf with up to five arguments.
**
**	If the first argument to syserr is not zero,
**	the message "SYSERR:" is prepended.
**
**	If the extern variable `IIproc_name' is assigned to a
**	string, that string is prepended to the message.
**
**	All arguments must be null-terminated.
**
**	The function pointed to by `Exitfn' is then called.
**	It is initialized to be `exit'.
*/

extern char	*IIproc_name;

IIsyserr(pv)
char	*pv;
{
	int		pid;
	register char	**p;
	extern int	errno;
	register int	usererr;

	p = &pv;
	printf("\n");
	usererr = pv == 0;

	if (!usererr)
	{
		if (IIproc_name)
			printf("%s ", IIproc_name);
		printf("SYSERR: ");
	}
	else
		p++;
	printf(p[0], p[1], p[2], p[3], p[4], p[5]);
	printf("\n");
	if (!usererr && errno)
		printf("\tsystem error %d\n", errno);
	fflush(stdout);
	exit (-1);
}
