#ifndef lint
static	char	*sccsid = "@(#)chgrp.c	1.4	(ULTRIX)	12/10/84";
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
 * chgrp.c
 *
 *	24-Oct-83	mah1: Suppress error message if -f option is
 *			      indicated.  Exception is the usage message.
 *
 *	17-Feb-84	mah. Change sccsid format to match
 *			ueg's sccsid format.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>

struct	group *gr, *getgrnam(), *getgrgid();
struct	passwd *getpwuid(), *pwd;
struct	stat stbuf;
int	gid, uid;
int	status;
int	fflag;
/* VARARGS */
int	fprintf();

main(argc, argv)
	int argc;
	char *argv[];
{
	register c, i;

	argc--, argv++;
	if (argc > 0 && strcmp(argv[0], "-f") == 0) {
		fflag++;
		argv++, argc--;
	}
	if (argc < 2) {
		printf("usage: chgrp [-f] gid file ...\n");
		exit(2);
	}
	uid = getuid();
	if (isnumber(argv[0])) {
		gid = atoi(argv[0]);
		gr = getgrgid(gid);
		if (uid && gr == NULL) {
			if (fflag)		/* mah1 - suppress msg */
				exit(0);
			printf("%s: unknown group\n", argv[0]);
			exit(2);
		}
	} else {
		gr = getgrnam(argv[0]);
		if (gr == NULL) {
			if (fflag)		/* mah1 - suppress msg */
				exit(0);
			printf("%s: unknown group\n", argv[0]);
			exit(2);
		}
		gid = gr->gr_gid;
	}
	pwd = getpwuid(uid);
	if (pwd == NULL) {
		if (fflag)			/* mah1 - suppress msg */
			exit(0);
		fprintf(stderr, "Who are you?\n");
		exit(2);
	}
	if (uid && pwd->pw_gid != gid) {
		for (i=0; gr->gr_mem[i]; i++)
			if (!(strcmp(pwd->pw_name, gr->gr_mem[i])))
				goto ok;
		if (fflag)
			exit(0);
		fprintf(stderr, "You are not a member of the %s group.\n",
		    argv[0]);
		exit(2);
	}
ok:
	for (c = 1; c < argc; c++) {
		if (stat(argv[c], &stbuf) && !fflag) {	/* mah1 - suppress msg*/
			perror(argv[c]);
			continue;
		}
		if (uid && uid != stbuf.st_uid) {
			if (fflag)
				continue;
			fprintf(stderr, "You are not the owner of %s\n",
			    argv[c]);
			status = 1;
			continue;
		}
		if (chown(argv[c], stbuf.st_uid, gid) && !fflag)
			perror(argv[c]);
	}
	exit(status);
}

isnumber(s)
	char *s;
{
	register int c;

	while (c = *s++)
		if (!isdigit(c))
			return (0);
	return (1);
}
