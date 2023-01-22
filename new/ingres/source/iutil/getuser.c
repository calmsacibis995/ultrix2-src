#ifndef lint
static	char	*sccsid = "@(#)getuser.c	1.1	(ULTRIX)	1/8/85";
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
**  GET LINE FROM USER FILE
**
**	Given a user code (a two byte string), this routine returns
**	the line from .../files/users into `buf'.  The users
**	file is automatically opened, and it is closed if getperm
**	is called with `code' == 0.
**
**	If `code' == -1 then getuser will reinitialize itself.
**	This will guarantee that getuser will reopen the file
**	if (for example) an interrupt occured during the previous
**	call.
*/

getuser(code, buf)
char	*code;
char	buf[];
{
	static FILE	*userf;
	register char	c;
	register char	*bp;
	extern char	*index();
	
	if (code == 0)
	{
		if (userf != NULL)
			fclose(userf);
		userf = NULL;
		return (0);
	}
	if (code == (char *) -1)
	{
		userf = NULL;
		return (0);
	}
	if (userf == NULL)
	{
		userf = fopen(ztack(Pathname, "/files/users"), "r");
		if (userf == NULL)
			syserr("getuser: open err");
	}
	rewind(userf);
	
	for (;;)
	{
		bp = buf;
		if (fgets(bp, MAXLINE, userf) == NULL)
			return (1);
		*index(bp, '\n') = '\0';
		while ((c = *bp++) != ':')
			if (c == '\0')
				return (1);
		if (bequal(bp, code, 2))
			return (0);
	}
}
