# ifndef lint
static char *sccsid = "@(#)mt.c	1.14	ULTRIX	2/12/87";
# endif not lint

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
 * mt --
 *   magnetic tape manipulation program
 */

/* ---------------------------------------------------------------------
 * Modification History
 *
 * Feb 11 1986  rsp     (Ricky Palmer)
 *	Removed "don't grok" error message.
 *
 * Sep 11 1986  fred	(Fred Canter)
 *	Bug fix to allow "mt status" to work with VAXstar TZK50.
 *
 * Sep  9 1986  fries	Corrected bugs whereas device was opened twice
 *			could not perform non-write functions to a write
 *			protected device.
 *
 * Aug 27 1986  fries	Made commands: clserex, clhrdsf, clsub, eoten
 *			and eotdis read only commands.
 *
 * Apr 28 1986  fries	Added commands: clserex, clhrdsf, clsub, eoten
 *			and eotdis. Added code to insure only superuser
 *			can change eot detect flag.
 *
 * Feb 10 1986  fries	Added commands: clserex, clhrdsf, clsub, eoten
 *			and eotdis. Added code to insure only superuser
 *			can change eot detect flag.
 *
 * Jan 29 1986  fries	Changed default tape definition DEFTAPE to
 *			DEFTAPE_NH to coincide with new mtio.h naming
 *			convention.
 *
 * Jul 17 1985	afd	on status command, check for tmscp device types
 *			and interpret the returned data accordingly.
 *
 * Jul 17 1985	afd	Added mt ops: "cache" and "nocache" to enable &
 *			disable caching on tmscp units.
 *
 * Dec 6 1984	afd	took out references to Sun tape devices and
 *			added tmscp device to the "tapes" table.
 *
 * Dec 6 1984	afd	derived from Berkeley 4.2BSD labeled
 * 			"@(#)mt.c	4.8 (Berkeley) 83/05/08"
 *
 * ---------------------------------------------------------------------
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>

#ifdef DEBUG
#include "../ioctl.h"
#include "../devio.h"
#include "../mtio.h"
#else
#include <sys/ioctl.h>
#include <sys/devio.h>
#include <sys/mtio.h>
#endif

#include <sys/errno.h>
#include <sys/file.h>

#define	equal(s1,s2)	(strcmp(s1, s2) == 0)

struct devget mt_info;

struct commands {
	char *c_name;
	int c_code;
	int c_ronly;
	int c_onlnck;
} com[] = {
	{ "weof",	MTWEOF,	0 , 1 },
	{ "eof",	MTWEOF,	0 , 1 },
	{ "fsf",	MTFSF,	1 , 1 },
	{ "bsf",	MTBSF,	1 , 1 },
	{ "fsr",	MTFSR,	1 , 1 },
	{ "bsr",	MTBSR,	1 , 1 },
	{ "rewind",	MTREW,	1 , 1 },
	{ "offline",	MTOFFL,	1 , 1 },
	{ "rewoffl",	MTOFFL,	1 , 1 },
	{ "status",	MTNOP,	1 , 0 },
	{ "cache",	MTCACHE, 1 , 0 },
	{ "nocache",	MTNOCACHE, 1 , 0 },
	{ "clserex",	MTCSE, 1 , 0 },
	{ "clhrdsf",	MTCLX, 1 , 0 },
	{ "clsub",	MTCLS, 1 , 0 },
	{ "eoten",	MTENAEOT, 1 , 0 },
	{ "eotdis",	MTDISEOT, 1 , 0 },
	{ 0 }
};

int mtfd;
int generic_ioctl_suceed;

struct mtop mt_com;
struct mtget mt_status;
char *tape;

main(argc, argv)
	char **argv;
{
	char line[80], *getenv();
	register char *cp;
	register struct commands *comp;

	if (argc > 2 && (equal(argv[1], "-t") || equal(argv[1], "-f"))) {
		argc -= 2;
		tape = argv[2];
		argv += 2;
	} else
		if ((tape = getenv("TAPE")) == NULL)
			tape = DEFTAPE_NH;
	if (argc < 2) {
		fprintf(stderr, "usage: mt [ -f device ] command [ count ]\n");
		exit(1);
	}
	cp = argv[1];
	for (comp = com; comp->c_name != NULL; comp++)
		if (strncmp(cp, comp->c_name, strlen(cp)) == 0)
			break;
	if (comp->c_name == NULL) {
		fprintf(stderr, "mt: invalid command: \"%s\"\n", cp);
		exit(1);
	}

	if ((mtfd = statchk(tape, comp->c_ronly ? 0 : 2, comp->c_onlnck)) < 0) {
		if (errno)perror(tape);
		exit(1);
	}
	/* check for enable or disable eot(must be superuser) */
	if ((comp->c_code == MTENAEOT || comp->c_code == MTDISEOT) && geteuid())
           {
            fprintf(stderr, "mt: must be Superuser to perform %s\n",comp->c_name);
	    exit(1);
	    }

	if (comp->c_code != MTNOP) {
		mt_com.mt_op = comp->c_code;
		mt_com.mt_count = (argc > 2 ? atoi(argv[2]) : 1);
		if (mt_com.mt_count < 0) {
			fprintf(stderr, "mt: negative repeat count\n");
			exit(1);
		}
		if (ioctl(mtfd, MTIOCTOP, &mt_com) < 0) {
			fprintf(stderr, "%s %s %d ", tape, comp->c_name,
				mt_com.mt_count);

			/* The following test is in case you are past */
			/* the EOT and rewind, the rewind completes   */
			/* but an errno of ENOSPC is returned...      */
			/* ALL OTHER ERRORS are REPORTED              */
			if ((mt_com.mt_op == MTREW) && (errno == ENOSPC))
						;/* do nothing */
			else
				perror("failed");/* else perror */
			exit(2);
		}
	} else {
		if (ioctl(mtfd, MTIOCGET, (char *)&mt_status) < 0) {
			perror("mt");
			exit(2);
		}
		status(&mt_status);
	}
}

#ifdef vax
#include <vaxmba/mtreg.h>
#include <vaxmba/htreg.h>

#include <vaxuba/utreg.h>
#include <vaxuba/tmreg.h>
#undef b_repcnt		/* argh */
#include <vaxuba/tsreg.h>
#endif

struct tape_desc {
	short	t_type;		/* type of magtape device */
	char	*t_name;	/* printing name */
	char	*t_dsbits;	/* "drive status" register */
	char	*t_erbits;	/* "error" register */
} tapes[] = {
	{ MT_ISTS,	"ts11",		0,		TSXS0_BITS },
	{ MT_ISHT,	"tm03",		HTDS_BITS,	HTER_BITS },
	{ MT_ISTM,	"tm11",		0,		TMER_BITS },
	{ MT_ISMT,	"tu78",		MTDS_BITS,	0 },
	{ MT_ISUT,	"tu45",		UTDS_BITS,	UTER_BITS },
	{ MT_ISTMSCP,	"tmscp",	0,		0 },
	{ MT_ISST,	"tzk50",	0,		0 },
	{ 0 }
};

/*
 * Interpret the status buffer returned
 */
status(bp)
	register struct mtget *bp;
{
	register struct tape_desc *mt;

	for (mt = tapes; mt->t_type; mt++)
		if (mt->t_type == bp->mt_type)
			break;
	if (mt->t_type == 0) {
		printf("unknown tape drive type (%d)\n", bp->mt_type);
		return;
	}
	if(generic_ioctl_suceed)
	   printf("%s tape drive, residual=%d\n", mt_info.device, bp->mt_resid);
	else
	   printf("%s tape drive, residual=%d\n", mt->t_name, bp->mt_resid);

	/*
	 * If its a tmscp device, then print out the info stored in the
	 * dsreg & erreg fields.  Of course tmscp devices do not really
	 * have such registers, so we return: flags, endcode, & cmd status.
	 */
	if (mt->t_type == MT_ISTMSCP) {
		printf("flags=0%o\n", (bp->mt_dsreg >> 8) & 0xFF);
		printf("endcode=0%o\n", bp->mt_dsreg & 0xFF);
		printf("status=0%o", bp->mt_erreg);
	}
	else {
		printreg("ds", bp->mt_dsreg, mt->t_dsbits);
		printreg("\ner", bp->mt_erreg, mt->t_erbits);
	}
	putchar('\n');
}

/*
 * Print a register a la the %b format of the kernel's printf
 */
printreg(s, v, bits)
	char *s;
	register char *bits;
	register unsigned short v;
{
	register int i, any = 0;
	register char c;

	if (bits && *bits == 8)
		printf("%s=%o", s, v);
	else
		printf("%s=%x", s, v);
	bits++;
	if (v && bits) {
		putchar('<');
		while (i = *bits++) {
			if (v & (1 << (i-1))) {
				if (any)
					putchar(',');
				any = 1;
				for (; (c = *bits) > 32; bits++)
					putchar(c);
			} else
				for (; *bits > 32; bits++)
					;
		}
		putchar('>');
	}
}

/* Routine to obtain generic device status */
statchk(tape,mode, c_onlnck)
	char	*tape;
	int	mode, c_onlnck;
{
	int to;
	int error = 0;
	
	generic_ioctl_suceed = 0;

	/* Force device open to obtain status */
	to = open(tape,mode|O_NDELAY);

	/* If open error, then error must be no such device and address */
	if (to < 0)return(-1);
	
	/* Get generic device status */
	if (ioctl(to,DEVIOCGET,(char *)&mt_info) < 0)return(to);

	/* Set flag indicating successful generic ioctl */
	generic_ioctl_suceed = 1;

	/* Check for device on line */
	if((c_onlnck) && (mt_info.stat & DEV_OFFLINE)){
	  fprintf(stderr,"\7\nError on device named %s - Place %s tape drive unit #%u ONLINE\n",tape,mt_info.device,mt_info.unit_num);
	  return(-2);
	}

	/* Check for device write locked when in write mode */
	if((c_onlnck) && (mt_info.stat & DEV_WRTLCK) && (mode != O_RDONLY)){
           fprintf(stderr,"\7\nError on device named %s - WRITE ENABLE %s tape drive unit #%u\n",tape,mt_info.device,mt_info.unit_num);
	   return(-3);
	 }
	   
	 /* All checked out ok, return file descriptor */
	 return(to);
}
