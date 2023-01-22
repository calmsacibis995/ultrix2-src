#ifndef lint
static	char	*sccsid = "@(#)cleanrel.c	1.1	(ULTRIX)	1/8/85";
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

# include <ingres.h>
# include <aux.h>
# include <access.h>


/*
** CLEANREL --
**	If there are any buffers being used by the relation described
**	in the descriptor struct, flush and zap the buffer.
**	This will force a UNIX disk read the next time the relation
**	is accessed which is useful to get the most up-to-date
**	information from a file that is being updated by another
**	program.
*/

cleanrel(d)
DESC	*d;
{
	/* flush and reset all pages of this rel */
	return (flush_rel(d, TRUE));
}
