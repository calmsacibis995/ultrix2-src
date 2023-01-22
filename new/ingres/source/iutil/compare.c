#ifndef lint
static	char	*sccsid = "@(#)compare.c	1.1	(ULTRIX)	1/8/85";
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
# include	<aux.h>


icompare(ax, bx, frmt, frml)
char	*ax, *bx, frmt, frml;
{
	register ANYTYPE	*a, *b;
	register int		length;
	ANYTYPE			atemp, btemp;

	length = frml & I1MASK;
	if (frmt == CHAR)
		return (scompare(ax, length, bx, length));
	a = &atemp;
	b = &btemp;
	bmove(ax, (char *) a, length);
	bmove(bx, (char *) b, length);
	if (bequal((char *) a, (char *) b, length))
		return (0);
	switch (frmt)
	{
	  case INT:
		switch (length)
		{
		  case 1:
			return (a->i1type - b->i1type);
		  case 2:
			return (a->i2type - b->i2type);
		  case 4:
			return (a->i4type > b->i4type ? 1 : -1);
		}
		break;

	  case FLOAT:
		if (frml == 4)
			return (a->f4type > b->f4type ? 1 : -1);
		else
			return (a->f8type > b->f8type ? 1 : -1);
		break;
	}
	syserr("compare: t=%d,l=%d", frmt, frml);
	/*NOTREACHED*/
}
/*
**  KCOMPARE -- key compare
**
**	compares all domains indicated by SETKEY in the tuple to the
**	corressponding domains in the key.
**	the comparison is done according to the format of the domain
**	as specified in the descriptor.
**
**	function values:
**		<0 tuple < key
** 		=0 tuple = key
**		>0 tuple > key
*/

kcompare (dx, tuple, key)
DESC	*dx;			/*relation descriptor	*/
char	tuple[MAXTUP];		/*tuple to be compared	*/
char	key[MAXTUP];		/*second tuple or key	*/
{
	register int	i, tmp;
	register DESC	*d;

	d = dx;
	for (i = 1; i <= d->reldum.relatts; i++)
		if (d->relgiven[i])
			if (tmp = icompare(&tuple[d->reloff[i]],
			        &key[d->reloff[i]],
			        d->relfrmt[i],
				d->relfrml[i]))
			{
				return (tmp);
			}
	return (0);
}
