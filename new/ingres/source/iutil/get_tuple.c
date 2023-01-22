#ifndef lint
static	char	*sccsid = "@(#)get_tuple.c	1.1	(ULTRIX)	1/8/85";
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
# include	<symbol.h>


/*
**	Routine associated with getting a tuple out of
**	the current buffer.
*/


/*
**	Get_tuple - take the tuple specified
**	by tid and move it to "tuple"
*/

get_tuple(d, tid, tuple)
register DESC	*d;
TID		*tid;
char		*tuple;
{
	register char	*cp;
	char		*get_addr();

	cp = get_addr(tid);

	if (d->reldum.relspec < 0)
		uncomp_tup(d, cp, tuple);	/* compressed tuple */
	else
		bmove(cp, tuple, d->reldum.relwid);	/* uncompressed tuple */
}
/*
**	Getint_tuple - get the tuple specified by
**	tid. If possible avoid moving the tuple.
**	Return value is address of tuple.
*/

char *
getint_tuple(d, tid, tuple)
register DESC	*d;
TID		*tid;
char		*tuple;
{
	register char	*cp, *ret;
	extern char	*get_addr();

	cp = get_addr(tid);

	if (d->reldum.relspec < 0)
	{
		ret = tuple;
		uncomp_tup(d, cp, ret);	/* compressed tuple */
	}
	else
		ret = cp;			/* uncompressed tuple */
	return (ret);
}
/*
**	Routine to compute the address of a tuple
**	within the current buffer.
**	Syserr if specified tuple deleted.
*/

char *
get_addr(tid)
register TID	*tid;
{
	register int	offset;

	offset = Acc_head->linetab[-(tid->line_id & I1MASK)];
	if (offset == 0)
	{
		syserr("get_addr rel=%ld tid=%ld", Acc_head->rel_tupid, *(long *)tid);
	}
#	ifdef xATR3
	if (offset < 0 || offset > PGSIZE)
		syserr("get_addr: offset=%d\n");
#	endif
	return (((char *) Acc_head) + offset);
}
/*
**	Uncompress - decompress the tuple at address cp
**	according to descriptor.
*/

uncomp_tup(d, cp, tuple)
register DESC	*d;
char		*cp;
char		*tuple;
{
	register char	*src, *dst;
	int		i, j;

	dst = tuple;
	src = cp;

	/* for each domain, copy and blank pad if char */
	for (i = 1; i <= d->reldum.relatts; i++)
	{
		j = d->relfrml[i] & I1MASK;
		if (d->relfrmt[i] == CHAR)
		{
			while (j--)
			{
				if ((*dst++ = *src++) == NULL)
				{
					/* back up one */
					dst--;
					j++;
					break;
				}
			}

			/* blank pad if necessary */
			while (j-- > 0)
				*dst++ = ' ';
		}
		else
		{
			while (j--)
				*dst++ = *src++;
		}
	}
}
/*
**	Check if a line number is valid.
**	If linenumber is illegal return AMINVL_ERR
**	if Line has been deleted return 2
**	else return 0
*/

invalid(tid)
register TID	*tid;
{
	register int	i;

	i = tid->line_id & I1MASK;

	if (i >= Acc_head->nxtlino)
		return (acc_err(AMINVL_ERR));

	if (Acc_head->linetab[-i] == 0)
		return (2);

	return (0);
}
