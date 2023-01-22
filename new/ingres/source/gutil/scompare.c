#ifndef lint
static	char	*sccsid = "@(#)scompare.c	1.1	(ULTRIX)	1/8/85";
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



/*
**  STRING COMPARE
**
**	The strings 'a_ptr' and 'b_ptr' are compared.  Blanks are
**	ignored.  The first string may be no longer than 'a_len'
**	bytes, and the second string may be no longer than 'b_len'
**	bytes.  If either length is zero, it is taken to be very
**	long.  A null byte also terminates the scan.
**
**	Compares are based on the ascii ordering.
**
**	Shorter strings are less than longer strings.
**
**	Return value is positive one for a > b, minus one for a < b,
**	and zero for a == b.
**
**	Examples:
**		"abc" > "ab"
**		"  a bc  " == "ab  c"
**		"abc" < "abd"
*/

scompare(a_ptr, a_len, b_ptr, b_len)
char	*a_ptr;
int	a_len;
char	*b_ptr;
int	b_len;
{
	char		*ap;
	char		*bp;
	register char	a;
	char		b;
	register int	al;
	register int	bl;

	ap = a_ptr;
	bp = b_ptr;
	al = a_len;
	if (al == 0)
		al = 32767;
	bl = b_len;
	if (bl == 0)
		bl = 32767;

	while (1)
	{

		/* supress blanks in both strings */
		while ((a = *ap) == ' ' && al > 0)
		{
			al--;
			ap++;
		}
		if (al == 0)
			a = 0;
		while (*bp == ' ' && bl > 0)
		{
			bl--;
			bp++;
		}
		if (bl == 0)
			b = 0;
		else
			b = *bp;

		/* do inequality tests */
		if (a < b)
			return (-1);
		if (a > b)
			return (1);
		if (a == 0)
			return (0);

		/* go on to the next character */
		ap++;
		al--;
		bp++;
		bl--;
	}
}
