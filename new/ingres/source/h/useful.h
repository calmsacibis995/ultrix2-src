/*
 *		@(#)useful.h	1.1	(ULTRIX)	1/8/85
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
**  USEFUL.H -- useful stuff.
**
**	Version:
**		@(#)useful.h	7.1	2/5/81
*/

# ifndef TRUE
# define TRUE		1	/* logical one, true, yes, ok, etc.*/
# define FALSE		0	/* logical zero, false, no, nop, etc. */

typedef char	bool;		/* the boolean type */
# endif TRUE

# ifndef NULL
# define NULL		0	/* the null pointer */
# endif NULL

# ifndef bitset
# define	bitset(bit, word)	((bit) & (word))
# define	setbit(bit, word)	word |= bit
# define	clrbit(bit, word)	word &= ~bit
# endif bitset

# ifndef min
# define	min(a, b)	(((a) < (b))? (a): (b))
# define	max(a, b)	(((a) > (b))? (a): (b))
# endif min
