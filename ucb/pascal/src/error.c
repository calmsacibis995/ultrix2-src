#ifndef lint
static char	*sccsid = "@(#)error.c	1.2	(ULTRIX)	1/27/86";
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
*	Based on:	error.c		5.1		6/5/85
*
*************************************************************************/

#include "whoami.h"
#include "0.h"
#ifndef PI1
#include "tree_ty.h"		/* must be included for yy.h */
#include "yy.h"
#endif

char	errpfx	= 'E';
extern	int yyline;
/*
 * Panic is called when impossible
 * (supposedly, anyways) situations
 * are encountered.
 * Panic messages should be short
 * as they do not go to the message
 * file.
 */
panic(s)
	char *s;
{

#ifdef DEBUG
#ifdef PI1
	printf("Snark (%s) line=%d\n", s, line);
	abort();
#else
	printf("Snark (%s) line=%d, yyline=%d\n", s, line, yyline);
	abort () ;	/* die horribly */
#endif
#endif
#ifdef PI1
	Perror( "Snark in pi1", s);
#else
	Perror( "Snark in pi", s);
#endif
	pexit(DIED);
}

/*
 * Error is called for
 * semantic errors and
 * prints the error and
 * a line number.
 */

/*VARARGS1*/

error(a1, a2, a3, a4, a5)
	register char *a1;
{
	char errbuf[256]; 		/* was extern. why? ...pbk */
	register int i;

	if (errpfx == 'w' && opt('w') != 0) {
		errpfx = 'E';
		return;
	}
	Enocascade = FALSE;
	geterr((int) a1, errbuf);
	a1 = errbuf;
	if (line < 0)
		line = -line;
#ifndef PI1
	if (opt('l'))
		yyoutline();
#endif
	yysetfile(filename);
	if (errpfx == ' ') {
		printf("  ");
		for (i = line; i >= 10; i /= 10)
			pchr( ' ' );
		printf("... ");
	} else if (Enoline)
		printf("  %c - ", errpfx);
	else
		printf("%c %d - ", errpfx, line);
	printf(a1, a2, a3, a4, a5);
	if (errpfx == 'E')
#ifndef PI0
		eflg = TRUE, codeoff();
#else
		eflg = TRUE;
#endif
	errpfx = 'E';
	if (Eholdnl)
		Eholdnl = FALSE;
	else
		pchr( '\n' );
}

/*VARARGS1*/

cerror(a1, a2, a3, a4, a5)
    char *a1;
{

	if (Enocascade)
		return;
	setpfx(' ');
	error(a1, a2, a3, a4, a5);
}

#ifdef PI1

/*VARARGS*/

derror(a1, a2, a3, a4, a5)
    char *a1, *a2, *a3, *a4, *a5;
{

	if (!holdderr)
		error(a1, a2, a3, a4, a5);
	errpfx = 'E';
}

char	*lastname, printed, hadsome;

    /*
     *	this yysetfile for PI1 only.
     *	the real yysetfile is in yyput.c
     */
yysetfile(name)
	char *name;
{

	if (lastname == name)
		return;
	printed =| 1;
	gettime( name );
	printf("%s  %s:\n" , myctime( &tvec ) , name );
	lastname = name;
}
#endif
