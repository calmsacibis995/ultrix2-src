
#ifndef lint
static char *sccsid = "@(#)param.c	1.19	ULTRIX	1/29/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985, 1986 by			*
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
 * Modification History
 *
 *	15-jan-87 -- koehler
 *	when calculating the number of gnodes, add the number of text
 *	slots needed
 *
 *	10-Jul-86 -- tresvik
 *	moved desfree, minfree, and lotsfree here from ../sys/vm_sched.c
 *	so that default paging limits could be overriden when running a
 *	SAS kernel.
 *
 *	29 Apr 86 -- depp
 *	Modified shared memory configuration data so that it's config file
 *	configurable.
 *
 *	09 Sep 85 -- reilly
 *	Modifed to handle the lockf call.
 *
 *	12 Aug 85 -- depp
 *	Changed sizing of "sem_undo" using NPROC rather than SEMUND since
 *	the addressing method is "sem_undo[u.u_procp - proc]".
 *
 *	12 Mar 85 -- depp
 *	Added kernel data structures for System V shared memory
 *
 *	27 Feb 85 -- depp
 *	Added kernel data structures for System V semaphores and Message queues
 *
 */


#include "../h/param.h"
#include "../h/systm.h"
#include "../h/socket.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/gnode.h"
#include "../h/file.h"
#include "../h/callout.h"
#include "../h/clist.h"
#include "../h/cmap.h"
#include "../h/mbuf.h"
#include "../h/quota.h"
#include "../h/kernel.h"
#include "../h/map.h"
#include "../h/ipc.h"
#include "../h/msg.h"
#include "../h/sem.h"
#include "../h/shm.h"
#include "../h/flock.h"
#include "../h/cpudata.h"
#include "../h/interlock.h"

/*
 * System parameter formulae.
 *
 * This file is copied into each directory where we compile
 * the kernel; it should be modified there to suit local taste
 * if necessary.
 *
 * Compiled with -DHZ=xx -DTIMEZONE=x -DDST=x -DMAXUSERS=xx
 */

#define	HZ 100
int	hz = HZ;
int	tick = 1000000 / HZ;
int     tickadj = 240000 / (60 * HZ);           /* can adjust 240ms in 60s */
struct	timezone tz = { TIMEZONE, DST };
#define	NPROC (20 + 8 * MAXUSERS)
int	nproc = NPROC;
#ifdef INET
#define	NETSLOP	20			/* for all the lousy servers*/
#else
#define	NETSLOP	0
#endif
#define NTEXT (24 + MAXUSERS + NETSLOP)
int	ntext = NTEXT;
#define	NGNODE ((NPROC + 16 + MAXUSERS) + 32 + NTEXT)
int	ngnode = NGNODE;
int	nchsize = NGNODE * 11 / 10;
struct flckinfo flckinfo = {  4 * ( NGNODE / 10 ), NGNODE / 10 };
int	nfile = 16 * (NPROC + 16 + MAXUSERS) / 10 + 32 + 2 * NETSLOP;
int	ncallout = 16 + NPROC;
int	nclist = 100 + 16 * MAXUSERS;
int	nport = NPROC / 2;
int     nmbclusters = NMBCLUSTERS;
int	nquota = (MAXUSERS * 9)/7 + 3;
int	ndquot = (MAXUSERS*NMOUNT)/4 + NPROC;
int 	maxusers = MAXUSERS;
int 	maxuprc	= MAXUPRC;
int	dmmin = DMMIN;
int	dmmax = DMMAX;

/*
 * These are initialized at bootstrap time
 * to values dependent on memory size
 */
int	nbuf, nswbuf;

/*
 * These have to be allocated somewhere; allocating
 * them here forces loader errors if this file is omitted.
 */
struct	proc *proc, *procNPROC;
struct	text *text, *textNTEXT;
struct	gnode *gnode, *gnodeNGNODE;
struct	file *file, *fileNFILE;
struct filock  	*flox;
struct flino   	*flinotab;
struct 	callout *callout;
struct	cblock *cfree;
struct	buf *buf, *swbuf;
short	*swsize;
int	*swpf;
char	*buffers;
struct	cmap *cmap, *ecmap;
#ifdef QUOTA
struct	quota *quota, *quotaNQUOTA;
struct	dquot *dquot, *dquotNDQUOT;
#endif

/* The following data structs are required for System V IPC */
char ipcmsgbuf[ MSGSEG * MSGSSZ];
struct map	msgmap[MSGMAP];
struct msqid_ds	msgque[MSGMNI];
struct msg	msgh[MSGTQL];
struct msginfo	msginfo = {
	MSGMAP,
	MSGMAX,
	MSGMNB,
	MSGMNI,
	MSGSSZ,
	MSGTQL,
	0,
	MSGSEG
};

struct	semid_ds	sema[SEMMNI];
struct	sem		sem[SEMMNS];
struct	map		semmap[SEMMAP];
struct	sem_undo	*sem_undo[NPROC];

#define	SEMUSZ	(sizeof(struct sem_undo)+sizeof(struct undo)*SEMUME)
int	semu[((SEMUSZ*SEMMNU)+NBPW-1)/NBPW];

union {
	short		semvals[SEMMSL];
	struct semid_ds	ds;
	struct sembuf	semops[SEMOPM];
}	semtmp;

struct	seminfo seminfo = {
	SEMMAP,
	SEMMNI,
	SEMMNS,
	SEMMNU,
	SEMMSL,
	SEMOPM,
	SEMUME,
	SEMUSZ,
	SEMVMX,
	SEMAEM
};

struct smem	smem[SMMNI];	

/*
 * SMMAX, SMMIN and SMBRK are 512 byte pages.  SMMAX is rounded up to bytes
 * on a cluster boundary.  SMMIN is truncated down to bytes on a cluster
 * boundary.  SMBRK is rounded up to a cluster boundary.
 */
#ifndef SMMAX
#define	SMMAX	256	   /* Max SM segment size (512 byte pages) */
#endif  SMMAX

#ifndef SMMIN
#define	SMMIN	0	   /* Min SM segment size (512 byte pages) */
#endif  SMMIN

#ifndef SMBRK
#define	SMBRK	64	   /* separation between end of data and
			      beginning of SM segments (512 byte pages) */
#endif  SMBRK

struct	sminfo sminfo = {
	clrnd(SMMAX) * NBPG,
	(SMMIN * NBPG) & ~(CLBYTES - 1),
	SMMNI,
	SMSEG,
	clrnd(SMBRK),
};

struct cpudata	cpudata[NCPU];

int maxcpu = NCPU;
int activecpu = 0;
int extracpu = NCPU-1;
int	mincurpri;	/* current minimum running priority */
int	minpricpu=0;	/* cpu that min priority task is running on */
int	runrun = 0;
char	*istack;	/* interrupt stacks for slave cpus */


int lockword = (1 << LOCK_ACTV);
int lastlock;

/*
 * The next three variables are forced to 1 for a SAS kernel, to block
 * paging.  This overrides default paging limits.
 */
#ifdef SAS
int desfree = 1;
int lotsfree = 1;
int minfree = 1;
#else SAS
int desfree = 0;
int lotsfree = 0;
int minfree = 0;
#endif SAS
