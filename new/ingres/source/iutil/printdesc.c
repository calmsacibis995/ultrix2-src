#ifndef lint
static	char	*sccsid = "@(#)printdesc.c	1.1	(ULTRIX)	1/8/85";
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


/*
**  PRINT RELATION DESCRIPTOR (for debugging)
**
**	A pointer of a file descriptor is passed.  All pertinent
**	info in that descriptor is printed on the standard output.
**
**	For debugging purposes only
*/

printdesc(d)
register DESC	*d;
{
	register int	i;
	register int	end;

	printf("Descriptor @ %x %.12s %.2s (%.12s)\n", d,
	    d->reldum.relid, d->reldum.relowner, d->relvname);
	printf("spec %d, indxd %d, stat %d, save %s",
		d->reldum.relspec, d->reldum.relindxd, d->reldum.relstat,
		ctime(&d->reldum.relsave));
	printf("tups %ld, atts %d, wid %d, prim %ld, stamp %s",
		d->reldum.reltups, d->reldum.relatts, d->reldum.relwid,
		d->reldum.relprim, ctime(&d->reldum.relstamp));
	printf("fp %d, opn %d, adds %ld, ",
		d->relfp, d->relopn, d->reladds);
	dumptid(&d->reltid);

	end = d->reldum.relatts;
	for (i = 0; i <= end; i++)
	{
		printf("[%2d] off %3d fmt %c%3d, xtra %3d, given %3d\n",
			i, d->reloff[i], d->relfrmt[i],
			d->relfrml[i] & 0377, d->relxtra[i], d->relgiven[i]);
	}
}
