#ifndef lint
static	char	*sccsid = "@(#)startovqp.c	1.1	(ULTRIX)	1/8/85";
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
# include	<aux.h>
# include	"../decomp/globs.h"
# include	"../ctlmod/pipes.h"
# include	<signal.h>



/*
**	startovqp is called at the beginning of
**	the execution of ovqp.
*/


startovqp()
{
	extern	flptexcep();

	if (Equel)
		startequel();

	De.ov_tupsfound = 0;	/* counts the number of tuples which sat the qual */
	De.ov_retrieve = De.ov_bopen = FALSE;
	/* catch floating point signals */
	signal(SIGFPE, (int) flptexcep);
}

/*
**	Give a user error for a floating point exceptions
*/
flptexcep()
{
	ov_err(FLOATEXCEP);
}
