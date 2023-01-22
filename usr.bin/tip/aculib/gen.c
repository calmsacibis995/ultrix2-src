/*      @(#)gen.c	1.2     Ultrix  10/21/85     */
/************************************************************************
 *                                                                      *
 *                      Copyright (c) 1985 by                           *
 *              Digital Equipment Corporation, Maynard, MA              *
 *                      All rights reserved.                            *
 *                                                                      *
 *   This software is furnished under a license and may be used and     *
 *   copied  only  in accordance with the terms of such license and     *
 *   with the  inclusion  of  the  above  copyright  notice.   This     *
 *   software  or  any  other copies thereof may not be provided or     *
 *   otherwise made available to any other person.  No title to and     *
 *   ownership of the software is hereby transferred.                   *
 *                                                                      *
 *   This software is  derived  from  software  received  from  the     *
 *   University    of   California,   Berkeley,   and   from   Bell     *
 *   Laboratories.  Use, duplication, or disclosure is  subject  to     *
 *   restrictions  under  license  agreements  with  University  of     *
 *   California and with AT&T.                                          *
 *                                                                      *
 *   The information in this software is subject to change  without     *
 *   notice  and should not be construed as a commitment by Digital     *
 *   Equipment Corporation.                                             *
 *                                                                      *
 *   Digital assumes no responsibility for the use  or  reliability     *
 *   of its software on equipment which is not supplied by Digital.     *
 *                                                                      *
 ************************************************************************/

/*
 *      Generic dialer routines (mainly the termcap part here). The
 *      Actual dial, abort & disconnect routines are in aculib/generic.c
 *
 *                                              Larry Palmer
 *                                              lp@decvax
 */

#include <sys/time.h>

typeahead(masker, secns, usecns)
int masker;
long secns, usecns;
{
	int nbits=16, *writemask=0, *exceptmask=0, found;
	struct timeval timeqr, *timerp;
	int *rdmsk, mask;

	timeqr.tv_sec = secns;
	timeqr.tv_usec = usecns;

	mask = masker;
	rdmsk = &mask;
	timerp = &timeqr;
/*
 *      Select returns some positive value if things in typeahead
 *      otherwise 0.
 */
	found = select(nbits, rdmsk, writemask, exceptmask, timerp);
	return (found > 0 ? 1 : 0);
}


#define DEFDELAY 1
#define GENBUF 512
#define MAXSTR 128

extern  char *syncstr, *respstr, *dialstr, *respdial, *abostr, *onlstr,
		*disconstr, charup, *compstr, hangup, dtr, stupidi,
		*respsync, *dialterm, *replacestr;
extern int syncdel, dialdel, cdelay, compdel;

char *acustrngs[] = { "ss", "sr", "di", "dr", "dt", "ab",
			"ds", "cs", "os", "rs", 0};

extern int FDD;

gen_setup(gbuf, FD)
char *gbuf;
int FD;
{
	char **p, *temp;
	char *rgetstr();
	extern char *tbuf;
	extern char lsleep, debugn;
	extern int dialack;

	/* get strings out */

	FDD = FD;
	tbuf = gbuf;

	for (p = acustrngs; *p != 0; p++) {
		temp = (char *) malloc(MAXSTR);
		if (!strcmp("ss", *p))
			syncstr = rgetstr(*p, &temp);
		else if (!strcmp("sr", *p))
			respsync = rgetstr(*p, &temp);
		else if (!strcmp("di", *p))
			dialstr = rgetstr(*p, &temp);
		else if (!strcmp("dr", *p))
			respdial = rgetstr(*p, &temp);
		else if (!strcmp("dt", *p))
			dialterm = rgetstr(*p, &temp);
		else if (!strcmp("ab", *p))
			abostr = rgetstr(*p, &temp);
		else if (!strcmp("ds", *p))
			disconstr = rgetstr(*p, &temp);
		else if (!strcmp("cs", *p))
			compstr = rgetstr(*p, &temp);
		else if (!strcmp("os", *p))
			onlstr = rgetstr(*p, &temp);
		else if (!strcmp("rs", *p))
			replacestr = rgetstr(*p, &temp);
		else
		       cfree(temp);
	}

	/*
	 * Get booleans
	 */

	charup = rgetflag("cr");
	hangup = rgetflag("hu");
	dtr = rgetflag("re");
	lsleep = rgetflag("ls");
	debugn = rgetflag("db");
	stupidi = rgetflag("si");

	/*
	 * Get delay numbers
	 */

	syncdel = rgetnum("sd");
	dialdel = rgetnum("dd");
	compdel = rgetnum("cd");
	cdelay = rgetnum("fd");
	dialack = rgetnum("da");

}

#define ACULIST         "/etc/acucap"

#ifndef BUFSIZ
#define BUFSIZ          1024
#endif

agetent(bp, name)
	char *bp, *name;
{
	extern char *tbuf;
	char lbuf[BUFSIZ], *cp, *p;
	int rc1, rc2;

	cp = ACULIST;
	if (cp == (char *)0 || strcmp(cp, ACULIST) == 0) {
		cp = ACULIST;
		return (getent(bp, name, cp));
	} else {
		if ((rc1 = getent(bp, name, cp)) != 1)
			*bp = '\0';
		cp = ACULIST;
		rc2 = getent(lbuf, name, cp);
		if (rc1 != 1 && rc2 != 1)
			return (rc2);
		if (rc2 == 1) {
			p = lbuf;
			if (rc1 == 1)
				while (*p++ != ':')
					;
			if (strlen(bp) + strlen(p) > BUFSIZ) {
				write(2, "Acucap entry too long\n", 23);
				return (-1);
			}
			strcat(bp, p);
		}
		tbuf = bp;
		return (1);
	}
}
