#ifndef lint
static	char	*sccsid = "@(#)ln.c	1.4	(ULTRIX)	12/10/84";
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
 * ln.c
 *
 *	03-Nov-83	mah1: Correct message suppression.
 *
 *	17-Feb-84	mah. Change sccsid format to match
 *			ueg's sccsid format.
 *
 */

/*
 * ln
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

struct	stat stb;
int	fflag;		/* force flag set? */
int	sflag;
char	name[BUFSIZ];
char	*rindex();
extern	int errno;

main(argc, argv)
	int argc;
	register char **argv;
{
	register int i, r;

	argc--, argv++;
again:
 	if (argc && strcmp(argv[0], "-f") == 0) {
 		fflag++;
 		argv++;
 		argc--;
 	}
	if (argc && strcmp(argv[0], "-s") == 0) {
		sflag++;
		argv++;
		argc--;
	}
	if (argc == 0) 
		goto usage;
	else if (argc == 1) {
		argv[argc] = ".";
		argc++;
	}
	if (sflag == 0 && argc > 2) {
		if (stat(argv[argc-1], &stb) < 0)
			goto usage;
		if ((stb.st_mode&S_IFMT) != S_IFDIR) 
			goto usage;
	}
	r = 0;
	for(i = 0; i < argc-1; i++)
		r |= linkit(argv[i], argv[argc-1]);
	exit(r);
usage:
	fprintf(stderr, "Usage: ln [ -f ] [ -s ] f1\nor: ln [ -f ] [ -s ] f1 f2\nln [ -f ] [ -s ] f1 ... fn d2\n");
	exit(1);
}

int	link(), symlink();

linkit(from, to)
	char *from, *to;
{
	char *tail;
	int (*linkf)() = sflag ? symlink : link;

	/* is target a directory? */
	if (sflag == 0 && stat(from, &stb) >= 0
	    && (stb.st_mode&S_IFMT) == S_IFDIR) {
		if (fflag)			/* mah1: check here for -f */
			exit(0);		/*       than in if_st.ment */
		printf("%s is a directory\n", from);
		return (1);
	}
	if (stat(to, &stb) >= 0 && (stb.st_mode&S_IFMT) == S_IFDIR) {
		tail = rindex(from, '/');
		if (tail == 0)
			tail = from;
		else
			tail++;
		sprintf(name, "%s/%s", to, tail);
		to = name;
	}
	if ((*linkf)(from, to) < 0) {
		if (errno == EEXIST)
			perror(to);
		else if (access(from, 0) < 0)
			perror(from);
		else
			perror(to);
		return (1);
	}
	return (0);
}
