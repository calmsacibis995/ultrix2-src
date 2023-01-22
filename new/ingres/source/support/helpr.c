#ifndef lint
static	char	*sccsid = "@(#)helpr.c	1.1	(ULTRIX)	1/8/85";
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
# include	<access.h>
# include	<lock.h>
# include	<pv.h>



extern int	Status;
short		tTdbu[100];

main(argc, argv)
int	argc;
char 	*argv[];
{
	extern struct out_arg	Out_arg;
	register char		**av;
	register int		i;
	register char		*p;
	extern char		*Parmvect[];
	extern char		*Flagvect[];
	extern char		*Dbpath;
	int			nc;
	PARM			newpv[PV_MAXPC];
	PARM			*nv;
	char			*qm;
	char			*qmtest();

	argv[argc] = NULL;

#	ifdef xSTR1
	tTrace(argv, 'T', tTdbu, 100);
#	endif

	i = initucode(argc, argv, TRUE, NULL, M_SHARE);
#	ifdef xSTR2
	if (tTf(0, 1))
		printf("initucode=%d, Dbpath='%s'\n", i, Dbpath);
#	endif
	switch (i)
	{
	  case 0:
	  case 5:
		break;

	  case 1:
	  case 6:
		printf("Database %s does not exist\n", Parmvect[0]);
		exit(-1);

	  case 2:
		printf("You are not authorized to access this database\n");
		exit(-1);

	  case 3:
		printf("You are not a valid INGRES user\n");
		exit(-1);

	  case 4:
		printf("No database name specified\n");
	usage:
		printf("usage: helpr database [relname ...]\n");
		exit(-1);

	  default:
		syserr("initucode %d", i);
	}

/*
	if (Flagvect[0] != NULL)
	{
		printf("No flags are allowed for this command\n");
		goto usage;
	}
*/

	if (chdir(Dbpath) < 0)
		syserr("cannot access data base %s", p);
#	ifdef xTTR2
	if (tTf(1, 0))
		printf("entered database %s\n", Dbpath);
#	endif

	/* initialize access methods (and Admin struct) for user_ovrd test */
	acc_init();
#	ifdef xTTR3
	if (tTf(2, 0))
		printf("access methods initialized\n");
#	endif

	set_so_buf();

	av = &Parmvect[1];	/* get first param after database name */
	p = *av;
	if (p == NULL)
	{
		/* special case of no relations specified */
		newpv[0].pv_type = PV_STR;
		newpv[0].pv_val.pv_str = "2";
		newpv[1].pv_type = PV_EOF;
#		ifdef xTTR3
		if (tTf(3, 0))
			printf("calling help, no relations specified\n");
#		endif
		help(1, newpv);
	}
	else
	{
		do
		{
			nc = 0;
			nv = newpv;

			if ((qm = qmtest(p)) != NULL)
			{
				/* either help view, integrity or protect */
				av++;
				while ((p = *av++) != NULL)
				{
					if ((i = (int) qmtest(p)) != NULL)
					{
						/* change of qmtest result */
						qm = (char *) i;
						continue;
					}
					(nv)->pv_type = PV_STR;
					(nv++)->pv_val.pv_str = qm;
					(nv)->pv_type = PV_STR;
					(nv++)->pv_val.pv_str = p;
					nc += 2;
				}
#				ifdef xTTR3
				if (tTf(3, 0))
					printf("calling display\n");
#				endif
				nv->pv_type = PV_EOF;
				/*
				display(nc, newpv);
				*/
			}
			else
			{
				/* help relname */
				while ((p = *av++) != NULL && qmtest(p) == NULL)
				{
					if (sequal("all", p))
					{
						(nv)->pv_type = PV_STR;
						(nv++)->pv_val.pv_str = "3";
						nc++;
					}
					else
					{
						(nv)->pv_type = PV_STR;
						(nv++)->pv_val.pv_str = "0";
						(nv)->pv_type = PV_STR;
						(nv++)->pv_val.pv_str = p;
						nc += 2;
					}
				}
				nv->pv_type = PV_EOF;
#				ifdef xTTR3
				if (tTf(3, 0))
					printf("calling help\n");
#				endif
				help(nc, newpv);
				/* this backs av up one step, so 
				 * that it points at the keywords (permit,
				 * integrity, view) or the NULL
				 */
				--av;
			}
		} while (p != NULL);
	}
	fflush(stdout);
	exit(0);
}



char *qmtest(p)
register char	*p;
{
	if (sequal("view", p))
		return ("4");
	else if (sequal("permit", p))
		return ("5");
	else if (sequal("integrity", p))
		return ("6");
	else
		return (NULL);
}


rubproc()
{
	exit(1);
}
