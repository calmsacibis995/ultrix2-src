/*	mass.c -
 *		mass shell variable assignment utility.
 *
 *	mass [-c] [-i'c'] "VAR1,VAR2,...,VARn"
 *		reads lines from stdin, writing assignment statements
 *		to stdout permitting assignment of values from stdin to
 *		variable names listed on cmd line. Null variable names
 *		on cmd line (",  ,,  ,") cause input lines to be skipped.
 *		Variable names that would be assigned to lines beyond EOF
 *		are output to be set to the null string.
 *		default output does assignments for sh(1), use of option
 *		-c allows assignment format accepable to csh(1).
 *
 *			Copyright (c) 1985 by
 *		Digital Equipment Corporation, Maynard, MA
 *			All rights reserved.
 *								
 *	This software is furnished under a license and may be used and
 *	copied  only  in accordance with the terms of such license and
 *	with the  inclusion  of  the  above  copyright  notice.   This
 *	software  or  any  other copies thereof may not be provided or
 *	otherwise made available to any other person.  No title to and
 *	ownership of the software is hereby transferred.		
 *								
 *	The information in this software is subject to change  without
 *	notice  and should not be construed as a commitment by Digital
 *	Equipment Corporation.					
 *								
 *	Digital assumes no responsibility for the use  or  reliability
 *	of its software on equipment which is not supplied by Digital.
 *
 *	modifications:
 *
 *	000	ccb@xylem.dec	19860214
 *		for MEO
*/

#ifndef lint
static	char *sccsid = "@(#)mass.c	1.1		5/1/86";
#endif lint
#include	<stdio.h>
#define		MAXVARS		256
#define		Usage()		fprintf(stderr,"Usage: %s [-c] varlist\n",\
						prog)

char *prog;	/* program name pointer */

main(argc,argv)
int argc;
char *argv[];
{
	register int	np;		/* number of parameters */
	char		*pvec[MAXVARS];	/* parameter vector */
	register char	**pvp = pvec;	/* parameter vector pointer */
	char		buf[BUFSIZ];	/* input buffer */
	char		*outfmt;	/* output format specifier */
	int		commchar = -1;	/* comment character */

	--argc;
	prog = *argv++;
	outfmt="%s=\"%s\"\n";
	while( **argv == '-' )
	{
		if( (*argv)[1] == 'c' ) /* csh */
			outfmt="set %s = \"%s\"\n";

		else if( (*argv)[1] == 'i' ) /* comment char */
			commchar = (int) (*argv)[2];

		else
		{
			Usage();
			exit(1);
		}
		--argc;++argv;
	}

	if( argc != 1 )
	{
		Usage();
		exit(1);
	}
	np = pvget(pvec,*argv);
	while(np--)
	{
		gets(buf);
		if(**pvp != '\0' && (int) *buf != commchar)
		{
			if(feof(stdin))
				*buf = '\0';
			printf(outfmt, *pvp, buf);
		}
		++pvp;
	}
	exit(0);
}


/*	pvget() -
 *		parse out varname string.
 *
 *	sets char pointers in vec to first character in each varname,
 *	replacing commas with nulls, counting the nuber of varnames.
 *	returns the number of varnames.
*/

pvget(vec,str)
char **vec;
register char *str;
{
	int	np = 0;

	*vec = str;
	for( ; np < MAXVARS; )
	{
		switch(*str)
		{
		case ',':
			*str = '\0';
			vec[++np] = ++str;
			break;
		case '\0':
			return(++np);
		default:
			++str;
		}
	}
}
