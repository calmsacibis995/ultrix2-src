#ifndef lint
static	char	*sccsid = "@(#)null_fn.c	1.1	(ULTRIX)	1/8/85";
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



/*
**  NULL_FN -- A null function
**
**	This routine does absolutely nothing at all.
**
**	Algorithm:
**		none.
**
**	Parameters:
**		none.
**
**	Returns:
**		zero
**
**	Side Effects:
**		none.
**
**	Defined Constants:
**		none.
**
**	Defines:
**		null_fn
**
**	Requires:
**		nothing.
**
**	Required By:
**		Lots (this system doesn't do much).
**
**	Files:
**		none.
**
**	Compilation Flags:
**		none.
**
**	Trace Flags:
**		none.
**
**	Diagnostics:
**		none.
**
**	Syserrs:
**		none.
**
**	Deficiencies:
**		It should do nothing faster.
**
**	History:
**		5/12/80 (eric & polly) -- written.
**
**	Version:
**		7.1
**
**	WARNING:
**		Do not use this routine if you want to do something.
*/

null_fn()
{
	return (0);
}
