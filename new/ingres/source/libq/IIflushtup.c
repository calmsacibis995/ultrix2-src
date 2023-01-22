#ifndef lint
static	char	*sccsid = "@(#)IIflushtup.c	1.1	(ULTRIX)	1/8/85";
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
# include	<symbol.h>
# include	"IIglobals.h"



/*
**	IIflushtup is called to syncronize the data pipe
**	after a retrieve.
*/

IIflushtup(file_name, line_no)
char	*file_name;
int	line_no;
{
	register int		i;

	if (IIproc_name = file_name)
		IIline_no = line_no;

#	ifdef xATR1
	if (IIdebug)
		printf("IIflushtup: IIerrflag %d\n", IIerrflag);
#	endif

	if (IIerrflag < 2000)
	{
		while ((IIpb.pb_stat & PB_EOF) == 0)
			IIpb_read(&IIpb);

		/* read the RESP block */
		IIpb_prime(&IIpb, PB_NOTYPE);
		IIreadinput(&IIpb);
	}

	IIin_retrieve = 0;
	IIndomains = 0;
	IIdomains = 0;
	IInewqry = 0;
}
