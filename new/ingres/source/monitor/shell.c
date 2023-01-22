#ifndef lint
static	char	*sccsid = "@(#)shell.c	1.1	(ULTRIX)	1/8/85";
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
# include	<opsys.h>




/*
**  SHELL -- call unix shell
**
**	The UNIX shell is called.  Shell() first tries to call an
**	alternate shell defined by the macro {shell}, and if it fails
**	calls /bin/sh.
**
**	If an argument is supplied, it is a shell file which is
**	supplied to the shell.
**
**	Trace Flags:
**		34
*/

shell()
{
	register int	i;
	register char	*p;
	register char	*shellfile;
	extern char	*getfilenm();
	extern char	*macro();

	shellfile = getfilenm();
	if (*shellfile == 0)
		shellfile = 0;

	fclose(Qryiop);
	if ((Xwaitpid = fork()) == -1)
		syserr("shell: fork");
	if (Xwaitpid == 0)
	{
		setuid(getuid());
#		ifndef xB_UNIX
		setgid(getgid());
#		endif
		for (i = 3; i < NOFILE; i++)
			close(i);
		p = macro("{shell}");
#		ifdef xMTR3
		tTfp(34, 0, "{shell} = '%o'\n", p);
#		endif
		if (p != 0)
		{
			execl(p, p, shellfile, Qbname, 0);
			printf("Cannot call %s; using /bin/sh\n", p);
		}
		execl("/bin/sh", "sh", shellfile, Qbname, 0);
		syserr("shell: exec");
	}

	if (Nodayfile >= 0)
		printf(">>shell\n");
	/* wait for shell to complete */
	xwait();
}
