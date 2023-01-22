#ifndef lint
static	char	*sccsid = "@(#)batchxtra.c	1.1	(ULTRIX)	1/8/85";
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
# include	<access.h>
# include	<batch.h>


rmbatch()
{
	char		*batchname();
	register char	*p;
	register int	i;

	p = batchname();
	if (i = close(Batch_fp))
		syserr("rmbatch:can't close %s %d", p, i);
	if (i = unlink(p))
		syserr("rmbatch:can't unlink %s %d", p, i);
	Batchhd.mode_up = 0;
	return (0);
}
/*
**  BATCHNAME -- create batch file name
*/

char *
batchname()
{
	extern char	*Fileset;
	extern char	*ztack();

	return(ztack("_SYSbatch", Fileset));
}
