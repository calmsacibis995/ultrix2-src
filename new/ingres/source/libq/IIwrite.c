#ifndef lint
static	char	*sccsid = "@(#)IIwrite.c	1.1	(ULTRIX)	1/8/85";
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
# include	<pipes.h>
# include	"IIglobals.h"



/*
**	IIwrite is used to write a string to the
**	quel parser
*/

IIwrite(str)
char	*str;
{
	register char	*s;
	register int	i;

	s = str;
#	ifdef xETR1
	if (IIdebug)
		printf("write:string='%s'\n", s);
#	endif
	if (!IIingpid)
		IIsyserr("no preceding ##ingres statement");
	if (IIin_retrieve)
		IIsyserr("IIwrite:you cannot call ingres while in a retrieve");

	if (!IInewqry)
	{
		IIpb_prime(&IIpb, PB_REG);
		IIpb.pb_proc = 1;
		IIpb_put("\0\0\0", 3, &IIpb);

		IInewqry = 1;
	}

	if ((i = IIlength(s)) != 0)
		IIpb_put(s, i, &IIpb);
}
