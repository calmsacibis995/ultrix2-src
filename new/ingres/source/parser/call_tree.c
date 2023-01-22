#ifndef lint
static	char	*sccsid = "@(#)call_tree.c	1.1	(ULTRIX)	1/8/85";
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
# include	<tree.h>
# include	<pv.h>
# include	"parser.h"


/*
**  CALL_TREE -- call the appropriate module below
**
**	Call_tree prepends a TREE node to the leftmost node on the tree,
**	adds the tree to the PARM, and does a CM call().
**
**	Parameters:
**		qmode -- qmode of query
**		dest -- module to call
**		err_fcn() -- function to call on error
**
**	Returns:
**		nothing
**
**	Trace Flags:
**		call_tree ~~ 44.0, 44.4
*/

call_tree(qmode, dest, err_fcn)
register int	qmode;
int		dest;
int		(*err_fcn)();
{
	extern int	Resrng;
	extern QTREE	*Lastree;

#	ifdef	xPTR2
	tTfp(44, 0, "call_tree: qm=%d, dest=%d\n", qmode, dest);
#	endif

	Qt.qt_qmode = qmode;

#	ifdef	xPTR2

	if (tTf(44, 4))
		if (Resrng >= 0)
			printf("resvarno:%d\n", Resrng);
#	endif

	Qt.qt_resvar = Resrng;

	/* the following attaches the TREE node to the far left of the tree */

	tlprepend(tree(NULL, NULL, TREE, 0), Lastree);

	setp(PV_QTREE, Lastree);

	call(dest, err_fcn);

#	ifdef	xPTR2
	tTfp(44, 5, "Call_tree: call returned\n");
#	endif
}
