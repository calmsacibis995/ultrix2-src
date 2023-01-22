#ifndef lint
static	char	*sccsid = "@(#)out_arg.c	1.1	(ULTRIX)	1/8/85";
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
# include	<aux.h>


/*
**  OUTPUT ARGUMENTS GLOBAL INITIALIZATION
*/

struct out_arg	Out_arg =		/* output arguments */
{
	6,		/* c0width */
	6,		/* i1width */
	6,		/* i2width */
	13,		/* i4width */
	10,		/* f4width */
	10,		/* f8width */
	3,		/* f4prec */
	3,		/* f8prec */
	'n',		/* f4style */
	'n',		/* f8style */
	66,		/* linesperpage */
	'|',		/* coldelim */
};
