#ifndef lint
static	char	*sccsid = "@(#)utility.c	1.1	(ULTRIX)	1/8/85";
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
# include	<access.h>


dumptid(tid)
register TID	*tid;
{
	long	pageid;

	pluck_page(tid, &pageid);
	printf("tid: %ld/%d\n", pageid, (tid->line_id & I1MASK));
	return (0);
}

/*
**	struct for extracting page number from a tid
**	and storing in a long
**
**	We want the line number (lpgx) to be in the low-order part of
**	a long.  Since PDP's and VAXes have the order of the half-
**	words reversed, this structure must be different.
*/

struct lpage
{
# ifdef PDP11
	char	lpg0, lpgx;
	char	lpg2, lpg1;
# else
	char	lpg2, lpg1, lpg0, lpgx;
# endif
};
/*  PLUCK_PAGE
**
**	pluck_page extracts the three byte page_id from a TID
**	and puts it into a long variable with proper allignment.
*/

pluck_page(t, var)
register TID	*t;
long		*var;
{
	register struct lpage	*v;

	v = (struct lpage *) var;
	v->lpg0 = t->pg0;
	v->lpg1 = t->pg1;
	v->lpg2 = t->pg2;
	v->lpgx = 0;
	return (0);
}

/*	stuff_page is the reverse of pluck_page	*/
stuff_page(t, var)
register TID	*t;
long		*var;
{
	register struct lpage	*v;

	v = (struct lpage *) var;
	t->pg0 = v->lpg0;
	t->pg1 = v->lpg1;
	t->pg2 = v->lpg2;
	return (0);
}
