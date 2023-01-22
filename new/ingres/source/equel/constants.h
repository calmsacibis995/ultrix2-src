/*
 *		@(#)constants.h	1.1	(ULTRIX)	1/8/85
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
**  CONSTANTS.H -- manifest constants, operand and operator codes
**
**	Defines:
**		op_codes for operands
**		Opflag's domain set
**		Type_spec's domain set
**
**	Version:
**		@(#)constants.h	7.2	10/27/81
*/

/*
** 	Manifest constants used throughout Equel
*/


# define	CONTINUE	1	/* "loop" flag for yylex */
# define	MAXNAME		13	/* maximum length for equel 
					 * identifiers (or keywords) 
					 */
# define	MAXSTRING	255	/* maximum length for equel strings */
# define	FILLCNT		110	/* length to fill lines when in Fillmode */


/* debugging info conditional compilation flag */
# define	xDEBUG			/* on for "-c" and "-v" flags */
# define	YYDEBUG			/* must ALWAYS be on-used in yyparse()*/


/* 
**	Character types [cmap.c] 
*/

# define	EOF_TOK		0	/* end of parse input too */
# define	ALPHA		1	/* alphabetic or '_' */
# define	NUMBR		2	/* numeric */
# define	OPATR		3	/* other non control characters */
# define	PUNCT		4	/* white space */
# define	CNTRL		5	/* control-characters */

/*
**	Modes for Lastc in w_op() and w_key() [prtout.c] 
*/

# define	OPCHAR		0	/* last character 
					 * printed was an operator 
					 */
# define	KEYCHAR		1	/* last was alphanumeric */


/*
 * Modes used in parser actions to distinguish contexts in which
 * the same syntax applies. Opflag is set to these modes.
 *
 * (There are some modes that are never referenced, but are useful
 * for extension).
 */

	/* quel statements */

# define	mdAPPEND	1	
# define	mdCOPY		2
# define	mdCREATE	3
# define	mdDEFINE	4
# define	mdDELETE	5
# define	mdDESTROY	6
# define	mdHELP		7
# define	mdINDEX		8
# define	mdINTEGRITY	9
# define	mdMODIFY	10
# define	mdPRINT		11
# define	mdRANGE		12
# define	mdREPLACE	13
# define	mdRETRIEVE	14
# define	mdSAVE		15
# define	mdVIEW		16
# define	mdPROT		17
	
	/* statements particular to Equel */

# define	mdDECL		16	/* C - declaration */
# define	mdCTLELM	17	/* left hand side of target list element
					 * in "retrieve" to C-variables
					 */
# define	mdEXIT		18	/* ## exit */
# define	mdINGRES	19	/* ## ingres */
# define	mdTUPRET	20	/* "retrieve" w/o an "into" */
# define	mdFILENAME	21	/* used in "copy" statement */







/* define	typTYPE		xx		/*   c types "Type_spec"  */
# define			opSHORT		1
# define			opFLOAT		2
# define			opSTRING	3
# define			opDOUBLE	4
# define			opCHAR		5
# define			opLONG		6
# define			opIDSTRING	7
# define			opSTRUCT	8
# ifdef PDP
# define			opINT		opSHORT
# else PDP
# define			opINT		opLONG
# endif PDP


/* define	typALLOC	xx		/*  c var allocation types  */
# define			opAUTO		0
# define			opSTATIC	1
# define			opEXTERN	2
# define			opREGISTER	3
