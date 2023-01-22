#ifndef lint
static	char	*sccsid = "@(#)subs.c	1.1	(ULTRIX)	1/8/85";
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
# include	<opsys.h>
# include	<sys/dir.h>


/*
** These are subroutines common to RESTORE and PURGE.
*/

#ifndef DIRBLKSIZ
typedef	DIR	FILE;

DIR *
opendir(d)
char *d;
{
	return(fopen(d, "r"));
}

closedir(d);
DIR *d;
{
	return(fclose(d));
}

struct direct *
readdir(dirp)
DIR *dirp;
{
	static struct direct direc;
	struct direct *d = &direc;
	int n;

	n = fread(&direc, sizeof(struct direct), 1, dirp);
	if (n <= 0)
		d = NULL;
	return(d);
}

#endif DIRBLKSIZ


char		All;
char		Qrymod;
char		Superuser;
char		Ask;
char		Purge;
char		Clean;
char		Lastflag;
DIR		*Direc;
extern int	Status;
extern char	*Usercode;
char		**Dblist;




/*
**  INITIALIZE GLOBALS
**
**	Set up Usercode and Status
*/

initialize(argc, argv)
int	argc;
char	**argv;
{
	register int	i;
	long		l;
	extern char	*Flagvect[];
	extern char	*Parmvect[];
	register char	*p;
	register char	**av;
	char		datadir[MAXLINE];

#	ifdef	xTTR2
	tTfp(40, 0, "entered initialize\n");
#	endif
	i = initucode(argc, argv, FALSE, NULL, -1);
#	ifdef	xTTR2
	tTfp(40, 1, "initucode ret:%d\n", i);
#	endif
	switch (i)
	{
	  case 0:
		break;

	  case 3:
		printf("You are not a valid INGRES user\n");
		exit(-1);

	  default:
		syserr("initucode %d", i);
	}
	initdbpath(NULL, datadir, FALSE);

	/* scan flags */
#	ifdef	xTTR2
	tTfp(40, 2, "scanning flags\n");
#	endif
	for (av = Flagvect;  *av != NULL; av++)
	{
		p = *av;
		if (p[0] != '-')
		{
		badflag:
			printf("Bad flag: %s\n", p);
			return (-1);
		}
		switch (p[1])
		{
		  case 'a':
			Ask++;
			break;

		  case 'p':
			Purge++;
			break;

		  case 's':
			if (sucheck())
				Superuser++;
			else
			{
				printf("You may not use the -s flag\n");
				exit(-1);
			}
			break;

		  case 'f':
			Clean++;
			break;

		  case 'T':
			break;

		  default:
			goto badflag;
		}
	}
	Dblist = Parmvect;
	if (*Dblist == 0)
	{
#		ifdef	xTTR2
		tTfp(40, 3, "doing all\n");
#		endif
		All++;
		Direc = opendir(datadir);
		if (Direc == NULL)
		{
			syserr("cannot read .../data/base");
		}
	}
#	ifdef	xTTR2
	tTfp(40, 0, "leaving initialize\n");
#	endif
}

/*
**  CHECK FOR SUPERUSER
**
**	The user has requested the -s flag.  Can he do it?  Will Martha
**	recover from cancer?  Will Dick get the girl?  Stay tuned for
**	"sucheck".
**
**	Permission is based on the U_SUPER bit in the status field
**	in the users file.
*/

sucheck()
{
	return (Status & U_SUPER);
}



/*
**  GET NEXT DATABASE
**
**	The next database to be purged is selected.  It comes from
**	either the directory or the database list.
**
**	Getnxtdb() leaves the user in the database directory.
*/

char *
getnxtdb()
{
	struct direct		*dp;
#ifndef DIRBLKSIZ
	static char		dbname[DIRSIZ+1];
#endif
	register char		*db;
	register FILE		*fd;
	register int		i;
	extern struct admin	Admin;
	static char		dbpbuf[MAXLINE];

#	ifdef	xTTR2
	tTfp(41, 0, "entered getnxtdb\n");
#	endif
	for (;;)
	{
		if (All)
		{

			dp = readdir(Direc);
			if (dp == NULL)
				db = NULL;
			else
			{
				if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
				{
					continue;
				}
#ifdef DIRBLKSIZ
				db = dp->d_name;
#else
				strncpy(dbname, dp->d_name, DIRSIZ);
				dbname[DIRSIZ] = '\0';
				db = dbname;
#endif
			}
		}
		else
		{
			db = *Dblist++;
		}
		if (db == NULL)
			return (NULL);
#		ifdef	xTTR2
		tTfp(41, 1, "using %s as Database\n", db);
#		endif
		i = initdbpath(db, dbpbuf, TRUE);
#		ifdef	xTTR2
		tTfp(41, 3, "initdbpath ret: %d, %s\n", i, dbpbuf);
#		endif
		switch (i)
		{
		  case 0:
		  case 1:
			break;

		  case 2:
		  case 3:
			printf("Database %s does not exist\n", db);
			continue;

		  default:
			syserr("initdbpath %d", i);
		}
		if (chdir(dbpbuf) < 0)
		{
			printf("Cannot enter %s", dbpbuf);
			continue;
		}
#		ifdef	xTTR2
		tTfp(41, 4, "chdir ok, Superuser: %d\n", Superuser);
#		endif
		fd = fopen("admin", "r");
		if (fd == NULL)
		{
			printf("Cannot open %s/admin\n", dbpbuf);
			continue;
		}
		fread(&Admin.adhdr, sizeof Admin.adhdr, 1, fd);
		fclose(fd);
#		ifdef	xTTR2
		tTfp(41, 5, "user: %.2s\n", Admin.adhdr.adowner);
#		endif

		/* set qrymod flag from database status */
		Qrymod = ((Admin.adhdr.adflags & A_QRYMOD) == A_QRYMOD);

		/* check for dba of database if not superuser */ 
		if (Superuser || bequal(Admin.adhdr.adowner, Usercode, 2))
			break;

		/*
		** not dba isn't an error if running in all mode since user
		** couln't have specified the database
		*/
		if (All)
			continue;
printf("You are not the dba for %s\n", db);
	}
#	ifdef	xTTR2
	tTfp(41, 6, "leaving getnxtdb, %s ok\n", db);
#	endif
	return (db);
}
/*
** ASK
**	If Ask is set desplay prompt and look for 'y' and return TRUE
**	If Ask is not set return TRUE
*/

ask(prompt)
char	*prompt;
{
	register char	*p;
	char		line[MAXLINE];
	extern char	Ask;

	if (!Ask)
		return (TRUE);
	p = prompt;

	while (*p)
	{
		putchar(*p);
		p++;
	}

	if (fgets(line, MAXLINE, stdin) == NULL)
		return(FALSE);

	return (line[0] == 'y');
}
