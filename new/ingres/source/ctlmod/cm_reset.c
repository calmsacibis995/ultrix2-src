#ifndef lint
static	char	*sccsid = "@(#)cm_reset.c	1.1	(ULTRIX)	1/8/85";
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


/*
**  CM_RESET -- reset control module
**
**	Called on a RESET block, this routine should reset the
**	state to something well known.  Useful for debugging.
**
**	Parameters:
**		none
**
**	Returns:
**		never -- non-locally to top of CM loop.
**
**	Side Effects:
**		'Syncs' is cleared.
**		'Qbuf' is reset.
**		'Cm.cm_input' is set to the default input.
**		All extra files are closed.
**
**	Trace Flags:
**		none
*/

cm_reset()
{
	register int	i;
	extern jmp_buf	CmReset;
	extern long	CmOfiles;

	/* clear all Syncs */
	for (i = 0; i < CM_MAXPROC; i++)
		Syncs[i] = 0;

	/* close and report all extraneous files */
	closeall(TRUE, CmOfiles);

	/* return to top of loop */
	longjmp(CmReset, 3);
}
