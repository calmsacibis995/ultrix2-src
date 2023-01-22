#ifndef lint
static char	*sccsid = "@(#)TRdata.c	1.2	(ULTRIX)	1/24/86";
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
*	Based on:	TRdata.c	5.1		6/4/85
*
*************************************************************************/

#include "whoami.h"
#include "0.h"
#ifdef	PI1
#ifdef	DEBUG
char	*trnames[] =
{
	0,
	"MINUS",
	"MOD",
	"DIV",
	"DIVD",
	"MULT",
	"ADD",
	"SUB",
	"EQ",
	"NE",
	"LT",
	"GT",
	"LE",
	"GE",
	"NOT",
	"AND",
	"OR",
	"ASGN",
	"PLUS",
	"IN",
	"LISTPP",
	"PDEC",
	"FDEC",
	"PVAL",
	"PVAR",
	"PFUNC",
	"PPROC",
	"NIL",
	"STRNG",
	"CSTRNG",
	"PLUSC",
	"MINUSC",
	"ID",
	"INT",
	"FINT",
	"CINT",
	"CFINT",
	"TYPTR",
	"TYPACK",
	"TYSCAL",
	"TYRANG",
	"TYARY",
	"TYFILE",
	"TYSET",
	"TYREC",
	"TYFIELD",
	"TYVARPT",
	"TYVARNT",
	"CSTAT",
	"BLOCK",
	"BSTL",
	"LABEL",
	"PCALL",
	"FCALL",
	"CASE",
	"WITH",
	"WHILE",
	"REPEAT",
	"FORU",
	"FORD",
	"GOTO",
	"IF",
	"ASRT",
	"CSET",
	"RANG",
	"VAR",
	"ARGL",
	"ARY",
	"FIELD",
	"PTR",
	"WEXP",
	"PROG",
	"BINT",
	"CBINT",
	"IFEL",
	"IFX",
	"TYID",
	"COPSTR",
	"BOTTLE",
	"RFIELD",
	"FLDLST",
	"LAST"
};
#endif
#endif

#ifndef PC
#ifndef OBJ
char	*trdesc[] =
{
	0,
	"dp",
	"dpp",
	"dpp",
	"dpp",
	"dpp",
	"dpp",
	"dpp",
	"dpp",
	"dpp",
	"dpp",
	"dpp",
	"dpp",
	"dpp",
	"dp",
	"dpp",
	"dpp",
	"npp",
	"dp",
	"dpp",
	"pp",
	"n\"pp",
	"n\"pp",
	"pp",
	"pp",
	"pp",
	"p",
	"d",
	"dp",
	"p",
	"p",
	"p",
	"p",
	"dp",
	"dp",
	"p",
	"p",
	"np",
	"np",
	"np",
	"npp",
	"npp",
	"np",
	"np",
	"np",
	"pp",
	"nppp",
	"npp",
	"npp",
	"np",
	"np",
	"n\"p",
	"n\"p",
	"n\"p",
	"npp",
	"npp",
	"npp",
	"npp",
	"nppp",
	"nppp",
	"n\"",
	"nppp",
	"np",
	"dp",
	"pp",
	"n\"p",
	"p",
	"p",
	"pp",
	"",
	"ppp",
	"n\"pp",
	"dp",
	"p",
	"nppp",
	"nppp",
	"np",
	"s",
	"nnnnn",
	"npp",
	"npp",
	"x"
};
#endif
#endif
char	*opnames[] =
{
	0,
	"unary -",
	"mod",
	"div",
	"/",
	"*",
	"+",
	"-",
	"=",
	"<>",
	"<",
	">",
	"<=",
	">=",
	"not",
	"and",
	"or",
	":=",
	"unary +",
	"in"
};
