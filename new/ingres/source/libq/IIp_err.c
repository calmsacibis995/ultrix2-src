#ifndef lint
static	char	*sccsid = "@(#)IIp_err.c	1.1	(ULTRIX)	1/8/85";
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

# include	<pipes.h>
# include	<ingres.h>
# include	<aux.h>
# include	<version.h>
# include	"IIglobals.h"



/*
**  IIp_err 
**
**	This routine processes an IIerror call for printout.This involves doing
**	a lookup in the .../files/error? files, where ? is the thous-
**	ands digit of the error number.  The associated error message
**	then goes through parameter substitution and is printed.
**
**	In the current version, the error message is just printed.
*/

IIp_err(err_num, argc, argv)
int		err_num;
int		argc;
char		**argv;
{
	register char	*p;
	char		buf[512];
	int		i;
	struct iob	iob;
	register char	c;
	extern char	*IIerrfilen();
	extern char	*IIconcatv();

	/* IIfopen the appropriate error file */
	p = buf;
	IIerrfilen(err_num / 1000, p);

#	ifdef xETR2
	if (IIdebug > 1)
		printf("IIp_err : with IIerrfilen ret \"%s\"\n", p);
#	endif

	if (IIfopen(p, &iob) < 0)
		return (0);

	/* read in the code and check for correct */
	for (;;)
	{
		p = buf;
		while ((c = IIgetc(&iob)) != '\t')
		{
			if (c <= 0)
			{
				if (IIclose(&iob) < 0)
					IIsyserr("IIp_err: bad close 1");
				return (0);
			}
			*p++ = c;
		}
		*p = 0;
		if (IIatoi(buf, &i))
			IIsyserr("IIp_err: bad error file %d\n%s",
				err_num, buf);
		if (i != err_num)
		{
			while ((c = IIgetc(&iob)) != ERRDELIM)
				if (c <= 0)
					IIsyserr("IIp_err: format err %d", err_num);
			IIgetc(&iob);	/* throw out the newline */
			continue;
		}

		/* got the correct line, print it doing parameter substitution */
		printf("%d: ", err_num);
		for (;;)
		{
			c = IIgetc(&iob);
			if (c <= 0 || c == ERRDELIM)
			{
				printf("\n");
				 if (IIclose(&iob) < 0)
					IIsyserr("IIp_err: bad close 2");
				return (1);
			}
			if (c == '%')
			{
				c = IIgetc(&iob);
				for (p = argv[c - '0']; c = *p; p++)
				{
					if (c < 040 || c >= 0177)
						printf("\\%o", c);
					else
						putchar(c);
				}
				continue;
			}
			putchar(c);
		}
	}
}

/*
** IIerrfilen -- Returns the pathname where the error file can be found
**	into "buf".
**
**	It is assumed that the error number cannot be more than 999.
**	Returned is concat of : IIPathname, "/files/error", VERSION (w/o mod),
**	"_num".
*/

char *
IIerrfilen(num, buf)
int	num;
char	*buf;
{
	register char	*cp;
	char		ver[12];
	extern char	*IIconcatv();

	IIconcatv(ver, VERSION, 0);

	/* now insert the "_X" */
	cp = &ver[IIlength(ver)];
	*cp++ = '_';
	*cp = '\0';
	IIconcatv(ver, ver, IIitos(num), 0);


	return (IIconcatv(buf, IIPathname, "/files/error", ver, 0));
}
