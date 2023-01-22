#ifndef lint
static	char	*sccsid = "@(#)getnuser.c	1.1	(ULTRIX)	1/8/85";
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
# include	<ingres.h>
# include	<aux.h>


/*
**  GETNUSER -- get line from user file based on name
**
**	Given a user name as a string, this routine returns the
**	corresponding line from .../files/users into a buffer.
**
**	Parameters:
**		name -- the name of the user
**		buf -- a buf to dump the line in (declare as
**			char buf[MAXLINE + 1]
**
**	Returns:
**		zero -- success
**		one -- failure (user not present)
**
**	Side effects:
**		none
**
**	Files:
**		.../files/users (readable)
*/

getnuser(name, buf)
char	*name;
char	buf[];
{
	FILE		*userf;
	register char	c;
	register char	*bp;
	
	userf = fopen(ztack(Pathname, "/files/users"), "r");
	if (userf == NULL)
		syserr("getuser: open err");
	
	for (;;)
	{
		bp = buf;
		while ((c = getc(userf)) != '\n')
		{
			if (c == EOF)
			{
				fclose(userf);
				return (1);
			}
			*bp++ = c;
		}
		*bp++ = '\0';
		bp = buf;
		while ((c = *bp++) != ':')
		{
			if (c == '\0')
			{
				fclose(userf);
				return (1);
			}
		}
		*--bp = 0;
		if (sequal(buf, name))
		{
			fclose(userf);
			*bp = ':';
			return (0);
		}
	}
}
