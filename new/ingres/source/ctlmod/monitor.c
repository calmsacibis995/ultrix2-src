#ifndef lint
static	char	*sccsid = "@(#)monitor.c	1.1	(ULTRIX)	1/8/85";
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

# include	<useful.h>
# include	<opsys.h>
# include	<pmon.h>


/*
**  MARKPERF -- mark the performance info in a monitor struct
**
**	At any point, there is a monitor struct representing the
**	current object running.  This is stored in the static
**	variable "curmon".  This call essentially does a "context
**	switch" to the structure passed as the argument.
**
**	Parameters:
**		mbuf -- a pointer to a monitor struct.
**
**	Returns:
**		a pointer to the previous monitor struct.
**
**	Side Effects:
**		none
*/

# define	HZ	60	/* speed of clock in ticks/second */

struct tbuffer
{
# ifdef xV7_UNIX
	long	tms_utime;
	long	tms_stime;
# else
	short	tms_utime;
	short	tms_stime;
# endif
	long	tms_cutime;
	long	tms_cstime;
};

struct monitor *
markperf(mbuf)
register struct monitor	*mbuf;
{
	struct tbuffer		tbuf;
	register long		ut;
	register long		st;
	static struct tbuffer	baset;
	static struct monitor	*curmon;
	register struct monitor	*oldmon;

	times(&tbuf);

	ut = tbuf.tms_utime + tbuf.tms_cutime - baset.tms_utime - baset.tms_cutime;
	st = tbuf.tms_stime + tbuf.tms_cstime - baset.tms_stime - baset.tms_cstime;
	oldmon = curmon;
	if (oldmon != NULL)
	{
		oldmon->mon_utime += ut;
		oldmon->mon_stime += st;
	}
	curmon = mbuf;
	bmove(&tbuf, &baset, sizeof baset);
	return (oldmon);
}
/*
**  ADD_MON -- "add" two monitor structs
**
**	The logical sum of two monitor structs is created
**
**	Parameters:
**		a -- the first monitor struct
**		b -- the second monitor struct; gets the result.
**
**	Returns:
**		none (value is returned through b)
**
**	Side Effects:
**		none.
*/

add_mon(a, b)
register struct monitor	*a;
register struct monitor	*b;
{
	b->mon_utime += a->mon_utime;
	b->mon_stime += a->mon_stime;
}
/*
**  CVT_TIME -- convert time for output
**
**	Converts a time in ticks to a string (in seconds) for
**	printing.
**
**	Parameters:
**		t -- time in ticks
**
**	Returns:
**		pointer to string suitable for printing or framing.
**
**	Side Effects:
**		previous return value is clobbered.
*/

char *
cvt_time(t)
long	t;
{
	static char	buf[30];

	sprintf(buf, "%.3f", ((float) t) / ((float) HZ));
	return (buf);
}
