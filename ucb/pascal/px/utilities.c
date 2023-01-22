#ifndef lint
static char	*sccsid = "@(#)utilities.c	1.2	(ULTRIX)	1/24/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
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
*
*			Modification History
*
*		David Metsky,	23-Jan-86
*
* 001	Replaced old version with BSD 4.3 version as part of upgrade
*
*	Based on:	utilities.c	5.1 (Berkeley)	6/5/85
*
*************************************************************************/

#include	<signal.h>
#include	"whoami.h"
#include	"vars.h"
#include	"objfmt.h"
#include	<sys/time.h>
#include	<sys/resource.h>

stats()
{
	struct rusage ru;
	register double l;
	register long count;
#	ifdef PROFILE
#	define	proffile	"/vb/grad/mckusick/px/profile/pcnt.out"
	struct cntrec {
		double	counts[NUMOPS];	/* instruction counts */
		long	runs;		/* number of interpreter runs */
		long	startdate;	/* date profile started */
		long	usrtime;	/* total user time consumed */
		long	systime;	/* total system time consumed */
		double	stmts;		/* number of pascal stmts executed */
	} profdata;
	FILE *datafile;
#	endif PROFILE

	if (_nodump)
		return(0);
	getrusage(RUSAGE_SELF, &ru);
#	ifdef PROFILE
	datafile = fopen(proffile,"r");
	if (datafile == NULL)
		goto skipprof;
	count = fread(&profdata,1,sizeof(profdata),datafile);
	if (count != sizeof(profdata))
		goto skipprof;
	for (count = 0;  count < NUMOPS;  count++)
		profdata.counts[count] += _profcnts[count];
	profdata.runs += 1;
	profdata.stmts += _stcnt;
	profdata.usrtime += ru.ru_utime.tv_sec;
	profdata.systime += ru.ru_stime.tv_sec;
	datafile = freopen(proffile,"w",datafile);
	if (datafile == NULL)
		goto skipprof;
	count = fwrite(&profdata,1,sizeof(profdata),datafile);
	if (count != sizeof(profdata))
		goto skipprof;
	fclose(datafile);
skipprof:
#	endif PROFILE
	fprintf(stderr,
		"\n%1ld statements executed in %d.%02d seconds cpu time.\n",
		_stcnt, ru.ru_utime.tv_sec, ru.ru_utime.tv_usec / 10000);
}

backtrace(type)
	char	*type;
{
	register struct dispsave *mydp;
	register struct blockmark *ap;
	register char *cp;
	register long i, linum;
	struct display disp;

	if (_lino <= 0) {
		fputs("Program was not executed.\n",stderr);
		return;
	}
	disp = _display;
	fprintf(stderr, "\n\t%s in \"", type);
	mydp = _dp;
	linum = _lino;
	for (;;) {
		ap = mydp->stp;
		i = linum - (((ap)->entry)->offset & 0177777);
		fprintf(stderr,"%s\"",(ap->entry)->name);
		if (_nodump == FALSE)
			fprintf(stderr,"+%D near line %D.",i,linum);
		fputc('\n',stderr);
		*mydp = (ap)->odisp;
		if (mydp <= &_display.frame[1]){
			_display = disp;
			return;
		}
		mydp = (ap)->dp;
		linum = (ap)->lino;
		fputs("\tCalled by \"",stderr);
	}
}

psexit(code)

	int	code;
{
	if (_pcpcount != 0)
		PMFLUSH(_cntrs, _rtns, _pcpcount);
	if (_mode == PIX) {
		fputs("Execution terminated",stderr);
		if (code)
			fputs(" abnormally",stderr);
		fputc('.',stderr);
		fputc('\n',stderr);
	}
	stats();
	exit(code);
}

/*
 * Routines to field various types of signals
 *
 * catch a library error and generate a backtrace
 */
liberr()
{
	backtrace("Error");
	psexit(2);
}

/*
 * catch an interrupt and generate a backtrace
 */
intr()
{
	signal(SIGINT, intr);
	backtrace("Interrupted");
	psexit(1);
}

/*
 * catch memory faults
 */
memsize()
{
	signal(SIGSEGV, memsize);
	ERROR("Run time stack overflow\n");
}

/*
 * catch random system faults
 */
syserr(signum)
	int signum;
{
	signal(signum, syserr);
	ERROR("Panic: Computational error in interpreter\n");
}
