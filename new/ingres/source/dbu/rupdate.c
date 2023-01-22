#ifndef lint
static	char	*sccsid = "@(#)rupdate.c	1.1	(ULTRIX)	1/8/85";
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

# include	<pv.h>
# include	<ingres.h>
# include 	<func.h>


extern	short	tTdbu[];
extern	int	rupdate();
extern	int	null_fn();

struct fn_def RupdatFn =
{
	"RUPDATE",
	rupdate,
	null_fn,		/* initialization function */
	null_fn,
	NULL,
	0,
	tTdbu,
	100,
	'Z',
	0
};
/*
**  RUBOUT SETUP FOR DEFFERED UPDATE PROCESSOR
**
**	These routines setup the special processing for the rubout
**	signal for the deferred update processor.  The update
**	processor is then called.
*/

rupdate(pc, pv)
int	pc;
PARM	pv[];
{
	register int	rtval;

	/* set up special signal processing */
	ruboff("batch update");

	/* call update */
	rtval = update(pc, pv);

	/* clean up signals */
	rubon();

	return (rtval);

}
