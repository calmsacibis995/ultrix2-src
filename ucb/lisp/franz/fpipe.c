#ifndef lint
static	char	*sccsid = "@(#)fpipe.c	1.2	(ULTRIX)	7/15/85";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
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
/************************************************************************
 *			Modification History
 *
 *	David L Ballenger, 9-Jul-1985
 * 001	Use fdopen() to associate a stream with a file descriptor.
 *
 *	Based on:  /na/franz/franz/RCS/fpipe.c,v 1.1 83/01/29 12:49:40 jkf
 *
 ************************************************************************/
/*					-[Sat Jan 29 12:44:16 1983 by jkf]-
 * 	fpipe.c				$Locker:  $
 * pipe creation
 *
 * (c) copyright 1982, Regents of the University of California
 */


#include "global.h"
#include <signal.h>

FILE * fpipe(info)
FILE *info[2];
{
	int descrips[2];

	if(pipe(descrips) >= 0) {

		/* Get the read stream
		 */
		if ( (info[0] = fdopen(descrips[0],"r")) != NULL) {

			/* Get the write stream.
			 */
			if ( (info[1] = fdopen(descrips[1],"w")) != NULL)
			
				/* Indicate sucess
				 */
				return((FILE *) 2);
		}
		close(descrips[0]);
		close(descrips[1]);
	}
	/* Failure
	 */
	return( (FILE *) -1);
}

FILE	FILEdummy,
	**xports;


#define LOTS (LBPG/sizeof(FILE *))

lispval
P(p)
	FILE		*p;
{
	register FILE	**q;
	extern int	fakettsize;

	if (xports == ((FILE **)0)) {
		xports = (FILE **)csegment(OTHER,LOTS,0);
		SETTYPE(xports,PORT,31);
		for (q = xports; q < xports + LOTS; q++ )
			*q = &FILEdummy;
	}

	for (q = xports; q < xports + LOTS; q++) {
		if (*q == p)
			return((lispval)q);
		if (*q == &FILEdummy) {
			*q = p;
			return((lispval)q);
		}
	}
	error("Ran out of Ports",FALSE);
}
