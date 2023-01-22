#ifndef lint
static	char	*sccsid = "@(#)IIretrieve.c	1.1	(ULTRIX)	1/8/85";
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
**	IIretrieve is called once for each element
**	in the target list of a retrieve.
**
**	The purpose is to set up the IIretsym structure
**	for IIgettup.
*/

IIretrieve(addr, type)
char	*addr;
int	type;

{
	register struct retsym	*sym;
	register int		t, l;

	sym = &IIretsym[IIndomains++];
	switch (type)
	{

	  case opSHORT:
		t = INT;
		l = 2;
		break;

	  case opLONG:
		t = INT;
		l = 4;
		break;

	  case opFLOAT:
		t = FLOAT;
		l = 4;
		break;

	  case opDOUBLE:
		t = FLOAT;
		l = 8;
		break;

	  case opSTRING:
		t = CHAR;
		l = 255;	/* with the current implementation the length is not known */
		break;

	  default:
		IIsyserr("retrieve:bad type %d", type);
	}
	sym->type = t;
	sym->len = l;
	sym->addr = addr;
#	ifdef xETR1
	if (IIdebug)
		printf("domain %d type %d len %d\n", IIndomains, t, l);
#	endif
}
