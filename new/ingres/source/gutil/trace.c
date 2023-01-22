#ifndef lint
static	char	*sccsid = "@(#)trace.c	1.1	(ULTRIX)	1/8/85";
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

# include	<useful.h>


# define	tTNCUTOFF	30

short		*tT;
int		tTsize;
char		*tTp, tTsav, tTany;
int		(*tTsetf)();
int		tTon(), tToff();

tTrace(argv, tflag, tvect, tsize)
char	**argv;
char	tflag;
short	*tvect;
int	tsize;
{
	register char	**pp;
	register bool	rtval;

	tT = tvect;
	tTsize = tsize;
	tTsetf = tTon;
	rtval = FALSE;

	for (pp = argv; *pp != NULL; pp++)
	{
		if ((*pp)[0] != '-' || (*pp)[1] != tflag)
			continue;
		tTflag(&(*pp)[2]);
		rtval = TRUE;
	}
	return (rtval);
}
/*
**  TTAMPER -- internal interface to set & clear trace flags
**
**	This routine is called from the ctlmod or whatever.
**
**	Parameters:
**		line -- a line like the normal trace flag lines.
**		tflag -- the trace flag to deal with.
**		tvect -- a pointer to a trace vector.
**		tsize -- the size of the trace vector.
**
**	Returns:
**		??
**
**	Side Effects:
**		none.
*/

tTamper(line, tflag, tvect, tsize)
char	*line;
char	tflag;
short	*tvect;
int	tsize;
{
	register char	*p;
	register char	*endp;
	register bool	rtval;
	char		save;
	short		*otvect;
	int		otsize;

	otvect = tT;
	otsize = tTsize;
	tT = tvect;
	tTsize = tsize;
	rtval = FALSE;

	for (p = line; *p != '\n' && *p != '\0'; p++)
	{
		switch (*p)
		{
		  case '+':
			tTsetf = tTon;
			break;

		  case '-':
			tTsetf = tToff;
			break;

		  default:
			continue;
		}
		if (*(++p) != tflag)
		{
			p--;
			continue;
		}

		for (endp = ++p; *endp != ' ' && *endp != '\t' && *endp != '\0' && *endp != '\n'; endp++)
			continue;

		save = *endp;
		*endp = '\0';

		tTflag(p);

		*endp = save;
		p = --endp;
		rtval = TRUE;
	}
	tT = otvect;
	tTsize = otsize;
	return (rtval);
}

tTflag(args)
char	*args;
{
	register int	fno;
	int		f;

	tTany++;
	tTp = args;
	if (*tTp == '\0')
	{
		for (fno = tTNCUTOFF; fno < tTsize; fno++)
			(*tTsetf)(fno, -1);
		return;
	}
	do
	{
		fno = tTnext();
		tTurn(fno);

		if (tTsav == '/')
		{
			f = fno + 1;
			fno = tTnext();
			while (f < fno)
				(*tTsetf)(f++, -1);
			tTurn(fno);
		}
	}  while(tTsav != '\0');
}

tTnext()
{
	register char	*c;
	auto int	ix;

	c = tTp;
	while (*tTp >= '0' && *tTp <= '9')
		tTp++;
	tTsav = *tTp;
	*tTp = '\0';
	ix = atoi(c);
	*tTp++ = tTsav;
	return (ix);
}

tTurn(fno)
register int	fno;
{
	register int	pt;

	if (tTsav == '.')
	{
		while (tTsav == '.')
		{
			pt = tTnext();
			(*tTsetf)(fno, pt);
		}
	}
	else
		(*tTsetf)(fno, -1);
}


tTon(fun, pt)
register int	fun;
register int	pt;
{
	if (fun >= tTsize || fun < 0)
		return;

	if (pt >= 0)
		tT[fun] |= (1<<pt%16);
	else
		tT[fun] = 0177777;
}


tToff(fun, pt)
register int	fun;
register int	pt;
{
	if (fun >= tTsize || fun < 0)
		return;

	if (pt >= 0)
		tT[fun] ^= (1<<pt%16);
	else
		tT[fun] = 0;
}


/*
**  CHECK TRACE FLAG AND PRINT INFORMATION
**
**	This routine is equivalent to
**		if (tTf(m, n))
**			printf(a1, a2, a3, a4, a5, a6);
**
**	and can be called to reduce process space.  The return value
**	is the value of the flag.
*/

# define	tTf(a, b)	((b < 0) ? tT[a] : (tT[a] & (1 << b)))

tTfp(m, n, a1, a2, a3, a4, a5, a6)
int	m;
int	n;
char	*a1, *a2, *a3, *a4, *a5, *a6;
{
	register int	rtval;

	if (rtval = tTf(m, n))
		printf(a1, a2, a3, a4, a5, a6);
	return (rtval);
}
