#ifndef lint
static	char	*sccsid = "@(#)ruptime.c	1.7	(ULTRIX)	2/12/87";
#endif lint

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

/*
 * ruptime.c
 *
 *	static char sccsid[] = "@(#)ruptime.c	4.14 (Berkeley) 83/07/01";
 *
 Modification History
 ~~~~~~~~~~~~~~~~~~~~
04	05-Aug-86  Aya Konishi
	A bug fix for SMU-00280.
	The bug was that ruptime did not count the users who are
	more than 1 hour idle.

03	13-May-85, Greg Tarsa
	Added a fix off the net to increase the uptime accepted before
	??:?? is printed as the uptime value from 3 to 12 months.

02	23-Apr-85, jrs
 	Add new switches to give subset listings.

01	12-Jul-84,	ma
	Correct error message and change *etc to *spdir for clarity.
*/

/*
 *	rr Jan 1987 - made this thing malloc and thus fixed it once and for all
 */

#include <sys/param.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/dir.h>
#include "../etc/rwhod/rwhod.h"


#define	NHOSTS	100		/* size to malloc each time */
struct	hs {
	struct	whod hs_wd;
	int	hs_nusers;
} 
*hs;

long	now;

#define	WHDRSIZE	(sizeof (struct whod) - sizeof (hs->hs_wd.wd_we))
#define	RWHODIR		"/usr/spool/rwho"
#define DOWN(h)		(now - (h)->hs_wd.wd_recvtime > 5 * 60)

int	hscmp(), ucmp(), lcmp(), tcmp();
char	*interval(), *malloc(), *realloc(), *sprintf(), *getwd();
long	time();
void	exit(), perror();


main(argc, argv)
int argc;
char **argv;
{
	register struct direct *dp;
	register int i;
	register int cc;
	register struct hs *hsp;
	register struct whod *wd;
	register struct whoent *we;
	int f, t;
	int aflg = 0;  /* Fix 04 */
	int maxloadav = 0;
	int downflag = 1;
	int upflag = 1;
	int minuse = -1;
	int sizehs = NHOSTS;
	int nhosts = 0;
	char *matchto = NULL;
	char *argp;
	char *myname;
	int (*cmp)() = hscmp;
	DIR	*spdir;
#ifdef GPROF
	char pathname[MAXPATHLEN];
#endif GPROF

	(void) time((long *)&t);
	myname = *argv;
	argc--, argv++;

	while (argc) {
		argp = *argv;
		if (*argp == '-') {
			argp++;
			while (*argp != '\0') {
				if (isdigit(*argp)) {
					minuse = atoi(argp);
					while (isdigit(*argp)) {
						argp++;
					}
				} 
				else {
					switch (*argp) {

					case 'a':	/* all */
						aflg++;
						break;

					case 'd':	/* down */
						upflag = 0;
						break;

					case 'l':	/* load sort */
						cmp = lcmp;
						break;

					case 'r':	/* running */
						downflag = 0;
						break;

					case 't':	/* uptime sort */
						cmp = tcmp;
						break;

					case 'u':	/* user count sort */
						cmp = ucmp;
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
		} 
		else {
			matchto = argp;
		}
		argc--;
		argv++;
	}
#ifdef GPROF
	(void) getwd(pathname);
#endif GPROF
	if (chdir(RWHODIR) < 0) {
		(void) perror(RWHODIR);
		exit(1);
	}
	spdir = opendir(".");
	if (spdir == NULL) {
		(void) perror(RWHODIR);
		exit(1);
	}
	hs = hsp = (struct hs *) (malloc((unsigned)sizehs * sizeof(struct hs)));
	if (hs == NULL) {
		perror(myname);
		exit(1);
	}
	while (dp = readdir(spdir)) {
		if (dp->d_ino == 0)
			continue;
		if (strncmp(dp->d_name, "whod.", 5))
			continue;
		if (matchto != NULL && strcmp(matchto, &dp->d_name[5]) != 0) {
			continue;
		}
		if (nhosts == sizehs) {
			sizehs += NHOSTS;
			hs = (struct hs *)(realloc((char *)hs, (unsigned)
			    sizehs * sizeof(struct hs)));
			if (hs == NULL) {
				perror(myname);
				exit(1);
			}
			hsp = &hs[nhosts];
		}
		f = open(dp->d_name, 0);
		if (f > 0) {
			cc = read(f, (char *)&hsp->hs_wd, sizeof(struct whod));
			if (cc >= WHDRSIZE) {
				wd = &hsp->hs_wd;
				hsp->hs_nusers = 0;
				for (i = 0; i < 2; i++)
					if (wd->wd_loadav[i] > maxloadav)
						maxloadav = wd->wd_loadav[i];
				we = (struct whoent *)(((char *)wd)+cc);
				while (--we >= wd->wd_we)  /* Fix 04 */
					if (!(aflg && we->we_idle > 3600))
						hsp->hs_nusers++;
				nhosts++; 
				hsp++;
			}
		}
		(void) close(f);
	}
	if (nhosts == 0) {
		if (matchto != NULL) {
			exit(0);
		} 
		else {
			(void) printf("no hosts!?!\n");
			exit(1);
		}
	}
	(void) time(&now);
	qsort((char *)hs, nhosts, sizeof (hs[0]), cmp);
	hsp = &hs[0];
	for (i = 0; i < nhosts; i++, hsp++) {
		if (DOWN(hsp)) {
			if (downflag != 0 && minuse < 0) {
				(void) printf("%-12.12s%s\n",
				hsp->hs_wd.wd_hostname,
				interval((int)(now - hsp->hs_wd.wd_recvtime),
				"down"));
			}
			continue;
		}
		if (upflag == 0 || (minuse >= 0 && hsp->hs_nusers < minuse)) {
			continue;
		}
		(void) printf(
		"%-12.12s%s,  %4d user%s  load %*.2f, %*.2f, %*.2f\n",
		hsp->hs_wd.wd_hostname,
		interval(hsp->hs_wd.wd_sendtime -
		    hsp->hs_wd.wd_boottime, "  up"),
		hsp->hs_nusers,
		hsp->hs_nusers == 1 ? ", " : "s,",
		maxloadav >= 1000 ? 5 : 4,
		hsp->hs_wd.wd_loadav[0] / 100.0,
		maxloadav >= 1000 ? 5 : 4,
		hsp->hs_wd.wd_loadav[1] / 100.0,
		maxloadav >= 1000 ? 5 : 4,
		hsp->hs_wd.wd_loadav[2] / 100.0);
	}
#ifdef GPROF
	(void) chdir(pathname);
#endif GPROF
	exit(0);
}

char *
interval(time, updown)
register int time;
register char *updown;
{
	static char resbuf[32];
	register int days, hours, minutes;

	if (time < 0 || time > 12*30*24*60*60) { /* GT003 */
		(void) sprintf(resbuf, "   %s ??:??", updown);
		return (resbuf);
	}
	minutes = (time + 59) / 60;		/* round to minutes */
	hours = minutes / 60; 
	minutes %= 60;
	days = hours / 24; 
	hours %= 24;
	if (days)
		(void) sprintf(resbuf, "%s %2d+%02d:%02d",
		updown, days, hours, minutes);
	else
		(void) sprintf(resbuf, "%s    %2d:%02d",
		updown, hours, minutes);
	return (resbuf);
}

/* for a faster compare on a vax*/
#define STRCMP(a,b) ((a)[0] == (b)[0]?strcmp((a),(b)) : (a)[0] > (b)[0]?1 : -1)

hscmp(h1, h2)
struct hs *h1, *h2;
{
	register char *a = h1->hs_wd.wd_hostname;
	register char *b = h2->hs_wd.wd_hostname;

	return (STRCMP(a,b));
}

/*
 * Compare according to load average.
 */
lcmp(h1, h2)
register struct hs *h1, *h2;
{

	if (DOWN(h1))
		if (DOWN(h2))
			return (tcmp(h1, h2));
		else
			return (1);
	else if (DOWN(h2))
		return (-1);
	else
		return (h2->hs_wd.wd_loadav[0] - h1->hs_wd.wd_loadav[0]);
}

/*
 * Compare according to number of users.
 */
ucmp(h1, h2)
register struct hs *h1, *h2;
{

	if (DOWN(h1))
		if (DOWN(h2))
			return (tcmp(h1, h2));
		else
			return (1);
	else if (DOWN(h2))
		return (-1);
	else
		return (h2->hs_nusers - h1->hs_nusers);
}

/*
 * Compare according to uptime.
 */
tcmp(h1, h2)
register struct hs *h1, *h2;
{
	return (
	(DOWN(h2) ? h2->hs_wd.wd_recvtime - now
	    : h2->hs_wd.wd_sendtime - h2->hs_wd.wd_boottime)
	    -
	    (DOWN(h1) ? h1->hs_wd.wd_recvtime - now
	    : h1->hs_wd.wd_sendtime - h1->hs_wd.wd_boottime)
	    );
}
