#ifndef lint
static	char	*sccsid = "@(#)opencat.c	1.1	(ULTRIX)	1/8/85";
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
# include	<aux.h>
# include	<opsys.h>
# include	<access.h>


/*
**  OPENCATALOG -- open system catalog
**
**	This routine opens a system catalog into a predetermined
**	cache.  If the catalog is already open, it is not reopened.
**
**	The 'Desxx' struct defines which relations may be opened
**	in this manner and is defined in .../source/aux.h.
**
**	The relation should not be closed after use (except in
**	special cases); however, it should be noclose'd after use
**	if the number of tuples in the catalog may have changed.
**
**	The Desxx structure has an alias field which
**	is the address of the 'Admin' structure cache which holds
**	the relation descriptor.  Thus, an openr need never actually
**	occur.
**
**	The actual desxx structure definition is in the file
**	
**		catalog_desc.c
**
**	which defines which relations can be cached and if any
**	alias descriptors exist for the relations. That file
**	can be redefined to include various caching.
**
**
**	Parameters:
**		name -- the name of the relation to open.  It must
**			match one of the names in the Desxx
**			structure.
**		mode -- just like 'mode' to openr.  If zero, it
**			is opened read-only; if two, it is opened
**			read/write.  In fact, the catalog is always
**			opened read/write, but the flags are set
**			right for concurrency to think that you are
**			using it as you have declared.
**
**	Returns:
**		none
**
**	Side Effects:
**		A relation is (may be) opened.
**
**	Trace Flags:
**		none
*/

opencatalog(name, mode)
char	*name;
int	mode;
{
	int			i;
	register DESC		*d;
	register char		*n;
	register struct desxx	*p;
	extern struct desxx	Desxx[];
	extern long		CmOfiles;

	n = name;

	/* find out which descriptor it is */
	for (p = Desxx; p->cach_relname; p++)
		if (sequal(n, p->cach_relname))
			break;
	if (!p->cach_relname)
		syserr("opencatalog: (%s)", n);

	d = p->cach_desc;

	/* if it's already open, just return */
	if (d->relopn)
	{
		clearkeys(d);
	}
	else
	{
		/* not open, open it */
		if (p->cach_alias)
		{
			acc_init();
			bmove((char *) p->cach_alias, (char *) d, sizeof (*d));
		}
		else
		{
			if ((i = openr(d, 2, n)) != 0)
				syserr("opencatalog: openr(%s) %d", n, i);
		}

		/* mark it as an open file */
		CmOfiles |= 1 << d->relfp;
	}

	/* determine the mode to mark it as for concurrency */
	switch (mode)
	{
	  case 0:	/* read only */
		d->relopn = abs(d->relopn);
		break;

	  case 2:	/* read-write */
		d->relopn = -abs(d->relopn);
		break;

	  default:
		syserr("opencatalog(%s): mode %d", n, mode);
	}

	return;
}
