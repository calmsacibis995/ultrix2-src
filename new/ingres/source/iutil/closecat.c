#ifndef lint
static	char	*sccsid = "@(#)closecat.c	1.1	(ULTRIX)	1/8/85";
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
# include	<access.h>


/*
**  CLOSECATALOG -- close system catalog
**
**	This routine closes the sysetm relations opened by calls
**	to opencatalog.
**
**	The 'Desxx' struct defines which relations will be closed
**	in this manner and is defined in .../source/aux.h.
**
**	The actual desxx structure definition is in the file
**	
**		catalog_desc.c
**
**	which defines which relations can be cached and if any
**	alias descriptors exist for the relations. That file
**	can be redefined to include various caching.
**
**	Parameters:
**		really - whether to actually close the relations
**			or just update and flush them.
**
**	Returns:
**		none
**
**	Side Effects:
**		A relation is (may be) closed and its pages flushed
**
**	Trace Flags:
**		none
*/


closecatalog(really)
bool	really;
{
	register struct desxx	*p;
	extern struct desxx	Desxx[];
	extern long		CmOfiles;

	for (p = Desxx; p->cach_relname; p++)
	{
		if (really && !p->cach_alias)
		{
			CmOfiles &= ~(1 << p->cach_desc->relfp);
			if (closer(p->cach_desc) < 0)
				syserr("closecat %s", p->cach_relname);
		}
		else
		{
			if (noclose(p->cach_desc) < 0)
				syserr("closecat %s", p->cach_relname);
			if (really)
				p->cach_desc->relopn = 0;
		}
	}
}
