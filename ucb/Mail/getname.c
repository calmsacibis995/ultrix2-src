#ifndef lint
static	char	*sccsid = "@(#)getname.c	1.4	(ULTRIX)	12/10/84";
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

#include <pwd.h>

/*
 * Code taken from 4.3BSD /usr/ucb/Mail. This code basically wraps the
 * getpwuid() and getpwnam() functions, and should simply be merged with
 * another source file.
 */

#include "rcv.h"

/*
 * Search the passwd file for a uid.  Return name through ref parameter
 * if found, indicating success with 0 return.  Return -1 on error.
 */

getname(uid, namebuf)
	char namebuf[];
{
	struct passwd *pw;

	if ((pw = getpwuid(uid)) == NULL)
		return(-1);
	strcpy(namebuf, pw->pw_name);
	return (0);
}

/*
 * Convert the passed name to a user id and return it.  Return -1
 * on error.
 */

getuserid(name)
	char name[];
{
	struct passwd *pw;

	if ((pw = getpwnam(name)) == NULL)
		return (-1);
	return (pw->pw_uid);
}
