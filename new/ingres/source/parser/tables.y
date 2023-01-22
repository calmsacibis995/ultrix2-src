/*
 *		@(#)tables.y	1.2	(ULTRIX)	1/22/85
 */

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
**	SCANNER KEYWORD TABLES
**
**	Keywords, tokens, and opcode tuples are included in this file
**	The keyword table MUST be in sorted order.
**	The operator table does not need to be sorted
**
**	Version:
**		@(#)tables.y	7.1	2/5/81
*/
struct optab	Keyword[] =
{
	"abs",			FOP,			opABS,
	"all",			ALL,			0,
	"and",			LBOP,			AND,
	"any",			AGOP, 			opANY,
	"append",		APPEND,			GOVAL,
	"ascii",		FOP,			opASCII,
	"at",			AT,			0,
	"atan",			FOP,			opATAN,
	"avg",			AGOP,			opAVG,
	"avgu",			AGOP,			opAVGU,
	"by",			BY,			opEQ,
	"concat", 		FBOP,			opCONCAT,
	"copy",			COPY,			GOVAL,
	"cos",			FOP,			opCOS,
	"count",		AGOP,			opCOUNT,
	"countu",		AGOP,			opCOUNTU,
	"create",		CREATE,			GOVAL,
	"define",		DEFINE,			GOVAL,
	"delete",		DELETE,			GOVAL,
	"destroy",		DESTROY,		GOVAL,
#	ifdef	DISTRIB
	"distribute",		DISTRIBUTE,		0,
	"distributed",		DISTRD,			0,
#	endif
	"exp",			FOP,			opEXP,
	"float4",		FOP,			opFLOAT4,
	"float8",		FOP,			opFLOAT8,
	"from",			FROM,			0,
# 	ifdef xV6_UNIX
	"gamma",		FOP,			opGAMMA,
# 	endif
	"help",			HELP,			GOVAL,
	"in",			IN,			0,
	"index",		INDEX,			GOVAL,
	"int1",			FOP,			opINT1,
	"int2",			FOP,			opINT2,
	"int4",			FOP,			opINT4,
	"integrity",		INTEGRITY,		0,
	"into",			INTO,			0,
	"is",			IS,			opEQ,
	"log",			FOP,			opLOG,
	"max",			AGOP,			opMAX,
	"min",			AGOP,			opMIN,
	"mod",			FBOP,			opMOD,
	"modify",		MODIFY,			GOVAL,
	"not",			LUOP,			opNOT,
	"of",			OF,			0,
	"on",			ON,			0,
	"onto",			ONTO,			0,
	"or",			LBOP,			OR,
	"permit",		PERMIT,			0,
	"print",		PRINT,			GOVAL,
	"range",		RANGE,			GOVAL,
	"replace",		REPLACE,		GOVAL,
	"retrieve",		RETRIEVE,		GOVAL,
	"save",			SAVE,			GOVAL,
	"sin",			FOP,			opSIN,
	"sqrt",			FOP,			opSQRT,
	"sum",			AGOP,			opSUM,
	"sumu",			AGOP,			opSUMU,
	"to",			TO,			0,
	"unique",		UNIQUE,			0,
	"until",		UNTIL,			0,
	"view",			VIEW,			0,
	"where",		WHERE,			0,
};
int	Keyent =		sizeof Keyword / sizeof Keyword[0];

struct optab	Optab[] =
{
	/* PUNCTUATION */
	",",			COMMA,			0,
	"(",			LPAREN,			0,
	".",			PERIOD,			0,
	")",			RPAREN,			0,
	"\"",			SCONST,			0,
	"/*",			BGNCMNT,		0,
	"*/",			ENDCMNT,		0,
	":",			COLON,			0,

	/* UNARY ARITHMETIC OPERATORS */
	"+",			UAOP,			opADD,
	"-",			UAOP,			opSUB,

	/* BINARY ARITHMETIC OPERATORS */
	"*",			BAOP,			opMUL,
	"/",			BAOP,			opDIV,
	"**",			BAOPH,			opPOW,

	/* BOUNDS OPERATORS */
	">",			BDOP,			opGT,
	">=",			BDOP,			opGE,
	"<",			BDOP,			opLT,
	"<=",			BDOP,			opLE,

	/* EQUALITY OPERATORS */
	"!=",			EOP,			opNE,
	"=",			IS,			opEQ,
	0
};

/* constant operators table for lookup */
struct constop	Coptab[] =
{
	"dba",		opDBA,		CHAR,		2,
	"usercode",	opUSERCODE,	CHAR,		2,
	0,		0,		0,		0,
	"date",		opDATE,		'?',		'?',
	"time",		opDATE,		'?',		'?',
	0,		0,		0,		0
};


/*
** Tokens
** a structure initialized here to contain the
** yacc generated tokens for the indicated
** terminal symbols.
*/
struct special	Tokens =
{
	SCONST,
	BGNCMNT,
	ENDCMNT,
	I2CONST,
	I4CONST,
	F4CONST,
	F8CONST,
	NAME
};
