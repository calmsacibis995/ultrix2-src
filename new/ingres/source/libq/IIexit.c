#ifndef lint
static	char	*sccsid = "@(#)IIexit.c	1.1	(ULTRIX)	1/8/85";
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
**	Exit ingres --
**		waits for the parser to return,
**		catching all children's death till then
**		or till an error return. 
**		In case wait(II) is interrupted while waiting,
**		as evidenced by errno == 4, waiting will resume.
**
*/

IIexit()
{
	register int	ndx;
	register int	pidptr;
	register int	err;
	int		status;
	int		pidlist[10];
	extern		errno;		/* perror(III) */

#	ifdef xETR1
	if (IIdebug)
		printf("IIexit\n");
#	endif

	if (close(IIw_down) || close(IIr_down))
		IIsyserr("IIexit:cant't close");

	pidptr = 0;
	err = 0;
	errno = 0;
	while ((ndx = wait(&status)) != IIingpid
		&& (ndx != -1 || errno == 4))
	{
		if (ndx == -1)
		{
			errno = 0;
			continue;
		}
#		ifdef xETR1
		if (IIdebug)
			printf("caught pid %u death, stat %d, %d\n",
				ndx, status >> 8, status & 0177);
#		endif

		pidlist [pidptr++] = ndx;
		if ((status & 0177) != 0)
		{
			printf("%d: Abnormal Termination %d", ndx, status & 0177);
			if ((status & 0200) != 0)
				printf(" -- Core Dumped");
			printf("\n");
			err++;
		}
	}
	if (err)
	{
		printf("pid list:");
		for (ndx = 0; ndx < pidptr; ndx++)
			printf(" %u", pidlist[ndx]);
		printf("\n");
	}

	IIingpid = 0;
}
