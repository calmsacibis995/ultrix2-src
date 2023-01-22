

#ifndef lint
static	char	*sccsid = "@(#)ipcs.c	1.2	(ULTRIX)	1/29/87";
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
 *
 *   Modification history:
 *
 *  30 Apr 85 -- depp
 *	New file to perform system maintenance on System V IPC
 *
 */

/*
**	ipcs - IPC status
**	Examine and print certain things about message queues, semaphores,
**		and shared memory.
*/

#include	<machine/pte.h>

#include	<sys/param.h>
#include	<sys/vm.h>
#include	<sys/ipc.h>
#include	<sys/msg.h>
#include	<sys/sem.h>
#include	<sys/shm.h>
#include	<nlist.h>
#include	<fcntl.h>
#include	<sys/time.h>
#include	<grp.h>
#include	<pwd.h>
#include	<stdio.h>

#define	TIME	0
#define	MSG	1
#define	SEM	2
#define	SHM	3
#define	MSGINFO	4
#define	SEMINFO	5
#define	SHMINFO	6

struct nlist    nl[] =
{			/* name list entries for IPC facilities */
    { "_time" },{ "_msgque" },{ "_sema" },{ "_smem" },{ "_msginfo" },
    { "_seminfo" }, { "_sminfo" }, 
    { "_Sysmap" },
#define SSYSMAP		7
    { "_Syssize" },
#define SSYSSIZE	8
    { NULL },
};


char    chdr[] = "T     ID     KEY        MODE       OWNER    GROUP",
 /* common header format */
        chdr2[] = "  CREATOR   CGROUP",
 /* c option header format */
       *name = "/vmunix",	/* name list file */
       *mem = "/dev/kmem",	/* memory file */
        opts[] = "abcmopqstC:N:";/* allowable options for getopt */
extern char *optarg;		/* arg pointer for getopt */
int     bflg,		/* biggest size: segsz on m; qbytes on q; nsems on s */
        cflg,		/* creator's login and group names */
        mflg,		/* shared memory status */
        oflg,		/* outstanding data: nattch on m; cbytes, qnum on q */
        pflg,		/* process id's: lrpid, lspid on q; cpid, lpid on m */
        qflg,		/* message queue status */
        sflg,		/* semaphore status */
        tflg,		/* times: atime, ctime, dtime on m; ctime,
				   rtime, stime on q; ctime, otime on s */
	nflg,		/* indicator that a particular IPC capability is
				   currently not being used */
        err;		/* option error count */

extern int  optind;		/* option index for getopt */

extern char *ctime ();
extern struct group *getgrgid ();
extern struct passwd   *getpwuid ();
extern long lseek ();

char	*kmemf, *memf, *swapf, *nlistf;
int	kmem, kflg, swap = -1;
#define clear(x) 	((int)x & 0x7fffffff)

main (argc, argv)
int     argc;			/* arg count */
char  **argv;			/* arg vector */
{
    register int    i,		/* loop control */
                    md,		/* memory file file descriptor */
                    o;		/* option flag */
    time_t time;		/* date in memory file */
    struct shmid_ds mds;	/* shared memory data structure */
    struct shminfo  shminfo;	/* shared memory information structure */
    struct msqid_ds qds;	/* message queue data structure */
    struct msginfo  msginfo;	/* message information structure */
    struct semid_ds sds;	/* semaphore data structure */
    struct seminfo  seminfo;	/* semaphore information structure */

    /* Go through the options and set flags. */
    while ((o = getopt (argc, argv, opts)) != EOF)
	switch (o)
	{
	    case 'a': 
		bflg = cflg = oflg = pflg = tflg = 1;
		break;
	    case 'b': 
		bflg = 1;
		break;
	    case 'c': 
		cflg = 1;
		break;
	    case 'C': 
		mem = optarg;
		break;
	    case 'm': 
		mflg = 1;
		break;
	    case 'N': 
		name = optarg;
		break;
	    case 'o': 
		oflg = 1;
		break;
	    case 'p': 
		pflg = 1;
		break;
	    case 'q': 
		qflg = 1;
		break;
	    case 's': 
		sflg = 1;
		break;
	    case 't': 
		tflg = 1;
		break;
	    case '?': 
		err++;
		break;
	}
    if (err || (optind < argc))
    {
	fprintf (stderr,
		"usage:  ipcs [-abcmopqst] [-C corefile] [-N namelist]\n");
	exit (1);
    }
    if ((mflg + qflg + sflg) == 0)
	mflg = qflg = sflg = 1;

    /* Check out namelist and memory files. */
    nlist (name, nl);
    if (!nl[TIME].n_value)
    {
	fprintf (stderr, "ipcs:  no namelist\n");
	exit (1);
    }
	openfiles();
/*
    if ((md = open (mem, O_RDONLY)) < 0)
    {
	fprintf (stderr, "ipcs:  no memory file\n");
	exit (1);
    }
*/
    klseek (kmem, (long) nl[TIME].n_value, 0);

/*
    puts ("before reade time");
    printf ("&time = %o\n", &time);
*/
    reade (kmem, &time, sizeof (time));
/*
    puts ("after reade time");
*/

    printf ("\nIPC status from %s as of %s", mem, ctime (&time));

    /* Print Message Queue status report. */
    if (qflg)
    {
	if (nl[MSG].n_value)
	{
	    i = 0;
	    klseek (kmem, (long) nl[MSGINFO].n_value, 0);
	    reade (kmem, &msginfo, sizeof (msginfo));
	    klseek (kmem, (long) nl[MSG].n_value, 0);
	    printf ("Message Queues:\n%s%s%s%s%s%s\n", chdr,
		    cflg ? chdr2 : "",
		    oflg ? " CBYTES  QNUM" : "",
		    bflg ? " QBYTES" : "",
		    pflg ? " LSPID LRPID" : "",
		    tflg ? "   STIME    RTIME    CTIME " : "");
	} else
	{
	    i = msginfo.msgmni;
	    printf ("Message Queue facility not in system.\n\n");
	}
	nflg = 1;
	while (i < msginfo.msgmni)
	{
	    reade (kmem, &qds, sizeof (qds));
	    if (!(qds.msg_perm.mode & IPC_ALLOC))
	    {
		i++;
		continue;
	    }
	    hp ('q', "SRrw-rw-rw-", &qds.msg_perm, i++, msginfo.msgmni);
	    if (oflg)
		printf ("%7u%6u", qds.msg_cbytes, qds.msg_qnum);
	    if (bflg)
		printf ("%7u", qds.msg_qbytes);
	    if (pflg)
		printf ("%6u%6u", qds.msg_lspid, qds.msg_lrpid);
	    if (tflg)
	    {
		tp (qds.msg_stime);
		tp (qds.msg_rtime);
		tp (qds.msg_ctime);
	    }
	    printf ("\n");
	    nflg = 0;
	}
	if (nflg)
		printf("*** No message queues are currently defined ***\n\n");
	else
		printf("\n");
    }

    /* Print Shared Memory status report. */
    if (mflg)
    {
	if (nl[SHM].n_value)
	{
	    i = 0;
	    klseek (kmem, (long) nl[SHMINFO].n_value, 0);
	    reade (kmem, &shminfo, sizeof (shminfo));
	    klseek (kmem, (long) nl[SHM].n_value, 0);
	    printf ("Shared Memory\n%s%s%s%s%s%s\n", chdr,
		cflg ? chdr2 : "",
		oflg ? " NATTCH" : "",
		bflg ? "  SEGSZ" : "",
		pflg ? "  CPID  LPID" : "",
		tflg ? "   ATIME    DTIME    CTIME " : "");
	}
	else
	{
	    i = shminfo.shmmni;
	    printf ("Shared Memory facility not in system.\n\n");
	}
	nflg = 1;
	while (i < shminfo.shmmni)
	{
	    reade (kmem, &mds, sizeof (mds));
	    if (!(mds.shm_perm.mode & IPC_ALLOC))
	    {
		i++;
		continue;
	    }
	    hp ('m', "DCrw-rw-rw-", &mds.shm_perm, i++, shminfo.shmmni);
	    if (oflg)
		printf ("%7u", mds.shm_nattch);
	    if (bflg)
		printf ("%7d", mds.shm_segsz);
	    if (pflg)
		printf ("%6u%6u", mds.shm_cpid, mds.shm_lpid);
	    if (tflg)
	    {
		tp (mds.shm_atime);
		tp (mds.shm_dtime);
		tp (mds.shm_ctime);
	    }
	    printf ("\n");
	    nflg = 0;
	}
	if (nflg)
		printf("*** No shared memory segments are currently defined ***\n\n");
	else
		printf("\n");
    }

    /* Print Semaphore facility status. */
    if (sflg)
    {
	if (nl[SEM].n_value)
	{
	    i = 0;
	    klseek (kmem, (long) nl[SEMINFO].n_value, 0);
	    reade (kmem, &seminfo, sizeof (seminfo));
	    klseek (kmem, (long) nl[SEM].n_value, 0);
	    printf ("Semaphores\n%s%s%s%s\n", chdr,
		cflg ? chdr2 : "",
		bflg ? " NSEMS" : "",
		tflg ? "   OTIME    CTIME " : "");
	}
	else
	{
	    i = seminfo.semmni;
	    printf ("Semaphore facility not in system.\n\n");
	}
	nflg = 1;
	while (i < seminfo.semmni)
	{
	    reade (kmem, &sds, sizeof (sds));
	    if (!(sds.sem_perm.mode & IPC_ALLOC))
	    {
		i++;
		continue;
	    }
	    hp ('s', "--ra-ra-ra-", &sds.sem_perm, i++, seminfo.semmni);
	    if (bflg)
		printf ("%6u", sds.sem_nsems);
	    if (tflg)
	    {
		tp (sds.sem_otime);
		tp (sds.sem_ctime);
	    }
	    printf ("\n");
	    nflg = 0;
	}
	if (nflg)
		printf("*** No semaphores are currently defined ***\n\n");
	else
		printf("\n");
    }
    exit (0);
}

struct pte *Sysmap = 0;

klseek(fd, loc, off)
	int fd;
	long loc;
	int off;
{
	static int	sizeSysmap;
int a;
extern errno;

	if( kflg && Sysmap == 0)
		{/* initialize Sysmap */

		sizeSysmap = nl[SSYSSIZE].n_value * sizeof( struct pte);
		Sysmap = (struct pte *)calloc( sizeSysmap, 1);
		a = lseek( kmem, (long)clear( nl[SSYSMAP].n_value), 0);
if(a == -1)
printf("sizeSysmap = %d    a = %d    errno = %d\n",
sizeSysmap, a, errno);
		if( (a = read( kmem, Sysmap, sizeSysmap)) != sizeSysmap)
			{
printf("sizeSysmap = %d    a = %d    errno = %d\n",
sizeSysmap, a, errno);
printf("kmem = %d    Sysmap = 0x%x\n", kmem, Sysmap);
			cantread( "system page table", kmemf);
			exit(1);
			}
		}
	if( kflg && (loc&0x80000000))
		{/* do mapping for kernel virtual addresses */
		struct pte *ptep;

		loc &= 0x7fffffff;
		ptep = &Sysmap[btop(loc)];
		if( (char *)ptep - (char *)Sysmap > sizeSysmap)
			{
			printf( "no system pte for %s\n", loc);
			exit(1);
			}
		if( ptep->pg_v == 0)
			{
			printf( "system pte invalid for %x\n", loc);
			exit(1);
			}
		loc = (off_t)((loc&PGOFSET) + ptob(ptep->pg_pfnum));
		}
	(void) lseek(fd, (long)loc, off);
}

openfiles()
/*
openfiles(argc, argv)
	char **argv;
*/
{

	kmemf = "/dev/kmem";
/*
	if (kflg)
		kmemf = argc > 2 ? argv[2] : "/vmcore";
*/
	kmem = open(kmemf, 0);
	if (kmem < 0) {
		perror(kmemf);
		exit(1);
	}
/*
	if (kflg)  {
		mem = kmem;
		memf = kmemf;
	} else {
		memf = "/dev/mem";
		mem = open(memf, 0);
		if (mem < 0) {
			perror(memf);
			exit(1);
		}
	}
	if (kflg == 0 || argc > 3) {
		swapf = argc>3 ? argv[3]: "/dev/drum";
		swap = open(swapf, 0);
		if (swap < 0) {
			perror(swapf);
			exit(1);
		}
	}
*/
}

/*
**	reade - read with error exit
*/

reade (f, b, s)
int     f;			/* fd */
char   *b;			/* buffer address */
int     s;			/* size */
{
    if (read (f, b, s) != s)
    {
	perror ("ipcs:  read error");
	printf ("%o\n", b);
	exit (1);
    }
}

/*
**	hp - common header print
*/

hp (type, modesp, permp, slot, slots)
char    type,			/* facility type */
       *modesp;			/* ptr to mode replacement characters */
register struct ipc_perm   *permp;/* ptr to permission structure */
int     slot,			/* facility slot number */
        slots;			/* # of facility slots */
{
    register int    i,		/* loop control */
                    j;		/* loop control */
    register struct group  *g;	/* ptr to group group entry */
    register struct passwd *u;	/* ptr to user passwd entry */

/*
    printf ("%c%7d%s%#8.8x ", type, slot + slots * permp -> seq,
	    permp -> key ? " " : " 0x", permp -> key);
	    */
    printf ("%c%7d %10d ", type, slot + slots * permp -> seq, permp -> key);
    for (i = 02000; i; modesp++, i >>= 1)
	printf ("%c", (permp -> mode & i) ? *modesp : '-');
    if ((u = getpwuid (permp -> uid)) == NULL)
	printf ("%9d", permp -> uid);
    else
	printf ("%9.8s", u -> pw_name);
    if ((g = getgrgid (permp -> gid)) == NULL)
	printf ("%9d", permp -> gid);
    else
	printf ("%9.8s", g -> gr_name);
    if (cflg)
    {
	if ((u = getpwuid (permp -> cuid)) == NULL)
	    printf ("%9d", permp -> cuid);
	else
	    printf ("%9.8s", u -> pw_name);
	if ((g = getgrgid (permp -> cgid)) == NULL)
	    printf ("%9d", permp -> cgid);
	else
	    printf ("%9.8s", g -> gr_name);
    }
}

/*
**	tp - time entry printer
*/

tp (time)
time_t time;			/* time to be displayed */
{
    register struct tm *t;	/* ptr to converted time */

    if (time)
    {
	t = localtime (&time);
	printf (" %2d:%2.2d:%2.2d", t -> tm_hour, t -> tm_min, t -> tm_sec);
    }
    else
	printf (" no-entry");
}

cantread(what, fromwhat)
	char *what, *fromwhat;
{

	fprintf(stderr,
		"ipcs: error reading %s from %s\n", what, fromwhat);
}
