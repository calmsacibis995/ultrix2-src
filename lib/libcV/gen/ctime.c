#ifndef lint
static	char	*sccsid = "@(#)ctime.c	1.2	(ULTRIX)	8/16/85";
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
 *	David L Ballenger, 16-Aug-1985
 * 001	Use real gettimeofday() call.
 *
 *	Based on:  ctime.c	1.2
 *		   3.0 SID #	1.2
 *
 ************************************************************************/

/*LINTLIBRARY*/
/*
 * This routine converts time as follows.
 * The epoch is 0000 Jan 1 1970 GMT.
 * The argument time is in seconds since then.
 * The localtime(t) entry returns a pointer to an array
 * containing
 *  seconds (0-59)
 *  minutes (0-59)
 *  hours (0-23)
 *  day of month (1-31)
 *  month (0-11)
 *  year-1970
 *  weekday (0-6, Sun is 0)
 *  day of the year
 *  daylight savings flag
 *
 * The routine corrects for daylight saving
 * time and will work in any time zone provided
 * "timezone" is adjusted to the difference between
 * Greenwich and local standard time (measured in seconds).
 * In places like Michigan "daylight" must
 * be initialized to 0 to prevent the conversion
 * to daylight time.
 * There is a table which accounts for the peculiarities
 * undergone by daylight time in 1974-1975.
 *
 * The routine does not work
 * in Saudi Arabia which runs on Solar time.
 *
 * asctime(tvec)
 * where tvec is produced by localtime
 * returns a ptr to a character string
 * that has the ascii time in the form
 *	Thu Jan 01 00:00:00 1970n0\\
 *	01234567890123456789012345
 *	0	  1	    2
 *
 * ctime(t) just calls localtime, then asctime.
 *
 * tzset() looks for an environment variable named
 * TZ. It should be in the form "ESTn" or "ESTnEDT",
 * where "n" represents a string of digits with an optional
 * negative sign (for locations east of Greenwich, England).
 * If the variable is present, it will set the external
 * variables "timezone", "daylight", and "tzname"
 * appropriately. It is called by localtime, and
 * may also be called explicitly by the user.
 */

#define	dysize(A) (((A)%4)? 365: 366)
#include <time.h>

long	timezone = 5*60*60;
int	daylight = 1;
char	*tzname[] = {"EST", "EDT",};

struct tm *gmtime(), *localtime();
char	*ctime(), *asctime();
void	tzset();

static char cbuf[26];
static int dmsize[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*
 * The following table is used for 1974 and 1975 and
 * gives the day number of the first day after the Sunday of the
 * change.
 */
static struct {
	int	daylb;
	int	dayle;
} daytab[] = {
	5,	333,	/* 1974: Jan 6 - last Sun. in Nov */
	58,	303,	/* 1975: Last Sun. in Feb - last Sun in Oct */
};


char *
ctime(t)
long	*t;
{
	return(asctime(localtime(t)));
}

struct tm *
localtime(tim)
long	*tim;
{
	register int dayno, daylbegin, daylend;
	register struct tm *ct;
	long copyt;

	tzset();
	copyt = *tim - timezone;
	ct = gmtime(&copyt);
	dayno = ct->tm_yday;
	daylbegin = 119;	/* last Sun in Apr */
	daylend = 303;		/* Last Sun in Oct */
	if(ct->tm_year == 74 || ct->tm_year == 75) {
		daylbegin = daytab[ct->tm_year-74].daylb;
		daylend = daytab[ct->tm_year-74].dayle;
	}
	daylbegin = sunday(ct, daylbegin);
	daylend = sunday(ct, daylend);
	if(daylight &&
	    (dayno>daylbegin || (dayno==daylbegin && ct->tm_hour>=2)) &&
	    (dayno<daylend || (dayno==daylend && ct->tm_hour<1))) {
		copyt += 1*60*60;
		ct = gmtime(&copyt);
		ct->tm_isdst++;
	}
	return(ct);
}

/*
 * The argument is a 0-origin day number.
 * The value is the day number of the last	DAG -- comment changed
 * Sunday on or before the day.			DAG -- as per David Tilbrook
 */

static int
sunday(t, d)
register struct tm *t;
register int d;
{
	if(d >= 58)
		d += dysize(t->tm_year) - 365;
	return(d - (d - t->tm_yday + t->tm_wday + 700) % 7);
}

struct tm *
gmtime(tim)
long	*tim;
{
	register int d0, d1;
	long hms, day;
	static struct tm xtime;

	/*
	 * break initial number into days
	 */
	hms = *tim % 86400L;
	day = *tim / 86400L;
	if(hms < 0) {
		hms += 86400L;
		day -= 1;
	}
	/*
	 * generate hours:minutes:seconds
	 */
	xtime.tm_sec = hms % 60;
	d1 = hms / 60;
	xtime.tm_min = d1 % 60;
	d1 /= 60;
	xtime.tm_hour = d1;

	/*
	 * day is the day number.
	 * generate day of the week.
	 * The addend is 4 mod 7 (1/1/1970 was Thursday)
	 */

	xtime.tm_wday = (day + 7340036L) % 7;

	/*
	 * year number
	 */
	if(day >= 0)
		for(d1=70; day >= dysize(d1); d1++)
			day -= dysize(d1);
	else
		for(d1=70; day < 0; d1--)
			day += dysize(d1-1);
	xtime.tm_year = d1;
	xtime.tm_yday = d0 = day;

	/*
	 * generate month
	 */

	if(dysize(d1) == 366)
		dmsize[1] = 29;
	for(d1=0; d0 >= dmsize[d1]; d1++)
		d0 -= dmsize[d1];
	dmsize[1] = 28;
	xtime.tm_mday = d0+1;
	xtime.tm_mon = d1;
	xtime.tm_isdst = 0;
	return(&xtime);
}

char *
asctime(t)
struct tm *t;
{
	register char *cp, *ncp;
	register int *tp;
	char	*ct_numb();

	cp = cbuf;
	for(ncp = "Day Mon 00 00:00:00 1900\n"; *cp++ = *ncp++; );
	ncp = &"SunMonTueWedThuFriSat"[3*t->tm_wday];
	cp = cbuf;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	cp++;
	tp = &t->tm_mon;
	ncp = &"JanFebMarAprMayJunJulAugSepOctNovDec"[(*tp)*3];
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	cp = ct_numb(cp, *--tp);
	cp = ct_numb(cp, *--tp+100);
	cp = ct_numb(cp, *--tp+100);
	cp = ct_numb(cp, *--tp+100);
	if(t->tm_year >= 100) {
		cp[1] = '2';
		cp[2] = '0';
	}
	cp += 2;
	cp = ct_numb(cp, t->tm_year+100);
	return(cbuf);
}

static char *
ct_numb(cp, n)
register char *cp;
int	n;
{
	cp++;
	if(n >= 10)
		*cp++ = (n/10)%10 + '0';
	else
		*cp++ = ' ';
	*cp++ = n%10 + '0';
	return(cp);
}

void
tzset()
{
	register char *p, *q;
	register int n;
	int sign;
	char *getenv();

	if((p = getenv ("TZ")) && *p) {
		n = 3;
		q = tzname[0];
		do {
			*q++ = *p? *p++: ' ';
		} while(--n);
		if(sign = *p == '-')
			p++;
		n = 0;
		while(*p >= '0' && *p <= '9')
			n = (n * 10) + *p++ - '0';
		if(sign)
			n = -n;
		timezone = ((long)(n * 60)) * 60;
		if(daylight = *p != '\0') {
			q = tzname[1];
			n = 3;
			do {
				*q++ = *p? *p++: ' ';
			} while(--n);
		}
	}
#if vax || sun || gould	/* DAG -- i.e., if 4.2BSD */
	else	{			/* TZ not set in evironment */
		extern int	gettimeofday();
		static struct tzinfo	/* time zone names */
		{
			long	secwest;	/* sec west of GMT */
			char	*stdzone;	/* standard name */
			char	*dstzone;	/* DST name */
		}	tzi[] =
		{
			{	5L * 60L * 60L,	"EST",	"EDT"	},
			{	6L * 60L * 60L,	"CST",	"CDT"	},
			{	7L * 60L * 60L,	"MST",	"MDT"	},
			{	8L * 60L * 60L,	"PST",	"PDT"	},
			{	0L * 60L * 60L,	"GMT",	"?DT"	},
			/* add your favorite time zones here */
		};
		register struct tzinfo	*tzip;
		struct
		{
			unsigned long	tv_sec;	/* since Jan. 1, 1970 */
			long		tv_usec;	/* microsec */
		}	tv;		/* real time from kernel */
		struct
		{
			long	tz_minuteswest;	/* of Greenwich */
			long	tz_dsttime;	/* "DST is in effect" */
		}	tz;		/* time zone data from kernel */

		if ( gettimeofday( &tv, &tz ) == 0 )
			{
			timezone = tz.tz_minuteswest * 60L;
			daylight = tz.tz_dsttime != 0L;
			/* look zone names up in table */
			for ( tzip = tzi;
			      tzip < &tzi[sizeof tzi / sizeof tzi[0]];
			      ++tzip
			    )
				if ( tzip->secwest == timezone )
					{
					tzname[0] = tzip->stdzone;
					tzname[1] = tzip->dstzone;
					return;
					}
			/* zone names not found in table */
			tzname[0] = "?ST";
			tzname[1] = "?DT";
			}
		/* else use hard-coded default values */
		}
#endif
}
