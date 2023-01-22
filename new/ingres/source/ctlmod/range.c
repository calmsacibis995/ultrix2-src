#ifndef lint
static	char	*sccsid = "@(#)range.c	1.1	(ULTRIX)	1/8/85";
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
# include	<range.h>
# include	<tree.h>




/*
**  CLRRANGE -- clear range table(s)
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		The range table (Qt.qt_rangev) is cleared.
*/

clrrange()
{
	register int	i;
	register DESC	*d;

	for (i = 0; i < MAXRANGE; i++)
	{
		Qt.qt_rangev[i].rngvmark = FALSE;
		Qt.qt_remap[i] = i;
		d = Qt.qt_rangev[i].rngvdesc;
		if (d != NULL)
		{
			xfree(d);
			Qt.qt_rangev[i].rngvdesc = NULL;
		}
	}
}
/*
**  DECLARE -- declare a range variable
**
**	A range variable is declared.  If possible, the preferred varno
**	stated is used (this is the one already in the tree).  This
**	should always be possible when reading the original tree (and
**	should probably stay this way to make debugging easier).  When
**	not possible, a new varno is chosen and the tree can later
**	be patched up by 'mapvars'.
**
**	Parameters:
**		varno -- the preferred varno.
**		desc -- the descriptor for this range variable.
**
**	Returns:
**		The actual varno assigned.
**
**	Side Effects:
**		Qt.qt_rangev is updated.
**
**	Trace Flags:
**		7.0-3
*/

declare(varno, desc)
int	varno;
DESC	*desc;
{
	register int	i;
	register int	vn;
	extern char	*trim_relname();

	vn = varno;

	/* check for preferred slot in range table available */
	if (desc != NULL && Qt.qt_rangev[vn].rngvdesc != NULL)
	{
		/* try to find another slot */
		for (i = 0; i < MAXRANGE; i++)
			if (Qt.qt_rangev[i].rngvdesc == NULL)
				break;

		if (i >= MAXRANGE)
		{
			/* too many variables */
			error(3100, trim_relname(desc->reldum.relid), 0);
		}

		vn = i;
	}

	/* if clearing, make sure something to clear */
	if (desc == NULL && Qt.qt_rangev[vn].rngvdesc == NULL)
		syserr("declare: null clr %d", vn);

	/* declare variable in the slot */
	Qt.qt_rangev[vn].rngvdesc = desc;
	Qt.qt_rangev[vn].rngvmark = (desc != NULL);

#	ifdef xQTR2
	if (tTf(7, 0))
		lprintf("declare(%d, %.14s) into slot %d\n", varno,
		    desc != NULL ? desc->reldum.relid : "(NULL)", vn);
#	endif

	return (vn);
}
