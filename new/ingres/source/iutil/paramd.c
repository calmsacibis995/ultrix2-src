#ifndef lint
static	char	*sccsid = "@(#)paramd.c	1.1	(ULTRIX)	1/8/85";
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
# include	<catalog.h>
# include	<access.h>




/*
**	get access parameters of a relation from its descriptor and return
**	them in struct pointed to by "ap".
*/


paramd(d, ap)
register DESC			*d;
register struct accessparam	*ap;
{
	register int	i;
	int		p;


	ap->mode = getmode(d->reldum.relspec);
	ap->sec_index = FALSE;	/* indicate that this isn't the index-rel */

	for (i = 0; i < MAXDOM+1; i++)
		ap->keydno[i] = 0;

	for (p = 1; p <= d->reldum.relatts; p++)
		if (i = d->relxtra[p])
			ap->keydno[i-1] = p;
	return (0);
}

parami(ip, param)
struct index		*ip;
struct accessparam	*param;
{
	register struct accessparam	*ap;

	ap  = param;
	ap->mode = getmode(ip->irelspeci);
	ap->sec_index = TRUE;	/* this is an index */

	bmove(ip->idom, ap->keydno, MAXKEYS);
	ap->keydno[MAXKEYS] = 0;
	return(0);
}


getmode(spec)
int	spec;
{
	switch (abs(spec))
	{
	  case M_HEAP:
		return(NOKEY);

	  case M_ISAM:
		return(LRANGEKEY);

	  case M_HASH:
		return(EXACTKEY);

	  default: 
		syserr("getmode:bad relspec %d", spec);
	}
}
