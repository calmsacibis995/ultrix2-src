#ifndef lint
static	char	*sccsid = "@(#)clock.c	1.1	(ULTRIX)	4/16/85";
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
 *			Modification History				*
 *									*
 *	David L Ballenger, 16-Apr-1985					*
 * 0001	Define HZ here instead of getting it from <sys/param.h>.  The	*
 *	real HZ value for the system but this corresponds to the usage	*
 *	in the times call.  This routine should be cleaned up to use	*
 *	getrusage() instead of times().					* 
 *									*
 ************************************************************************/

/*	@(#)clock.c	1.1	*/
/*LINTLIBRARY*/

#include <sys/types.h>
#include <sys/times.h>
#define HZ 60
#define TIMES(B)	(B.tms_utime+B.tms_stime+B.tms_cutime+B.tms_cstime)

extern long times();
static long first = 0L;

long
clock()
{
	struct tms buffer;

	if (times(&buffer) == -1L)
		return 0L;		/* DAG -- bug fix */
	if (first == 0L)
		first = TIMES(buffer);
	return ((TIMES(buffer) - first) * (1000000L/HZ));
}
