#ifndef lint
static	char	*sccsid = "@(#)getkey.c	1.1	(ULTRIX)	1/8/85";
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

# include	<stdio.h>
# include	"constants.h"
# include	"globals.h"



/*
**  GETKEY -- Get the optab entry for a keyword
**
**	Performs a binary search through Kwrdtab
**	for a given keyword.
**
**	Parameters:
**		key -- char * to the keywords character 
**			representation.
**
**	Returns:
**		a pointer to the optab struct node for that 
**		keyword, or 0 if not found.
*/



struct optab
*getkey(key)
char		*key;
{
	register struct optab	*op;
	int			top, bot;
	register int		k;
	extern int		Kwrdnum;

	op = Kwrdtab;
	bot = 0;
	top = Kwrdnum - 1;
	do 
	{
		k = (top + bot) / 2;
		switch (scompare(key, 0, op [k].op_term, 0))
		{

		  case 1 :
			bot = k + 1;
			break;
		
		  case 0 :
			return (&op [k]);

		  case -1 :
			top = k - 1;
			break;
		}
	}  while (bot <= top);
	return (0);
}
