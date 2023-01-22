#ifndef lint
static	char	*sccsid = "@(#)xalloc.c	1.1	(ULTRIX)	1/8/85";
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


/*
**  XALLOC -- allocate block of memory.
**
**	This is just like malloc, except that it is guaranteed
**	to succeed.  It will syserr if it fails.
**
**	Parameters:
**		sz -- size in bytes of memory area to allocate.
**
**	Returns:
**		pointer to area allocated.
**
**	Side Effects:
**		none.
**
**	Trace Flags:
**		none.
*/

char *
xalloc(sz)
int	sz;
{
	register char	*p;
	extern char	*malloc();

	p = malloc(sz);
	if (p == NULL)
		syserr("Out of memory");
	return (p);
}
