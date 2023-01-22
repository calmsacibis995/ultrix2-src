#ifndef lint
static char	*sccsid = "@(#)yyerror.c	1.2	(ULTRIX)	1/27/86";
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
*	Based on:	yyerror.c	5.1		6/5/85
*
*************************************************************************/

#include "whoami.h"
#include "0.h"
#include "tree_ty.h"	/* must be included for yy.h */
#include "yy.h"

/*
 * Yerror prints an error
 * message and then returns
 * NIL for the tree if needed.
 * The error is flagged on the
 * current line which is printed
 * if the listing is turned off.
#ifdef PXP
 *
 * As is obvious from the fooling around
 * with fout below, the Pascal system should
 * be changed to use the new library "lS".
#endif
 */
/*VARARGS*/
yerror(s, a1, a2, a3, a4, a5)
	char *s;
	char *a1, *a2, *a3, *a4, *a5;
{
#ifdef PI
	char buf[256];
#endif
	register int i;
	static yySerrs;
#ifdef PXP
	int ofout;
#endif

	if (errpfx == 'w' && opt('w') != 0) {
		errpfx = 'E';
		return;
	}
#ifdef PXP
	flush();
	ofout = fout[0];
	fout[0] = errout;
#endif
	yyResume = 0;
#ifdef PI
	geterr((int) s, buf);
	s = buf;
#endif
	yysync();
	pchr(errpfx);
	pchr(' ');
	for (i = 3; i < yyecol; i++)
		pchr('-');
	printf("^--- ");
/*
	if (yyecol > 60)
		printf("\n\t");
*/
	printf(s, a1, a2, a3, a4, a5);
	pchr('\n');
	if (errpfx == 'E')
#ifdef PI
		eflg = TRUE, codeoff();
#endif
#ifdef PXP
		eflg = TRUE;
#endif
	errpfx = 'E';
	yySerrs++;
	if (yySerrs >= MAXSYNERR) {
		yySerrs = 0;
		yerror("Too many syntax errors - QUIT");
		pexit(ERRS);
	}
#ifdef PXP
	flush();
	fout[0] = ofout;
	return (0);
#endif
}

/*
 * A bracketing error message
 */
brerror(where, what)
	int where;
	char *what;
{

	if (where == 0) {
		line = yyeline;
		setpfx(' ');
		error("End matched %s on line %d", what, (char *) where);
		return;
	}
	if (where < 0)
		where = -where;
	yerror("Inserted keyword end matching %s on line %d", what, (char *) where);
}
