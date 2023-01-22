#ifndef lint
static	char	*sccsid = "@(#)nalloc.c	1.1	(ULTRIX)	1/8/85";
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




/*
**	NALLOC --
**		Dynamic allocation routine which
** 		merely calls alloc(III), returning 
**		0 if no core and a pointer otherwise.
**
*/

char *nalloc(s)
int	s;
{
#	ifdef PDP11
	unsigned	size;

	size = s;
	size = alloc(size);
	if (size == -1)
		return (0);
	return (size);

#	else
	extern char	*malloc();

	return (malloc(s));
#	endif
}
/*
**	SALLOC -- allocate
**		place for string and initialize it,
**		return string or 0 if no core.
**
*/

char *salloc(s)
char		*s;
{
	register unsigned	i;
	register char		*string;
	char			*malloc();

	string = s;
#	ifdef PDP11
	i = alloc(length(string) + 1);
	if (i == -1)
		return (0);
#	else
	i = (unsigned)malloc(length(string) + 1);
#	endif

	if (i)
		smove(string, i);
	return ((char *)i);
}

/*
**	XFREE -- Free possibly dynamic storage
**		checking if its in the heap first.
**
**		0 - freed
**		1 - not in heap
**
*/

xfree(cp)
char		*cp;
{
	extern 			end;	/* break (II) */
	register char		*lcp, *lend, *lacp;

	lcp = cp;
	lacp = (char *)&cp;
	lend = (char *)&end;
	if (lcp >= lend && lcp < lacp)	/* make sure its in heap */
	{
		free(lcp);
		return (0);
	}
	return (1);
}
