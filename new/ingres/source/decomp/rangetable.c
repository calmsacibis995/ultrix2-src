#ifndef lint
static	char	*sccsid = "@(#)rangetable.c	1.1	(ULTRIX)	1/8/85";
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
# include	<symbol.h>
# include	"globs.h"


/*
** Allocation of range table.
** The size of the range table for decomp is
** MAXRANGE plus 2; 1 for a free aggregate slot and 1 for
** a free secondary index slot.
**
**	Trace Flags:
**		63
*/



initrange()
{
	register struct rang_tab	*rt;

	for (rt = De.de_rangev; rt <= &De.de_rangev[MAXRANGE+1]; rt++)
		rt->relnum = -1;
}
/*
**	Save the entry for var in the range table.
*/

savrang(locrang, var)
int	locrang[];
int	var;
{
	register int	i;

	i = var;
	locrang[i] = De.de_rangev[i].relnum;
}
/*
**	Restore the entry for var from the local range
**	table locrang.
*/

rstrang(locrang, var)
int	locrang[];
int	var;
{
	register int	i;

	i = var;
	De.de_rangev[i].relnum = locrang[i];
}
/*
**	Update the range name. It is up to
**	the calling routine to openr the new rel.
*/

new_range(var, relnum)
int	var;
int	relnum;
{
	register int	i, old;

	i = var;

	old = De.de_rangev[i].relnum;
	De.de_rangev[i].relnum = relnum;

	return (old);
}
/*
**	Make a copy of the current range table.
*/

newquery(locrang)
int	locrang[];
{
	register struct rang_tab	*rp;
	register int			*ip, i;

	ip = locrang;
	rp = De.de_rangev;

	for (i = 0; i < MAXRANGE; i++)
		*ip++ = (rp++)->relnum;
}
/*
**	Check the range table to see if any
**	relations changed since the last call
**	to newquery. If so, they were caused
**	by reformat. Restore back the orig relation
**	Reopen it if reopen == TRUE.
*/

endquery(locrang, reopen)
int	locrang[];
int	reopen;
{
	register struct rang_tab	*rp;
	register int			*ip, i;
	int				old;
	bool				dstr_flag;
	extern DESC			*openr1();

	rp = De.de_rangev;
	ip = locrang;

	dstr_flag = FALSE;
	initp();
	for (i = 0; i < MAXRANGE; i++)
	{
		if (rp->relnum != *ip)
		{
#			ifdef xDTR1
			if (tTf(63, -1))
			printf("reformat or reduct changed var %d (%d,%d)\n", i, *ip, rp->relnum);
#			endif

			old = new_range(i, *ip);
			dstr_flag |= dstr_mark(old);
			if (reopen)
				openr1(i);
		}

		ip++;
		rp++;
	}

	if (dstr_flag)
		call_dbu(mdDESTROY, FALSE);
	else
		resetp();
}
/*
**	Return the name of the variable "var"
**	in the range table
*/

char *
rangename(var)
int	var;
{
	extern char	*rnum_convert();

	return (rnum_convert(De.de_rangev[var].relnum));
}
