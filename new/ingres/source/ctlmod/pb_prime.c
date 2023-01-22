#ifndef lint
static	char	*sccsid = "@(#)pb_prime.c	1.1	(ULTRIX)	1/8/85";
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
# include	"pipes.h"


/*
**  PB_PRIME -- prime a pipe for reading or writing
**
**	This clears out any nasty stuff in the pipe block.  If
**	we are reading, it reads the first block so that we can
**	know what sort it is.
**
**	Parameters:
**		ppb -- a pointer to the pipe block.
**		type -- if PB_NOTYPE, we are setting for reading.
**			Otherwise, we are setting to write a
**			message of the indicated type.
**
**	Returns:
**		none
**
**	Side Effects:
**		Changes *ppb.
**
**	Trace Flags:
**		12.0
*/

pb_prime(ppb, type)
register pb_t	*ppb;
int		type;
{
# ifdef xCTR2
	if (tTf(12, 0))
		lprintf("pb_prime: type %d\n", type);
# endif
	if (type == PB_NOTYPE)
	{
		/* read pipe prime -- get the first block */
		pb_read(ppb);
	}
	else
	{
		/* write pipe prime -- set up initial pointers */
		ppb->pb_from = Cm.cm_myproc;
		ppb->pb_type = type;
		ppb->pb_stat = 0;
		ppb->pb_nleft = PB_DBSIZE;
		ppb->pb_nused = 0;
	}

	/* do common initialization */
	ppb->pb_xptr = ppb->pb_data;
}
