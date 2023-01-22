#ifndef lint
static char sccsid[] = "@(#)rwho.c	1.3	(ULTRIX)	2/6/87";
#endif

/************************************************************************
 *									*
 *			Copyright (c) 1987 by				*
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

/*-----------------------------------------------------------------------
 *
 * 02-mar-87 -- logcher
 *	Fixed to recognize eight character names.
 *
 * 24-apr-85 -- jrs
 *	Added features to allow subset selection and sort by host name
 *
 *	Based on 4.2BSD, labelled:
 *		rwho.c	4.7	83/07/01
 *
 *----------------------------------------------------------------------*/
/*
 *	rr - made this thing malloc and fixed it once and for all
 */

#include <sys/param.h>
#include <stdio.h>
#include <sys/dir.h>
#include "../etc/rwhod/rwhod.h"

#define	NUSERS	100		/* size to malloc each time */
struct	myutmp {
	char	myhost[32];
	int	myidle;
	struct	outmp myutmp;
};				/* malloc this guy */

long	now;
struct	whod *wd;
#define	WHDRSIZE	(sizeof (struct whod) - sizeof (wd->wd_we))
#define	RWHODIR		"/usr/spool/rwho"

int	utmpcmp(), hostcmp();
char	*ctime(), *strcpy(), *malloc(), *realloc(), *getwd();
long	time();
void	exit(), perror();

main(argc, argv)
int argc;
char **argv;
{
	register struct whod *w;
	register struct whoent *we;
	register struct myutmp *mp;
	register int f, n, i;
	struct myutmp *myutmp;
	char *matchto = NULL;
	char *argp;
	char *myname;
	char buf[22];
	struct direct *dp;
	DIR	*spdir;
	int	cc, width, j;
	int	hostsrt = 0;
	int	aflg = 0;
	int	sizemyutmp = NUSERS;
	int	nusers = 0;
#ifdef GPROF
	char 	pathname[MAXPATHLEN];
#endif GPROF

	myname = *argv;
	argc--, argv++;

	while (argc) {
		argp = *argv;
		if (*argp == '-') {
			argp++;
			while (*argp != '\0') {
				switch (*argp) {

				case 'a':	/* all */
					aflg++;
					break;

				case 'h':	/* host sort */
					hostsrt = 1;
					break;

				default:
					(void) fprintf(stderr,
					"%s: Invalid switch - %c\n",
					myname, *argp);
					exit(1);
				}
				argp++;
			}
		} 
		else {
			matchto = argp;
		}
		argc--;
		argv++;
	}
	(void) time(&now);
#ifdef GPROF
	(void) getwd(pathname);
#endif GPROF
	if (chdir(RWHODIR) < 0) {
		perror(RWHODIR);
		exit(1);
	}
	spdir = opendir(".");
	if (spdir == NULL) {
		perror(RWHODIR);
		exit(1);
	}
	w = (struct whod *) malloc((unsigned)sizeof(struct whod));
	if (w == NULL) {
		perror(myname);
		exit(1);
	}
	mp = myutmp = (struct myutmp *)
		malloc((unsigned)sizeof(struct myutmp) * sizemyutmp);
	if (myutmp == NULL) {
		perror(myname);
		exit(1);
	}
	while (dp = readdir(spdir)) {
		if (dp->d_ino == 0)
			continue;
		if (strncmp(dp->d_name, "whod.", 5))
			continue;
		f = open(dp->d_name, 0);
		if (f < 0)
			continue;
		cc = read(f, (char *)w, sizeof (struct whod));
		if (cc < WHDRSIZE) {
			(void) close(f);
			continue;
		}
		if (now - w->wd_recvtime > 5 * 60) {
			(void) close(f);
			continue;
		}
		cc -= WHDRSIZE;
		we = w->wd_we;
		for (n = cc / sizeof (struct whoent); n > 0; n--) {
			if (aflg == 0 && we->we_idle >= 60*60) {
				we++;
				continue;
			}
			/*
			 * Check if length of matchto is 8.
			 * If it is, the null has been overwritten
			 * and a strcmp will not match matchto.
			 * A strncmp is needed only when matchto is 8.
			 */
			if  (strlen(matchto) < 8) {
				if (matchto != NULL
			    		&& strcmp(matchto, w->wd_hostname) != 0
			    		&& strcmp(matchto, we->we_utmp.out_name) != 0
			    		&& strcmp(matchto, we->we_utmp.out_line) != 0) {
						we++;
						continue;
				}
			}
			else {
				if (strncmp(matchto, w->wd_hostname, strlen(matchto)) != 0
			    		&& strncmp(matchto, we->we_utmp.out_name, strlen(matchto)) != 0
			    		&& strncmp(matchto, we->we_utmp.out_line, strlen(matchto)) != 0) {
						we++;
						continue;
				}
			}
			if (nusers >= sizemyutmp) {
				sizemyutmp += NUSERS;
				myutmp = (struct myutmp *)
					(realloc((char *) myutmp,
					(unsigned) sizemyutmp *
					    sizeof(struct myutmp)));
				if (myutmp == NULL) {
					perror(myname);
					exit(1);
				}
				mp = &myutmp[nusers];
			}
			mp->myutmp = we->we_utmp; 
			mp->myidle = we->we_idle;
			(void) strcpy(mp->myhost, w->wd_hostname);
			nusers++; 
			we++; 
			mp++;
		}
		(void) close(f);
	}
	qsort((char *)myutmp, nusers, sizeof (struct myutmp),
	hostsrt? hostcmp: utmpcmp);
	mp = myutmp;
	width = 0;
	for (i = 0; i < nusers; i++) {
		j = strlen(mp->myhost) + 1 + strlen(mp->myutmp.out_line);
		if (j > width)
			width = j;
		mp++;
	}
	mp = myutmp;
	for (i = 0; i < nusers; i++) {
		(void) sprintf(buf, "%s:%s", mp->myhost, mp->myutmp.out_line);
		(void) printf("%-8.8s %-*s %.12s",
		mp->myutmp.out_name,
		width,
		buf,
		ctime((long *)&mp->myutmp.out_time)+4);
		mp->myidle /= 60;
		if (mp->myidle) {
			if (aflg) {
				if (mp->myidle >= 100*60)
					mp->myidle = 100*60 - 1;
				if (mp->myidle >= 60)
					(void) printf(" %2d", mp->myidle / 60);
				else
					(void) printf("   ");
			} 
			else
				(void) printf(" ");
			(void) printf(":%02d", mp->myidle % 60);
		}
		(void) printf("\n");
		mp++;
	}
#ifdef GPROF
	(void) chdir(pathname);
#endif GPROF
	exit(0);
}

utmpcmp(u1, u2)
register struct myutmp *u1, *u2;
{
	register int rc;

	rc = strncmp(u1->myutmp.out_name, u2->myutmp.out_name, 8);
	if (rc)
		return (rc);
	rc = strncmp(u1->myhost, u2->myhost, 8);
	if (rc)
		return (rc);
	return (strncmp(u1->myutmp.out_line, u2->myutmp.out_line, 8));
}

hostcmp(u1, u2)
register struct myutmp *u1, *u2;
{
	register int rc;

	rc = strncmp(u1->myhost, u2->myhost, 8);
	if (rc)
		return (rc);
	rc = strncmp(u1->myutmp.out_name, u2->myutmp.out_name, 8);
	if (rc)
		return (rc);
	return (strncmp(u1->myutmp.out_line, u2->myutmp.out_line, 8));
}
