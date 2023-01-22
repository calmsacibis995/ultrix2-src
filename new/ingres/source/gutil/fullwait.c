#ifndef lint
static	char	*sccsid = "@(#)fullwait.c	1.1	(ULTRIX)	1/8/85";
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

# include	<errno.h>


/*
**  FULLWAIT -- performs a wait(II) primitive for a given child.
**
**	The wait primitive is executed as many times as needed to get
**	termination status for a given child.  The case of an interrupt
**	during the wait is handled.  No other children may die during
**	the wait.  Also, the child must die "normally", that is, as the
**	result of an exit() system call, or from an interrupt.
**
**	Parameters:
**		"child" -- the pid of the child process to wait for,
**			returned from the fork() system call.
**		"name" -- a character string representing the name of
**			the calling routine; printed on syserr's.
**	Returns:
**		The exit parameter of the child process.
*/

fullwait(child, name)
int	child;
char	*name;
{
	auto int	st;
	register int	i;
	extern int	errno;
	register char	*n;
	register char	*coredump;

	n = name;

	/* wait for a child to die */
	while ((i = wait(&st)) != child)
	{
		/* it is not the child we want; chew on it a little more */
		if (i != -1)
			syserr("%s: bad child: pid %d st 0%o", n, i, st);

		/* check for interrupted system call */
		if (errno != EINTR)
		{
			/* wait with no child */
			syserr("%s: no child", n);
		}
		errno = 0;

		/* dropped out from signal: reexecute the wait */
	}

	/* check termination status */
	i = st & 0377;
	if (i > 2)
	{
		/* child collapsed */
		if (i & 0200)
		{
			coredump = " -- core dumped";
			i &= 0177;
		}
		else
		{
			coredump = "";
		}
		syserr("%s: stat %d%s", n, i, coredump);
	}

	/* return exit status */
	return (st >> 8);
}
