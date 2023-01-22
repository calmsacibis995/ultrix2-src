#ifndef lint
static char	*sccsid = "@(#)string.c	1.2	(ULTRIX)	1/27/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
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

/************************************************************************
*
*			Modification History
*
*		David Metsky,	20-Jan-86
*
* 001	Replaced old version with BSD 4.3 version as part of upgrade
*
*	Based on:	string.c	5.1		6/5/85
*
*************************************************************************/

#include "whoami.h"
#include "0.h"
#ifndef PI01
#ifndef PXP
#include "send.h"
#endif
#endif

/*
 * STRING SPACE DECLARATIONS
 *
 * Strng is the base of the current
 * string space and strngp the
 * base of the free area therein.
 * Strp is the array of descriptors.
 */
#ifndef PI0
STATIC	char strings[STRINC];
STATIC	char *strng = strings;
STATIC	char *strngp = strings;
#else
char	*strng, *strngp;
#endif
#ifndef PI01
#ifndef PXP
STATIC	char *strp[20];
STATIC	char **stract strp;
int	strmax;
#endif
#endif

#ifndef PI01
#ifndef PXP
#ifndef PI0
initstring()
#else
initstring(strings)
	char *strings;
#endif
{

	*stract++ = strings;
#ifdef PI0
	strng = strngp = strings;
#endif
	strmax = STRINC * 2;
}
#endif
#endif

/*
 * Copy a string into the string area.
 */
char *
savestr(cp)
	register char *cp;
{
	register int i;

	i = strlen(cp) + 1;
	if (strngp + i >= strng + STRINC) {
		strngp = malloc(STRINC);
		if (strngp == 0) {
			yerror("Ran out of memory (string)");
			pexit(DIED);
		}
#ifndef PI01
#ifndef PXP
		*stract++ = strngp;
		strmax =+ STRINC;
#endif
#endif
		strng = strngp;
	}
	(void) pstrcpy(strngp, cp);
	cp = strngp;
	strngp = cp + i;
#ifdef PI0
	send(RSTRING, cp);
#endif
	return (cp);
}

#ifndef PI1
#ifndef PXP
char *
esavestr(cp)
	char *cp;
{

#ifdef PI0
	send(REVENIT);
#endif
	strngp = ( (char *) ( ( (int) (strngp + 1) ) &~ 1 ) );
	return (savestr(cp));
}
#endif
#endif

#ifndef PI01
#ifndef PXP
soffset(cp)
	register char *cp;
{
	register char **sp;
	register int i;

	if (cp == NIL || cp == OCT || cp == HEX)
		return (-cp);
	for (i = STRINC, sp = strp; sp < stract; sp++) {
		if (cp >= *sp && cp < (*sp + STRINC))
			return (i + (cp - *sp));
		i =+ STRINC;
	}
	i = nlfund(cp);
	if (i != 0)
		return (i);
	panic("soffset");
}
#ifdef PI1
sreloc(i)
	register int i;
{

	if (i == 0 || i == -OCT || i == -HEX)
		return (-i);
	if (i < STRINC) {
		if (i >= INL)
			panic("sreloc INL");
		i = nl[i].symbol;
		if (i == 0)
			panic("sreloc nl[i]");
		return (i);
	}
	if (i > strmax || i < 0)
		panic("sreloc");
	return (strp[(i / STRINC) - 1] + (i % STRINC));
}

evenit()
{

	strngp = (strngp + 1) &~ 1;
}
#endif
#endif
#endif
