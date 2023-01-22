/*
 *		@(#)pmon.h	1.1	(ULTRIX)	1/8/85
 */

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

/*
**  MONITOR.H -- structures dealing with performance monitoring
**
**	Version:
**		@(#)pmon.h	7.1	2/5/81
*/

struct monitor
{
	long	mon_utime;	/* user time in milliseconds */
	long	mon_stime;	/* system time in milliseconds */
	long	mon_pread;	/* pages read */
	long	mon_pwrit;	/* pages written */
	long	mon_cread;	/* catalog pages read */
	long	mon_cwrit;	/* catalog pages written */
	long	mon_xread;	/* ISAM/BTREE index pages read */
};
