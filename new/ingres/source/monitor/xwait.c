#ifndef lint
static	char	*sccsid = "@(#)xwait.c	1.1	(ULTRIX)	1/8/85";
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
**  WAIT FOR PROCESS TO DIE
**
**	Used in edit() and shell() to terminate the subprocess.
**	Waits on pid "Xwaitpid".  If this is zero, xwait() returns
**	immediately.
**
**	This is called by "catchsig()" so as to properly terminate
**	on a rubout while in one of the subsystems.
**
**	Trace Flags:
**		41
*/

xwait()
{
	int		status;
	register int	i;
	register char	c;

#	ifdef xMTR2
	if (tTf(41, 0))
		printf("xwait: [%d]\n", Xwaitpid);
#	endif
	if (Xwaitpid == 0)
	{
		cgprompt();
		return;
	}
	while ((i = wait(&status)) != -1)
	{
#		ifdef xMTR2
		if (tTf(41, 1))
			printf("pid %d stat %d\n", i, status);
#		endif
		if (i == Xwaitpid)
			break;
	}

	Xwaitpid = 0;

	/* reopen query buffer */
	if ((Qryiop = fopen(Qbname, "a")) == NULL)
		syserr("xwait: open %s", Qbname);
	Notnull = 1;

	cgprompt();
}
