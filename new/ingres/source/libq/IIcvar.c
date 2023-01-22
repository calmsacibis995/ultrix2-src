#ifndef lint
static	char	*sccsid = "@(#)IIcvar.c	1.1	(ULTRIX)	1/8/85";
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
**	IIcvar -- write C variable values to parser
**
**
**		IIcvar is used to write the contents
**		of a C-variable to the quel parser.
**
**		Floats are converted to doubles first.
**
*/

IIcvar(obj, type, len)
char	*obj;
int	type;
int	len;
{
	register int		length;
	register ANYTYPE	*addr;
	int			t;
	double			d;

	t = type;	/* copy type of symbol */
	length = len;	/* and its length */
	addr = (ANYTYPE *) obj;	/* and a pointer to it */

	switch (t)
	{

	  case opFLOAT:
		/* convert from f4 to f8 */
		d = addr->f4type;
		addr = (ANYTYPE *) &d;
		length = sizeof d;
		t = opDOUBLE;
		break;

	  case opSTRING:
		length = IIlength(addr) + 1;	/* length includes null byte at end */

	  case opSHORT:
	  case opLONG:
	  case opDOUBLE:
		break;

	  default:
		IIsyserr("IIcvar:bad type %d", t);
	}


#	ifdef xETR1
	if (IIdebug)
		printf("IIcvar:type %d, length %d\n", t, length);
#	endif

	IIpb_put(&t, 1, &IIpb);
	IIpb_put(addr, length, &IIpb);
}
