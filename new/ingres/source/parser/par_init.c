#ifndef lint
static	char	*sccsid = "@(#)par_init.c	1.1	(ULTRIX)	1/8/85";
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

# include	<ingres.h>
# include	<symbol.h>
# include	"parser.h"
# include	<access.h>


/*
**  PAR_INIT -- initialization call for parser
**
**	Trace Flags:
**		par_init ~~ 60.0
*/

par_init(argc, argv1)
int	argc;
char	*argv1[];
{
	register int	rt;
	register char	**argv;

	extern int		Noupdt;
	extern int		Dcase;
	extern char		*Relspec;
	extern char		*Indexspec;
	extern DESC		Attdes;
	extern struct admin	Admin;
	extern int		Qrymod;
	extern int		yydebug;

	/* set up parser */
	argv = argv1;



#	ifdef	xPTR1
	if (tTf(60, 0))
		yydebug = 1;
#	endif

#	ifdef	xPTR2
	if (tTf(60, 1))
	{
		printf("Par_init:	");
		prargs(argc, argv);
	}
#	endif

	Noupdt = !setflag(argv, 'U', 0);
	Dcase = setflag(argv, 'L', 1);

	/* if param specified, set result reln storage structures */
	Relspec = "cheapsort";		/* default to cheapsort on ret into */
	Indexspec = "isam";		/* isam on index */

	for (rt = FREEFLAGS; rt < argc; rt++)
	{
		if (argv[rt][0] == '-')
		{
			if (argv[rt][1] == 'r')
			{
				Relspec = &argv[rt][2];
			}
			if (argv[rt][1] == 'n')
			{
				Indexspec = &argv[rt][2];
				continue;
			}
		}
	}
	if (sequal(Relspec, "heap"))
		Relspec = 0;
	if (sequal(Indexspec, "heap"))
		Indexspec = 0;

	rnginit();
	opencatalog("attribute", 0);

	Qrymod = ((Admin.adhdr.adflags & A_QRYMOD) == A_QRYMOD);

#	ifdef	xPTR2
	if (tTf(60, 2))
	{
		printf("Par_init: Results:\n");
		printf("\tQrymod: %d\n", Qrymod);
		printf("\tIndexspec: %s\n", Indexspec);
		printf("\tRelspec: %s\n", Relspec);
		printf("\tDcase: %d\n", Dcase);
		printf("\tNoupdt: %d\n", Noupdt); 
	}
#	endif
}
