#ifndef lint
static	char	*sccsid = "@(#)IIresync.c	1.1	(ULTRIX)	1/8/85";
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
# include	"IIglobals.h"
# include	<signal.h>



/*
**  RESYNCHRONIZE PIPES AFTER AN INTERRUPT
**
**	The pipes are all cleared out.  This routines must be called
**	by all processes in the system simultaneously.  It should be
**	called from the interrupt catching routine.
*/

int	IISyncs[CM_MAXPROC];
extern	exit();
int	(*IIinterrupt)() =	exit;

IIresync()
{
	register int	i;
	pb_t		pb;
	register int	stat;

	signal(SIGINT,SIG_IGN);

	/*
	**  Send SYNC blocks to all processes that are adjacent
	**	in the write direction.
	**  Arrange to ignore blocks from all processes that
	**	are adjacent in the read direction.
	*/

	IIpb_prime(&pb, PB_SYNC);
	for (i = 0; i < CM_MAXPROC; i++)
	{
		IISyncs[i]++;

		/* send SYNC to parser */
		pb.pb_proc = 1;
		IIpb_write(&pb);
	}

	/* ovqp buffer flush is done in IIsetup() */

	/* Get out of a retrieve and clear errors */
	IIin_retrieve = 0;
	IIerrflag = 0;
	IIndomains = IIdomains = 0;
	IInewqry = 0;


	/* reset the signal */
	signal(SIGINT, IIresync);
	/* allow the user to service the interrupt */
	(*IIinterrupt)(-1);
	/*
	** If IIinterupt returns the user might hang in a retrieve
	*/

	IIsyserr("Interupt returned");
}

