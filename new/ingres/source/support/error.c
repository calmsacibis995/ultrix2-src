#ifndef lint
static	char	*sccsid = "@(#)error.c	1.1	(ULTRIX)	1/8/85";
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

# include	<stdio.h>
# include	<ingres.h>
# include	<aux.h>


# define	ERRDELIM	'~'

/*
**  PROCESS ERROR MESSAGE (Standalone override)
**
**	This routine replaces the "error" routine for use in
**	standalone routines such as creatdb and printr.  Its usage
**	is identical to that of normal "error".
**
**	This routine is assumed to be called by process five; hence,
**	all error messages it produces have a 5000 offset.
**
*/

error(number, argvect)
int	number;
char	*argvect;
{
	FILE		*iop;
	char		**pv;
	int		i;
	register char	*p;
	register int	err;
	char		buf[10];
	register char	c;
	char		*errfilen();

	pv = &argvect;
	err = number;
	if ((iop = fopen(errfilen(5), "r")) == NULL)
		syserr("error: open");

	/* read in the code and check for correct */
	for (;;)
	{
		p = buf;
		while ((c = getc(iop)) != '\t')
		{
			if (c <= 0)
			{
				/* no code exists, print the first parm */
				printf("%d: %s\n\n", err, pv[0]);
				fclose(iop);
				return (err);
			}
			*p++ = c;
		}
		*p = 0;
		i = atoi(buf);
		if (i != err)
		{
			while ((c = getc(iop)) != ERRDELIM)
				if (c <= 0)
					syserr("proc_error: format err %d", err);
			getc(iop);	/* throw out the newline */
			continue;
		}

		/* got the correct line, print it doing parameter substitution */
		printf("%d: ", err);
		c = '\n';
		for (;;)
		{
			c = getc(iop);
			if (c == EOF || c == ERRDELIM)
			{
				printf("\n");
				fclose(iop);
				return (err);
			}
			if (c == '%')
			{
				c = getc(iop);
				for (p = pv[c - '0']; c = *p; p++)
				{
					putchar(c);
				}
				continue;
			}
			putchar(c);
		}
	}
}

nferror(number, arg1, arg2, arg3, arg4, arg5, arg6)
int	number;
{
	error(number, arg1, arg2, arg3, arg4, arg5, arg6);
}
