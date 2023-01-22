#ifndef lint
static	char	*sccsid = "@(#)de_init.c	1.1	(ULTRIX)	1/8/85";
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

# include <ingres.h> 
# include <symbol.h>
# include <range.h>
# include "globs.h"


/*ARGSUSED*/
de_init(argc, argv)
int	argc;
char	**argv;
{
	Batchupd = setflag(argv, 'b', 1);

	/*
	** Do the necessary decomp initialization. This includes
	** buffering standard output (if i/d system) and giving
	** access methods more pages (if i/d system).
	** init_decomp is defined in either call_ovqp or call_ovqp70.
	*/

	init_decomp();
}
/*
**  RUBPROC -- process a rubout signal
**
**	Called from the principle rubout catching routine
**	when a rubout is to be processed. Notice that rubproc
**	must return to its caller and not call reset itself.
**
**	Parameters:
**		none
**
**	Returns:
**		none
**
**	Side Effects:
**		reinitializes the state of the world.
**
**	Called By:
**		rubcatch
*/


de_rubproc()
{
	extern int	Equel;

	/*
	** Sync with equel if we have the equel pipe.
	**	This can happen only if ovqp and decomp
	**	are combined.
	*/
/*
	if (W_front >= 0 && Equel)
	Error_flag = pv[0].pv_val.pv_int;
		wrpipe(P_INT, &pipebuf, W_front);
*/

	endovqp(RUBACK);
	reinit();
	return;
}
