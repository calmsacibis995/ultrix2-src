#ifndef lint
static char	*sccsid = "@(#)yypanic.c	1.2	(ULTRIX)	1/27/86";
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
*	Based on:	yypanic.c	5.1		6/5/85
*
*************************************************************************/

#include "whoami.h"
#include "0.h"
#include "tree_ty.h"	/* must be included for yy.h */
#include "yy.h"

struct yytok oldpos;
/*
 * The routine yyPerror coordinates the panic when
 * the correction routines fail. Three types of panics
 * are possible - those in a declaration part, those
 * in a statement part, and those in an expression.
 *
 * Declaration part panics consider insertion of "begin",
 * expression part panics will stop on more symbols.
 * The panics are otherwise the same.
 *
 * ERROR MESSAGE SUPPRESSION STRATEGY: August 11, 1977
 *
 * If the parser has not made at least 2 moves since the last point of
 * error then we want to suppress the supplied error message.
 * Otherwise we print it.
 * We then skip input up to the next solid symbol.
 */
yyPerror(cp, kind)
	char *cp;
	register int kind;
{
	register int ishifts, brlev;

	copy((char *) (&oldpos), (char *) (&Y), sizeof oldpos);
	brlev = 0;
	if (yychar < 0)
		yychar = yylex();
	for (ishifts = yyshifts; ; yychar = yylex(), yyshifts++)
		switch (yychar) {
			case YILLCH:
				yerror("Illegal character");
				if (ishifts == yyshifts)
					yyOshifts = 0;
				continue;
			case YEOF:
				if (kind == PDECL) {
					/*
					 * we have paniced to end of file
					 * during declarations. Separately
					 * compiled segments can syntactically
					 * exit without any error message, so
					 * we force one here.
					 */
					yerror(cp);
					continuation();
					yyunexeof();
				}
				goto quiet;
			case ';':
				if (kind == PPROG)
					continue;
				if (kind == PDECL)
					yychar = yylex();
				goto resume;
			case YEND:
				if (kind == PPROG)
					continue;
			case YPROCEDURE:
			case YFUNCTION:
				goto resume;
			case YLABEL:
			case YTYPE:
			case YCONST:
			case YVAR:
				if (kind == PSTAT) {
					yerror("Declaration found when statement expected");
					goto quiet;
				}
			case YBEGIN:
				goto resume;
			case YFOR:
			case YREPEAT:
			case YWHILE:
			case YGOTO:
			case YIF:
				if (kind != PDECL)
					goto resume;
				yerror("Expected keyword begin after declarations, before statements");
				unyylex(&Y);
				yychar = YBEGIN;
				yylval = nullsem(YBEGIN);
				goto quiet;
			case YTHEN:
			case YELSE:
			case YDO:
				if (kind == PSTAT) {
					yychar = yylex();
					goto resume;
				}
				if (kind == PEXPR)
					goto resume;
				continue;
			case ')':
			case ']':
				if (kind != PEXPR)
					continue;
				if (brlev == 0)
					goto resume;
				if (brlev > 0)
					brlev--;
				continue;
			case '(':
			case '[':
				brlev++;
				continue;
			case ',':
				if (brlev != 0)
					continue;
			case YOF:
			case YTO:
			case YDOWNTO:
				if (kind == PEXPR)
					goto resume;
				continue;
#ifdef PI
			/*
			 * A rough approximation for now
			 * Should be much more lenient on suppressing
			 * warnings.
			 */
			case YID:
				syneflg = TRUE;
				continue;
#endif
		}
resume:
	if (yyOshifts >= 2) {
		if (yychar != -1)
			unyylex(&Y);
		copy((char *) (&Y), (char *) (&oldpos), sizeof Y);
		yerror(cp);
		yychar = yylex();
	}
quiet:
	if (yyshifts - ishifts > 2 && opt('r')) {
		setpfx('r');
		yerror("Parsing resumes");
	}
	/*
	 * If we paniced in the statement part,
	 * and didn't stop at a ';', then we insert
	 * a ';' to prevent the recovery from immediately
	 * inserting one and complaining about it.
	 */
	if (kind == PSTAT && yychar != ';') {
		unyylex(&Y);
		yyshifts--;
		yytshifts--;
		yychar = ';';
		yylval = nullsem(';');
	}
}
