#ifndef lint
static	char	*sccsid = "@(#)chmod.c	1.2	(ULTRIX)	12/9/85";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
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
/************************************************************************
 *			Modification History
 *
 * 001	David L Ballenger, 5-Dec-1985
 *	If chmod fails due to a stat() or chmod() call, tell the
 *	user why.
 *
 *	Based on:  chmod.c	4.1 (Berkeley) 10/1/80
 *
 ************************************************************************/


/*
 * chmod [ugoa][+-=][rwxstugo] files
 *  change mode of files
 */
#include <stdio.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	USER	05700	/* user's bits */
#define	GROUP	02070	/* group's bits */
#define	OTHER	00007	/* other's bits */
#define	ALL	01777	/* all (note absence of setuid, etc) */

#define	READ	00444	/* read permit */
#define	WRITE	00222	/* write permit */
#define	EXEC	00111	/* exec permit */
#define	SETID	06000	/* set[ug]id */
#define	STICKY	01000	/* sticky bit */

char	*ms;
int	um;
struct	stat st;

extern int sys_nerr;
extern char *sys_errlist[];

/* print_error()
 *
 *	Routine to print error message, after a system call failed.
 */
static void
print_error(operation,name,error)
	char 		*operation,	/* operation */
			*name;		/* file name */
	unsigned	error;		/* error number */
{

	/* Print the operation and file name
	 */
	fprintf(stderr,"chmod: can't %s %s, ",operation,name);

	/* If there is no text for the error number just tell
	 * what the number is, otherwise print the text.
	 */
	if (errno > sys_nerr)
		fprintf(stderr,"(errno = %d)\n",error);
	else
		fprintf(stderr,"%s\n",sys_errlist[error]);
}

main(argc,argv)
char **argv;
{
	register i;
	register char *p;
	int status = 0;

	if (argc < 3) {
		fprintf(stderr, "Usage: chmod [ugoa][+-=][rwxstugo] file ...\n");
		exit(255);
	}
	ms = argv[1];
	um = umask(0);
	newmode(0);
	for (i = 2; i < argc; i++) {
		p = argv[i];
		if (stat(p, &st) < 0) {
			print_error("access",p,errno);
			++status;
			continue;
		}
		ms = argv[1];
		if (chmod(p, newmode(st.st_mode)) < 0) {
			print_error("change",p,errno);
			++status;
			continue;
		}
	}
	exit(status);
}

newmode(nm)
unsigned nm;
{
	register o, m, b;

	m = abs();
	if (!*ms)
		return(m);
	do {
		m = who();
		while (o = what()) {
			b = where(nm);
			switch (o) {
			case '+':
				nm |= b & m;
				break;
			case '-':
				nm &= ~(b & m);
				break;
			case '=':
				nm &= ~m;
				nm |= b & m;
				break;
			}
		}
	} while (*ms++ == ',');
	if (*--ms) {
		fprintf(stderr, "chmod: invalid mode\n");
		exit(255);
	}
	return(nm);
}

abs()
{
	register c, i;

	i = 0;
	while ((c = *ms++) >= '0' && c <= '7')
		i = (i << 3) + (c - '0');
	ms--;
	return(i);
}

who()
{
	register m;

	m = 0;
	for (;;) switch (*ms++) {
	case 'u':
		m |= USER;
		continue;
	case 'g':
		m |= GROUP;
		continue;
	case 'o':
		m |= OTHER;
		continue;
	case 'a':
		m |= ALL;
		continue;
	default:
		ms--;
		if (m == 0)
			m = ALL & ~um;
		return m;
	}
}

what()
{
	switch (*ms) {
	case '+':
	case '-':
	case '=':
		return *ms++;
	}
	return(0);
}

where(om)
register om;
{
	register m;

	m = 0;
	switch (*ms) {
	case 'u':
		m = (om & USER) >> 6;
		goto dup;
	case 'g':
		m = (om & GROUP) >> 3;
		goto dup;
	case 'o':
		m = (om & OTHER);
	dup:
		m &= (READ|WRITE|EXEC);
		m |= (m << 3) | (m << 6);
		++ms;
		return m;
	}
	for (;;) switch (*ms++) {
	case 'r':
		m |= READ;
		continue;
	case 'w':
		m |= WRITE;
		continue;
	case 'x':
		m |= EXEC;
		continue;
	case 's':
		m |= SETID;
		continue;
	case 't':
		m |= STICKY;
		continue;
	default:
		ms--;
		return m;
	}
}
