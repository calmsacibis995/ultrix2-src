#ifndef lint
static	char	*sccsid = "@(#)do_u_flag.c	1.1	(ULTRIX)	1/8/85";
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

# include	<ingres.h>
# include	<aux.h>


/*
**  DO_U_FLAG -- Get User Code from User Code or Name
**
**	This routine is the "-u" flag processing.
**
**	Parameters:
**		u_flag -- a pointer to the -u flag.
**
**	Returns:
**		zero -- user not found (message also printed).
**		else -- pointer to the user code for the override user.
**
**	Side Effects:
**		The actual text of the -u flag may be trashed with the
**			new user code.
*/

char *
do_u_flag(u_flag)
char	*u_flag;
{
	register char	*p;
	char		buf[MAXLINE + 1];

	p = u_flag;

	if (getnuser(&p[2], buf) != 0)
	{
		if (p[2] == 0 || p[3] == 0 || p[4] != 0 ||
		    getuser(&p[2], buf) != 0)
		{
			getuser(0);
			printf("User %s does not exist\n", &p[2]);
			return (0);
		}
		getuser(0);
		return (&p[2]);
	}
	smove(getufield(buf, 1), p);
	return (p);
}
