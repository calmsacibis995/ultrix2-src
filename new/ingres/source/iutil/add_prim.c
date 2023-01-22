#ifndef lint
static	char	*sccsid = "@(#)add_prim.c	1.1	(ULTRIX)	1/8/85";
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


/*
**	ADD_PRIM -- Add a primary page to the relation.  Assumes it is to
**		be tacked onto page in current access method buffer.  No
**		disk write is done but the page is marked for writing.
**		It is assumed that the current page in access method buffer
**		is the last physical page in the relation.
**
**	Trace Flags:
**		26.0,2
*/

add_prim(d, tidx)
DESC	*d;
TID	*tidx;
{
	register struct accbuf	*b;
	register int		i;

	b = Acc_head;
	b->mainpg = b->thispage + 1;
	b->bufstatus |= BUF_DIRTY;
	if (i = pageflush(b))
		return (i);

	/*
	** Now form the new primary page
	*/

	b->thispage = b->mainpg;
	b->mainpg = 0;
	b->ovflopg = 0;
	b->linetab[0] = (int) b->firstup - (int) b;
	b->nxtlino = 0;
	b->bufstatus |= BUF_DIRTY;

	/*
	** Update tid to be new page
	*/
	stuff_page(tidx, &b->thispage);
	return (0);
}
