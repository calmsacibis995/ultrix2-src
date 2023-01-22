#ifndef lint
static	char	*sccsid = "@(#)del_tuple.c	1.1	(ULTRIX)	1/8/85";
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
**	Delete the specified tuple from the
**	current page.
**
**	The space occupied by the tuple is
**	compacted and the effected remaining
**	tuple line offsets are adjusted.
*/

del_tuple(tid, width)
TID	*tid;
int	width;
{
	register char	*startpt, *midpt;
	register int	i;
	extern char	*get_addr();
	char		*endpt;
	int		cnt, offset, nextline;
	int		linenum;

	linenum = tid->line_id & I1MASK;
	offset = Acc_head->linetab[-linenum];
	nextline = Acc_head->nxtlino;

	startpt = get_addr(tid);
	midpt = startpt + width;
	endpt = (char *)Acc_head + Acc_head->linetab[-nextline];

	cnt = endpt - midpt;

	/* delete tuple */
	Acc_head->linetab[-linenum] = 0;

	/* update affected line numbers */
	for (i = 0; i <= nextline; i++)
	{
		if (Acc_head->linetab[-i] > offset)
			Acc_head->linetab[-i] -= width;
	}

	/* compact the space */
	while (cnt--)
		*startpt++ = *midpt++;
	Acc_head->bufstatus |= BUF_DIRTY;
}
