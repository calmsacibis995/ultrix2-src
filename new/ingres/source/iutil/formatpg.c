#ifndef lint
static	char	*sccsid = "@(#)formatpg.c	1.1	(ULTRIX)	1/8/85";
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
# include	<access.h>


formatpg(d, n)
DESC	*d;
long	n;
{
	struct accbuf	buf;
	register char	*p;
	extern long	lseek();

	if (Acc_head == 0)
		acc_init();
	if (lseek(d->relfp, 0l, 0) == -1)
		return (-2);
	buf.rel_tupid = d->reltid.ltid;
	buf.filedesc = d->relfp;
	for (p = (char *) &buf; p <= (char *) buf.linetab; p++)
		*p = NULL;
	buf.nxtlino = 0;
	buf.linetab[0] = (int) buf.firstup - (int) &buf;
	buf.ovflopg = 0;
	for (buf.mainpg = 1; buf.mainpg < n; (buf.mainpg)++)
	{
		if (write(buf.filedesc, (char *) &buf, PGSIZE) != PGSIZE)
			return (-3);
	}
	buf.mainpg = 0;
	if (write(buf.filedesc, (char *) &buf, PGSIZE) != PGSIZE)
		return (-4);
	Accuwrite += n;
	return (0);
}
