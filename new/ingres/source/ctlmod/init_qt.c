#ifndef lint
static	char	*sccsid = "@(#)init_qt.c	1.1	(ULTRIX)	1/8/85";
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

# include	"ctlmod.h"
# include	<ingres.h>
# include	<aux.h>
# include	<tree.h>


/*
**  INIT_QT -- initialize query tree
**
**	This routine saves and initializes the global portion
**	of a query tree.  It must be called before doing any
**	creation of a query tree which depends on or modifies
**	the global portion of a query tree.
**
**	The global portion of a query tree includes the range
**	table, the query mode, etc.
**
**	The algorithm for saving the query tree global portion
**	("Qt") is somewhat obscure.  To understand it, it is
**	critical to be clear that every query tree is associated
**	with a context, but not every context is associated with
**	a query tree.  We further constrain that a context
**	is associated with at most one query tree; if more than
**	one is needed in a single context, it is always safe
**	to reuse the previous one.
**
**	When we allocate a new context (in 'initp'), the current
**	Qt (if in use) is associated with the save image of the
**	context.  When we call init_qt, we check to see if such
**	an association exists.  If so, we save Qt, and adjust
**	the old context to point to the save image of Qt.  When
**	we call init_qt again, Qt will be active, but not
**	associated with a context save image; thus, we do not
**	save Qt.  On context restore, if it has a pointer to a
**	saved Qt, we restore that also.
**
**	Parameters:
**		none.
**	
**	Returns:
**		none
**	
**	Side Effects:
**		Qt possibly gets saved in an area set up by
**			calling malloc.
**	
**	Trace Flags:
**		none
*/

init_qt()
{
	extern char	*malloc();
	register char	*p;

	/*
	**  Save Qt if associated with a saved context.
	*/

	if (Qt.qt_ctx != NULL)
	{
		p = malloc(sizeof Qt);
		if (p == NULL)
			syserr("init_qt: no mem");
		bmove((char *)&Qt, p, sizeof Qt);
		((ctx_t *)Qt.qt_ctx)->ctx_qt = p;
		Qt.qt_ctx = NULL;
	}

	/* mark the Qt as active */
	Qt.qt_active++;
}
