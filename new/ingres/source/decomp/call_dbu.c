#ifndef lint
static	char	*sccsid = "@(#)call_dbu.c	1.1	(ULTRIX)	1/8/85";
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
# include	<tree.h>
# include	<symbol.h>
# include	<pv.h>
# include	"globs.h"


int Synconly, Error_flag;


/*
**	Call the appropriate dbu with the arguments
**	given in the globals Pc and Pv. Code is a
**	number identifing which dbu to call. Errflag
**	indicates whether an error return from the dbu
**	is possibly expected.
**
**	If errflag is FALSE then call_dbu will syserr on an error
**	If errflag is TRUE then call_dbu will return error value
**
**	Trace Flags:
**		60
*/

call_dbu(code, errflag)
int	code;
bool	errflag;
{
#	ifdef xDTR1
	if (tTf(60, 0))
		printf("Calling DBU %d\n", code);
#	endif

	Error_flag = 0;
	call(code, NULL);
	if (Error_flag != 0 && !errflag)
		syserr("call_dbu:%d,ret %d", code, Error_flag);
	return(Error_flag);
}



/*
**	Proc_error is called if an error
**	block is encountered.
**	Otherwise the error block(s) are passed on up.
*/

/*ARGSUSED*/
catcherr(pc, pv)
int	pc;
PARM	*pv;
{
	extern int	Error_flag;

	Error_flag = pv[0].pv_val.pv_int;
}
