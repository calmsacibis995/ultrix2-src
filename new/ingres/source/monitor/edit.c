#ifndef lint
static	char	*sccsid = "@(#)edit.c	1.1	(ULTRIX)	1/8/85";
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
**  CALL TEXT EDITOR
**
**	The UNIX text editor is called.  The actual call is to
**	the macro {editor}.  If that fails, /bin/ed is called.
**	This routine suppressed the autoclear function.
**
**	Uses trace flag 4
*/

edit()
{
	register int	i;
	register char	*p;
	register char	*editfile;
	extern char	*getfilenm(), *macro();

	editfile = getfilenm();
	if (*editfile == 0)
		editfile = Qbname;

	Autoclear = 0;
	fclose(Qryiop);

	/* FORK SENTRY PROCESS & INVOKE THE EDITOR */
	if ((Xwaitpid = fork()) < 0)
		syserr("edit: fork");
	if (Xwaitpid == 0)
	{
		setuid(getuid());
#		ifndef xB_UNIX
		setgid(getgid());
#		endif
		for (i = 3; i < NOFILE; i++)
			close(i);
		p = macro("{editor}");
		if (p != 0)
		{
			execl(p, p, editfile, 0);
			printf("Cannot call %s; using /bin/ed\n", p);
		}
		execl("/bin/ed", "ed", editfile, 0);
		syserr("edit: exec");
	}

	/* WAIT FOR SENTRY TO DIE */
	if (Nodayfile >= 0)
		printf(">>ed\n");
	xwait();
}
