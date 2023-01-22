/*
 *		@(#)ctype.h	1.4	(ULTRIX)	10/21/85
 *		ctype.h	4.1	83/05/03
 */

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
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
 *			Modification History				*
 *									*
 *	David L Ballenger, 11-Oct-1985					*
 * 002	Remove the defintion of the toupper() and tolower() macros so	*
 *	that the toupper() and tolower() routines will be used as	*
 *	documented.  The _toupper() and _tolower() macros are still	*
 *	provided.  Also add _B so that isgraph() and ispunct() can	*
 *	exclude space (ie. blank) and isprint() can include it.  	*
 *									*
 *	David L Ballenger, 28-Mar-1985					*
 * 0001 Add definitions for System V compatibility			*
 *									*
 ************************************************************************/



#define	_U	0001	/* Upper case */
#define	_L	0002	/* Lower case */
#define	_N	0004	/* Numeral (digit) */
#define	_S	0010	/* Spacing character */
#define _P	0020	/* Punctuation */
#define _C	0040	/* Control character */
#define _X	0100	/* Hexadecimal */
#define _B	0200	/* Blank */

extern	char	_ctype_[];

#define	isalpha(c)	((_ctype_+1)[c]&(_U|_L))
#define	isupper(c)	((_ctype_+1)[c]&_U)
#define	islower(c)	((_ctype_+1)[c]&_L)
#define	isdigit(c)	((_ctype_+1)[c]&_N)
#define	isxdigit(c)	((_ctype_+1)[c]&(_N|_X))
#define	isspace(c)	((_ctype_+1)[c]&_S)
#define ispunct(c)	((_ctype_+1)[c]&_P)
#define isalnum(c)	((_ctype_+1)[c]&(_U|_L|_N))
#define isprint(c)	((_ctype_+1)[c]&(_P|_U|_L|_N|_B))
#define isgraph(c)	((_ctype_+1)[c]&(_P|_U|_L|_N))
#define iscntrl(c)	((_ctype_+1)[c]&_C)
#define isascii(c)	((unsigned)(c)<=0177)
#define _toupper(c)	((c)-'a'+'A')
#define _tolower(c)	((c)-'A'+'a')
#define toascii(c)	((c)&0177)
