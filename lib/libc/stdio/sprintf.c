#ifndef lint
static	char	*sccsid = "@(#)sprintf.c	1.2	(ULTRIX)	11/25/85";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
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
/************************************************************************
 *			Modification History
 *
 * 001	David L Ballenger, 12-Nov-1985
 *	Add code for System V support.  In the System V environment, 
 *	sprintf() returns the number of characters (excluding the
 *	terminating '\0') placed in the string, in the ULTRIX
 *	environment the address of the string is returned.
 *
 ************************************************************************/

#include	<stdio.h>

#ifndef SYSTEM_FIVE
#define RETURN_TYPE char *
#else
#define RETURN_TYPE int
#endif

RETURN_TYPE
sprintf(str, fmt, args)
	char *str;
	char *fmt;
{
	struct _iobuf _strbuf;
#ifdef SYSTEM_FIVE
	register int n_chars;
#endif

	_strbuf._flag = _IOWRT|_IOSTRG;
	_strbuf._base = _strbuf._ptr = str;
	_strbuf._cnt = 32767;

	/* Call _doprnt() to do the dirty work.  In the ULTRIX environment,
	 * value is ignored.
	 */
#ifndef SYSTEM_FIVE
	(void)_doprnt(fmt, &args, &_strbuf);
#else
	n_chars = _doprnt(fmt, &args, &_strbuf);
#endif
	/* Terminate the string with a null character.
	 */
	*_strbuf._ptr = '\0';

	/* In the ULTRIX environment return the string, and in the
	 * System V environment return the number of characters
	 * transmitted by _doprnt().
	 */
#ifndef SYSTEM_FIVE
	return(str);
#else
	return(n_chars);
#endif
}
