#ifndef lint
static	char	*sccsid = "@(#)pass_err.c	1.1	(ULTRIX)	1/8/85";
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
# include	<pv.h>


/*
**  PASS_ERR -- the error passing routine for the parser
**
**	Trace Flags:
**		pass_err ~~ 65
*/

pass_err(pc, pv)
int	pc;
PARM	pv[];
{
	extern int	Ingerr;
	extern int	Err_fnd;
	register int	num;

	num = pv[0].pv_val.pv_int;
# ifdef	xPTR1
	tTfp(65, 0, "pass_err %d\n", num);
# endif

	if (Ingerr)
		Ingerr = num;
	else
		Ingerr = 1;

	Err_fnd += 1;

	return (-1);		/* means pass error message up to front end */
}
