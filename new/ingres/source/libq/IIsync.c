#ifndef lint
static	char	*sccsid = "@(#)IIsync.c	1.1	(ULTRIX)	1/8/85";
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
# include	"IIglobals.h"



/*
**	IIsync is called to syncronize the running
**	of a query with the running of the equel process.
**
**	The query is flushed and an EOP is written
**	to the quel parser.
**
**	The quel parser will write an end-of-pipe when
**	an operation is complete.
*/

IIsync(file_name, line_no)
char	*file_name;
int	line_no;
{
	pb_t	pb;

	IIpb_flush(&IIpb);
	if (IIproc_name = file_name)
		IIline_no = line_no;

#	ifdef xETR1
	if (IIdebug)
		printf("IIsync\n");
#	endif


	IIerrflag = 0;	/* reset error flag. If an error occures,
			** IIerrflag will get set in IIerror
			*/

	IIpb_prime(&pb, PB_NOTYPE);
	IIreadinput(&pb);
	IInewqry = 0;
}
