#ifndef lint
static	char	*sccsid = "@(#)ingresname.c	1.1	(ULTRIX)	1/8/85";
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


/*
**  MAKE INGRES FILE NAME
**
**	The null-terminated string 'iname' is converted to a
**	file name as used by the ingres relations.  The name
**	of the relation is padded out to be MAXNAME bytes long,
**	and the two-character id 'id' is appended.  The whole
**	thing will be null-terminated and put into 'outname'.
**
**	'Outname' must be at least MAXNAME + 3 bytes long.
*/

ingresname(iname, id, outname)
char	*iname;
char	*id;
char	*outname;
{
	register char	*p;
	extern char	*pmove();

	p = outname;
	p = pmove(iname, p, MAXNAME, ' ');
	bmove(id, p, 2);
	p[2] = NULL;
}
