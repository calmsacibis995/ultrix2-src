/* @(#)param.h	1.16	(ULTRIX)	9/11/86 */

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
 *			Modification History
 *						
 * 11 Sep 86 -- koehler
 *	added a constant so that everyone may always tell what the
 *	units returned are from getmnt 
 *
 * 29 Apr 86 -- depp
 *	Moved System IPC constants to either /sys/h: msg.h, sem.h
 *
 * 14 Aug 85 -- depp
 *	Removed SEMUND definition as this should be NPROC
 *
 * 03 May 85 -- Larry Cohen						*
 *	Increase size of CBSIZE to 60 from 28				*
 *									*
 * 25 Apr 85 -- depp
 *	Removed SHMEM ifdefs
 *
 * 24 Apr 85 -- depp
 *	Increased SMBRK (delta between end of data and beginning of 
 *	shared memory) to 32K (64 pages).
 *
 *	Stephen Reilly, 18-Apr-1985
 * 004- Increased the number mounted devices from 15 to 64.
 *
 * 	David L Ballenger, 16-Apr-1985
 * 003-	Remove definition of HZ.
 *
 *  9 Apr 85 -- depp
 *	Define System V IPC constants
 *
 *	David L Ballenger, 31-Mar-1985					*
 * 0002 Define HZ for System V routines.				*
 *									*
 *	001 - March 11 1985 - Larry Cohen				*
 *	     increase NOFILE from 20 to 64				*
 *									*
 ************************************************************************/

/*	param.h	6.1	83/07/29	*/

/*
 * Machine type dependent parameters.
 */
#ifdef KERNEL
#include "../machine/param.h"
#else
#include <machine/param.h>
#endif

#define	NPTEPG		(NBPG/(sizeof (struct pte)))

/*
 * Shared memory -- number of segments per process
 */

#define	SMSEG	6

/*
 * Machine-independent constants
 */
#define	NMOUNT	64		/* 004 number of mountable file systems */
#define	MSWAPX	64		/* 004 pseudo mount table index for swapdev */
#define NUM_FS  0xff		/* maximum number of file system types */
#define MINPGTHRESH 8192	/* size of file where we demand page it in */
				/* files > this size are demand paged */
				/* files < this size are entirely read in */
				/* settable at mount time */
				/* default set by each file system */

/*
 * MAXUPRC is now a global variable: maxuprc whose value is assigned 
 * in param.c
 */
#define	NOFILE	64		/* max open files per process */ /*LSC001*/
/* NOFILE MUST NOT BE >= 31; SEE pte.h */
#define	CANBSIZ	256		/* max size of typewriter line */
#define	NCARGS	10240		/* # characters in exec arglist */
#define	NGROUPS	32		/* max number groups */

#define	NOGROUP	-1		/* marker for empty group set member */

/*
 * Priorities
 */
#define PCATCH  0400
#define PMASK   0177
#define	PSWP	0
#define	PINOD	10
#define	PRIBIO	20
#define	PRIUBA	24
#define	PZERO	25
#define	PPIPE	26
#define	PWAIT	30
#define	PLOCK	35
#define	PSLEP	40
#define	PUSER	50

#define	NZERO	20

/*
 * Signals
 */
#ifdef KERNEL
#include "../h/signal.h"
#else
#include <signal.h>
#endif

#define	ISSIG(p) \
	((p)->p_sig && ((p)->p_flag&STRC || \
	 ((p)->p_sig &~ ((p)->p_sigignore | (p)->p_sigmask))) && issig())

/*
 * Fundamental constants of the implementation.
 */
#define	NBBY	8		/* number of bits in a byte */
#define	NBPW	sizeof(int)	/* number of bytes in an integer */

#define	NULL	0
#define	CMASK	0		/* default mask for file creation */
#define	NODEV	(dev_t)(-1)

/*
 * Clustering of hardware pages on machines with ridiculously small
 * page sizes is done here.  The paging subsystem deals with units of
 * CLSIZE pte's describing NBPG (from vm.h) pages each... BSIZE must
 * be CLSIZE*NBPG in the current implementation, that is the paging subsystem
 * deals with the same size blocks that the file system uses.
 *
 * NOTE: SSIZE, SINCR and UPAGES must be multiples of CLSIZE
 */
#define	CLBYTES		(CLSIZE*NBPG)
#define	CLOFSET		(CLSIZE*NBPG-1)	/* for clusters, like PGOFSET */
#define	claligned(x)	((((int)(x))&CLOFSET)==0)
#define	CLOFF		CLOFSET
#define	CLSHIFT		(PGSHIFT+CLSIZELOG2)

#if CLSIZE==1
#define	clbase(i)	(i)
#define	clrnd(i)	(i)
#else
/* give the base virtual address (first of CLSIZE) */
#define	clbase(i)	((i) &~ (CLSIZE-1))
/* round a number of clicks up to a whole cluster */
#define	clrnd(i)	(((i) + (CLSIZE-1)) &~ (CLSIZE-1))
#endif

#ifndef INTRLVE
/* macros replacing interleaving functions */
#define	dkblock(bp)	((bp)->b_blkno)
#define	dkunit(bp)	(minor((bp)->b_dev) >> 3)
#endif

#define	CBSIZE	60		/* number of chars in a clist block */
#define	CROUND	0x3F		/* clist rounding; sizeof(int *) + CBSIZE -1*/

#ifndef KERNEL
#include	<sys/types.h>
#else
#ifndef LOCORE
#include	"../h/types.h"
#endif
#endif

/*
 * File system parameters and macros.
 *
 * The file system is made out of blocks of at most MAXBSIZE units,
 * with smaller units (fragments) only in the last direct block.
 * MAXBSIZE primarily determines the size of buffers in the buffer
 * pool. It may be made larger without any effect on existing
 * file systems; however making it smaller make make some file
 * systems unmountable.
 *
 * Note that the blocked devices are assumed to have DEV_BSIZE
 * "sectors" and that fragments must be some multiple of this size.
 * Block devices are read in BLKDEV_IOSIZE units. This number must
 * be a power of two and in the range of
 *	DEV_BSIZE <= BLKDEV_IOSIZE <= MAXBSIZE
 * This size has no effect upon the file system, but is usually set
 * to the block size of the root file system, so as to maximize the
 * speed of ``fsck''.
 */
#define	MAXBSIZE	8192
#define	DEV_BSIZE	512
#define	DEV_BSHIFT	9		/* log2(DEV_BSIZE) */
#define BLKDEV_IOSIZE	2048
#define MAXFRAG 	8
#define FSDUNIT		1024		/* used by getmnt */

#define	btodb(bytes)	 		/* calculates (bytes / DEV_BSIZE) */ \
	((unsigned)(bytes) >> DEV_BSHIFT)
#define	dbtob(db)			/* calculates (db * DEV_BSIZE) */ \
	((unsigned)(db) << DEV_BSHIFT)

/*
 * Map a ``block device block'' to a file system block.
 * This should be device dependent, and will be after we
 * add an entry to cdevsw for that purpose.  For now though
 * just use DEV_BSIZE.
 */
#define	bdbtofsb(bn)	((bn) / (BLKDEV_IOSIZE/DEV_BSIZE))

/*
 * MAXPATHLEN defines the longest permissable path length
 * after expanding symbolic links. It is used to allocate
 * a temporary buffer from the buffer pool in which to do the
 * name expansion, hence should be a power of two, and must
 * be less than or equal to MAXBSIZE.
 * MAXSYMLINKS defines the maximum number of symbolic links
 * that may be expanded in a path name. It should be set high
 * enough to allow all legitimate uses, but halt infinite loops
 * reasonably quickly.
 */
#define MAXPATHLEN	1024
#define MAXSYMLINKS	8

/*
 * bit map related macros
 */
#define	setbit(a,i)	((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define	clrbit(a,i)	((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define	isset(a,i)	((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define	isclr(a,i)	(((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)

/*
 * Macros for fast min/max.
 */
#define	MIN(a,b) (((a)<(b))?(a):(b))
#define	MAX(a,b) (((a)>(b))?(a):(b))

/*
 * Macros for counting and rounding.
 */
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#define	roundup(x, y)	((((x)+((y)-1))/(y))*(y))

