#ifndef lint
static	char	*sccsid = "@(#)add_ovflo.c	1.1	(ULTRIX)	1/8/85";
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
# include	<aux.h>
# include	<lock.h>


/*
**	ADD_OVFLO -- Allocates an overflow page which will be
**		attached to current page.  TID must be for current page.
**		TID will be updated to id of new overflow page.
*/

add_ovflo(dx, tid)
DESC	*dx;
TID	*tid;
{
	register DESC		*d;
	register struct accbuf	*b;
	extern struct accbuf	*choose_buf();
	register int		lk;
	int			i;
	long			mpage, newpage;
	TID			tidx;

	d = dx;
#	ifdef xATR2
	if (tTf(26, 3))
		printf("ADD_OVFLO:\n");
#	endif

	/*
	**	save main page pointer so that it may be used when
	**	setting up new overflow page.
	*/
	mpage = Acc_head->mainpg;

	if (lk = (Acclock && (d->reldum.relstat & S_CONCUR) && (d->relopn < 0 )))
	{
		setcsl(Acc_head->rel_tupid);
		Acclock = FALSE;
	}

	/*
	**	Determine last page of the relation
	*/
	last_page(d, &tidx, Acc_head);
	pluck_page(&tidx, &newpage);
	newpage++;

	/*
	**	choose an available buffer as victim for setting up
	**	overflow page.
	*/
	if ((b = choose_buf(d, newpage)) == NULL)
	{
		if (lk)
		{
			Acclock = TRUE;
			unlcs(Acc_head->rel_tupid);
		}
		return(-1);
	}

	/*
	**	setup overflow page
	*/

	b->mainpg = mpage;
	b->ovflopg = 0;
	b->thispage = newpage;
	b->linetab[0] = (int) b->firstup - (int) b;
	b->nxtlino = 0;
	b->bufstatus |= BUF_DIRTY;
	if (pageflush(b))
		return (-2);

	/*
	**	now that overflow page has successfully been written,
	**	get the old current page back and link the new overflow page
	**	to it.
	**	If the relation is a heap then don't leave the old main
	**	page around in the buffers. This is done on the belief
	**	that it will never be accessed again.
	*/

	if (get_page(d, tid))
		return (-3);
	Acc_head->ovflopg = newpage;
	Acc_head->bufstatus |= BUF_DIRTY;
	i = pageflush(Acc_head);
	if (lk)
	{
		Acclock = TRUE;
		unlcs(Acc_head->rel_tupid);
	}
	if (i)
		return (-4);
	if (abs(d->reldum.relspec) == M_HEAP)
		resetacc(Acc_head);	/* no error is possible */

	/*
	**	now bring the overflow page back and make it current.
	**	if the overflow page is still in AM cache, then this will not
	**	cause any disk activity.
	*/

	stuff_page(tid, &newpage);
	if (get_page(d, tid))
		return (-5);
	return (0);
}
