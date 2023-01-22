#ifndef lint
static	char	*sccsid = "@(#)last_page.c	1.1	(ULTRIX)	1/8/85";
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
# include 	<opsys.h>


/*
**	LAST_PAGE -- computes a tid for the last page in the relation.
*/

# ifdef xV6_UNIX
struct stat
{
	char	junk[9], size0;
	int	size1;
	char	junk2[25];
};
# endif

last_page(d, tid, buf)
register DESC		*d;
register TID		*tid;
register struct accbuf	*buf;
{
	long		lpage;
	struct stat	stats;

	if ((buf != 0) && (abs(d->reldum.relspec) == M_HEAP) && (buf->mainpg == 0) && (buf->ovflopg == 0))
		lpage = buf->thispage;
	else
	{
		if (fstat(d->relfp, &stats))
			syserr("last_page: fstat err %.14s", d->reldum.relid);
#		ifdef xV6_UNIX
		/* number of pages in relation - 1 */
		lpage = ((stats.size1 >> 9) & 0177) + ((stats.size0 & 0377) << 7)- 1;
#		else
		lpage = stats.st_size / PGSIZE - 1;
#		endif
#		ifdef xATR2
		if (tTf(26, 8))
			printf("fstat-lp %.12s %ld\n", d->reldum.relid, lpage);
#		endif
	}
	stuff_page(tid, &lpage);
	tid->line_id = 0;
	return (0);
}
