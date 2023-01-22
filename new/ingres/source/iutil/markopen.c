#ifndef lint
static	char	*sccsid = "@(#)markopen.c	1.1	(ULTRIX)	1/8/85";
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
# include	<opsys.h>


/*
**  MARKOPEN -- mark all open files
**
**	Marked files will not be closed later.
**
**	Parameters:
**		ovect -- pointer to bitmap of open files.
**
**	Returns:
**		none
**
**	Side Effects:
**		Sets *ovect to represent the open files.
*/

long	CmOfiles;	/* default set of files, used all over */

markopen(ovect)
register long	*ovect;
{
	register int	i;
	register int	j;
	extern int	errno;
	struct stat	sbuf;

	if (ovect == NULL)
		ovect = &CmOfiles;

	*ovect = 0;
	for (i = 0; i < NOFILE; i++)
	{
		if (fstat(i, &sbuf) >= 0)
			*ovect |= 1 << i;
	}
	errno = 0;
}
/*
**  CLOSEALL -- close all open files (except marked files)
**
**	Parameters:
**		tell -- if set, report files that are open and should
**			not have been.
**		ovect -- vector of files to leave open.
**
**	Returns:
**		none
**
**	Side Effects:
**		none
**
**	Trace Flags:
**		none
*/

closeall(tell, ovect)
register int	tell;
register long	ovect;
{
	register int	i;

	ovect |= CmOfiles;

	for (i = 0; i < NOFILE; i++)
	{
		if (!bitset(1 << i, ovect))
			if (close(i) >= 0 && tell)
				lprintf("File %d open\n", i);
	}
}
