#ifndef lint
static char *sccsid = "@(#)init_main.c	1.21	ULTRIX	12/16/86";
#endif

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/************************************************************************
 *
 *			Modification History
 *
 * 15-Dec-86 -- depp
 *	Fixed problem in swapconf() that caused crash when a MASSBUS
 *	disk is booted.
 *
 * 11-Sep-86 - koehler
 *	moved the mounting of the root filesystem to gfs_data.
 *
 * 16-Apr-89 - ricky palmer
 *
 *	Replaced root device "case" code with devioctl code.
 *
 * 15-Apr-86 -- jf
 *	call system process initialization routine
 *
 * 12-Mar-86 -- bjg
 *	dumplo now decreased by one byte; after msgbuf removed, not
 *	enough space for dumps, so decremented dumplo by 1.
 *
 *
 * 18-Feb-86 -- jrs
 *	Move unlock that releases slaves down a little to be extra safe
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 *	03-Oct-85	Stephen
 * 004- Dumplo will now be calculated on the physical memory size and not on
 *	a predefined constant.
 *
 *	09-Sept-85	Stephen Reilly
 * 003- Modified to handle the new lockf code.
 *
 *	01-Jul-85	Stephen Reilly
 * 002- Dump's size was being based on the first entry of the
 *	swap table.  It is now fixed so that the size of the dump
 *	partition will be based on the dump device itself.
 *
 * 24 Oct 84 -- jrs
 *	Add initialization for nami cacheing and linked proc list
 *	Derived from 4.2BSD, labeled:
 *		init_main.c 6.3 84/05/22
 *
 *	01-Nov-84	Stephen Reilly
 * 001- The size of the swap device is now done here rather than in
 *	autoconf.c.  The reason is becuase of the new disk partitioning
 *	scheme.
 *	init_main.c	6.1	83/07/29
 *
 *	22 Feb 85	Greg Depp
 *	Add in calls to msginit() and seminit() for System V IPC
 *
 ***********************************************************************/
#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kernel.h"
#include "../h/mount.h"
#include "../h/map.h"
#include "../h/proc.h"
#include "../h/gnode.h"
#include "../h/fs_types.h"
#include "../h/seg.h"
#include "../h/conf.h"
#include "../h/buf.h"
#include "../h/vm.h"
#include "../h/cmap.h"
#include "../h/text.h"
#include "../h/clist.h"
#ifdef INET
#include "../h/protosw.h"
#endif
#include "../h/quota.h"
#include "../h/interlock.h"
#include "../h/kmalloc.h"
#include "../machine/reg.h"
#include "../machine/cpu.h"
#include "../machine/vmparam.h"

extern	struct user u;		/* have to declare it somewhere! */
extern	gno_t rootino;		/* XXX look in ufs_mount.c */
/*
 * Initialization code.
 * Called from cold start routine as
 * soon as a stack and segmentation
 * have been established.
 * Functions:
 *	clear and free user core
 *	turn on clock
 *	hand craft 0th process
 *	call all initialization routines
 *	fork - process 0 to schedule
 *	     - process 2 to page out
 *	     - process 1 execute bootstrap
 *
 * loop at loc 13 (0xd) in user mode -- /etc/init
 *	cannot be executed.
 */

int maxdsiz = MAXDSIZ;
int maxssiz = MAXSSIZ;

main(firstaddr)
	int firstaddr;
{
	register int i, m;
	register struct proc *p;
	register struct mount *mp;
	register int s;

	if (dmmin <= 0) {
		dmmin = 32;
	}
	if (dmmax <= 0) {
		dmmax = 1024;
	}
	for (i=0, m=dmmin; m<dmmax; i++) /* calculate maxssiz */
		m *= 2;
	maxdsiz = maxssiz = (NDMAP - i) * dmmax - dmmin - SLOP;
	if (maxdsiz <= 0)
		maxdsiz = MAXDSIZ;
	if (maxssiz <= 0)
		maxssiz = MAXSSIZ;

	rqinit();
#include "loop.h"
	startup(firstaddr);

	/*
	 * set up system process 0 (swapper)
	 */
	p = &proc[0];
	p->p_p0br = u.u_pcb.pcb_p0br;
	p->p_szpt = 1;
	p->p_addr = uaddr(p);
	p->p_stat = SRUN;
	p->p_flag |= SLOAD|SSYS;
	p->p_nice = NZERO;
	setredzone(p->p_addr, (caddr_t)&u);
	u.u_procp = p;
	u.u_cmask = CMASK;

	u.u_cred = crget();

	for (i = 1; i < NGROUPS; i++)
		u.u_groups[i] = NOGROUP;
	for (i = 0; i < sizeof(u.u_rlimit)/sizeof(u.u_rlimit[0]); i++)
		u.u_rlimit[i].rlim_cur = u.u_rlimit[i].rlim_max =
		    RLIM_INFINITY;
	u.u_rlimit[RLIMIT_STACK].rlim_cur = 512*1024;
	u.u_rlimit[RLIMIT_STACK].rlim_max = ctob(maxssiz);
	u.u_rlimit[RLIMIT_DATA].rlim_max =
	    u.u_rlimit[RLIMIT_DATA].rlim_cur = ctob(maxdsiz);
	p->p_maxrss = RLIM_INFINITY/NBPG;
#ifdef QUOTA
	qtinit();
	p->p_quota = u.u_quota = getquota(0, 0, Q_NDQ);
#endif
	startrtclock();
#include "kg.h"
#if NKG > 0
	startkgclock();
#endif

	/*
	 * Initialize tables, protocols, and set up well-known inodes.
	 */
	mbinit();
	cinit();			/* needed by dmc-11 driver */
#ifdef INET
#if NLOOP > 0
	loattach();			/* XXX */
#endif
	/*
	 * Block reception of incoming packets
	 * until protocols have been initialized.
	 */
	s = splimp();
	ifinit();
#endif
	domaininit();
#ifdef INET
	splx(s);
#endif
	pqinit();
	ghinit();
	bhinit();
	binit();
	textinit();
	bswinit();
	nchinit();
	flckinit();			/* 003 */
#ifdef GPROF
	kmstartup();
#endif
	msginit();
	seminit();

	/*
	 * Initialize system processes.
	 */
	sysprocinit();

	bzero(mount, sizeof(struct mount) * NMOUNT);
	init_fs();
	mp = &mount[0];
	mp->m_fs_data = (struct fs_data *)
			km_alloc(sizeof(struct fs_data),KM_CLRSG);
	mount_root(mp);
	nmount++;
	mp->m_gnodp = (struct gnode *) NULL;
	mp->m_flags |= M_DONE;
	
	boottime = time;

/* kick off timeout driven events by calling first time */
	roundrobin();
	schedcpu();
	schedpaging();

/* set up the root file system */
	rootdir = GGET(rootdev, mp, rootino, 0);
	if(rootdir == NULL)
		panic("init_main: rootdir == NULL");
	gfs_unlock(rootdir);
	u.u_cdir = GGET(rootdev, mp, rootino, 0);
	if(u.u_cdir == NULL)
		panic("init_main: cdir == NULL");

	gfs_unlock(u.u_cdir);
	u.u_rdir = NULL;

	u.u_dmap = zdmap;
	u.u_smap = zdmap;

	/*
	 * Set the scan rate and other parameters of the paging subsystem.
	 */
	setupclock();

	/*
	 * make page-out daemon (process 2)
	 * the daemon has ctopt(nswbuf*CLSIZE*KLMAX) pages of page
	 * table so that it can map dirty pages into
	 * its address space during asychronous pushes.
	 */
	mpid = 1;
	proc[0].p_szpt = clrnd(ctopt(nswbuf*CLSIZE*KLMAX + UPAGES));
	proc[1].p_stat = SZOMB; 	/* force it to be in proc slot 2 */
	p = freeproc;
	freeproc = p->p_nxt;
	if (newproc(0)) {
		proc[2].p_flag |= SLOAD|SSYS;
		proc[2].p_dsize = u.u_dsize = nswbuf*CLSIZE*KLMAX;
		pageout();
		/*NOTREACHED*/
	}

	/*
	 * make init process and
	 * enter scheduling loop
	 */

	mpid = 0;
	proc[1].p_stat = 0;
	proc[0].p_szpt = CLSIZE;
	p->p_nxt = freeproc;
	freeproc = p;
	if (newproc(0)) {
		expand(clrnd((int)btoc(szicode)), 0);
		(void) swpexpand(u.u_dsize, 0, &u.u_dmap, &u.u_smap);
		(void) copyout((caddr_t)icode, (caddr_t)0, (unsigned)szicode);
		/*
		 * Return goes to loc. 0 of user init
		 * code just copied out.
		 */
		return;
	}
	mpid = 2;
	proc[0].p_szpt = 1;
	unlock(LOCK_ACTV);
	sched();
}

/*
 * Initialize hash links for buffers.
 */
bhinit()
{
	register int i;
	register struct bufhd *bp;

	for (bp = bufhash, i = 0; i < BUFHSZ; i++, bp++)
		bp->b_forw = bp->b_back = (struct buf *)bp;
}

/*
 #define	DMMIN	32
 #define	DMMAX	1024
*/
#define DMTEXT	1024
int binit_flag; 			/* 001 Indicates that we have init the
					 *     buffer headers.
					 */
/*
 * Initialize the buffer I/O system by freeing
 * all buffers and setting all device buffer lists to empty.
 */
binit()
{
	register struct buf *bp, *dp;
	register int i;
	int base, residual;

	for (dp = bfreelist; dp < &bfreelist[BQUEUES]; dp++) {
		dp->b_forw = dp->b_back = dp->av_forw = dp->av_back = dp;
		dp->b_flags = B_HEAD;
	}
	base = bufpages / nbuf;
	residual = bufpages % nbuf;
	for (i = 0; i < nbuf; i++) {
		bp = &buf[i];
		bp->b_dev = NODEV;
		bp->b_bcount = 0;
		bp->b_un.b_addr = buffers + i * MAXBSIZE;
		if (i < residual)
			bp->b_bufsize = (base + 1) * CLBYTES;
		else
			bp->b_bufsize = base * CLBYTES;
		binshash(bp, &bfreelist[BQ_AGE]);
		bp->b_flags = B_BUSY|B_INVAL;
		brelse(bp);
	}
	/*
	 *	Indicate that we have finished with the initing the buffer
	 *	header's and are ready to determine the swap devices.
	 */
	binit_flag = -1;
	swapconf();
}

/*
 * Initialize linked list of free swap
 * headers. These do not actually point
 * to buffers, but rather to pages that
 * are being swapped in and out.
 */
bswinit()
{
	register int i;
	register struct buf *sp = swbuf;

	bswlist.av_forw = sp;
	for (i=0; i<nswbuf-1; i++, sp++)
		sp->av_forw = sp+1;
	sp->av_forw = NULL;
}

/*
 * Initialize clist by freeing all character blocks, then count
 * number of character devices. (Once-only routine)
 */
cinit()
{
	register int ccp;
	register struct cblock *cp;

	ccp = (int)cfree;
	ccp = (ccp+CROUND) & ~CROUND;
	for(cp=(struct cblock *)ccp; cp < &cfree[nclist-1]; cp++) {
		cp->c_next = cfreelist;
		cfreelist = cp;
		cfreecount += CBSIZE;
	}
}

/*
 *	This routine is a combination of both autoconf.c and binit
 *	code.  The reason for this was with the new disk partitioning
 *	scheme the swap size can not be determined until the buffer
 *	headers have been inittialized.
 */
int swapconf_flag = 0;
swapconf()
{
	register int nblks;				/* 001 */
	register struct swdevt *swp;			/* 001 */


	/*
	 *	Don't try to determine swap sizes until we have finished
	 *	with binit.
	 */
	if(!binit_flag)
		return;

	/*
	 *			001
	 *	Most of the rest of the routine was taken from autoconf.c.
	 *	The reason was that to get the correct sizing of a device
	 *	with the new partitioning scheme, it requires the device to be
	 *	open. This is the first time we can do this because of
	 *	buf structure being init. here.
	 */
	for (swp = swdevt; swp->sw_dev; swp++) {
		bdevsw[major(swp->sw_dev)].d_open(swp->sw_dev);

		if (bdevsw[major(swp->sw_dev)].d_psize) {
			nblks =
			  (*bdevsw[major(swp->sw_dev)].d_psize)(swp->sw_dev);
		}
		bdevsw[major(swp->sw_dev)].d_close(swp->sw_dev);

		if (swp->sw_nblks == 0 || swp->sw_nblks > nblks)
			swp->sw_nblks = nblks;
	}

	/*
	 *	If called by disk device, the ignore balance of the routine
 	 */
	if (swapconf_flag)
		return;
	swapconf_flag++;

	/*
	 *			002
	 *	Before we can determine the size of the dump device we
	 *	first open it.	This is now a requirement because the
	 *	partition tables are not setup until an open is
	 *	done on the drive.  This change is due to the new
	 *	disk partitioning scheme.
	 */
	bdevsw[major(dumpdev)].d_open(dumpdev);
	if (dumplo == 0)
		dumplo = (*bdevsw[major(dumpdev)].d_psize)(dumpdev)
				- (physmem*(NBPG/DEV_BSIZE)) - 1; /* 004 */
	/*
	 *	Don't forget to close it
	 */
	bdevsw[major(dumpdev)].d_close(dumpdev);

	if (dumplo < 0)
		dumplo = 0;
/*
	if (dmmin == 0)
		dmmin = DMMIN;
	if (dmmax == 0)
		dmmax = DMMAX;
*/
	if (dmtext == 0)
		dmtext = DMTEXT;
	if (dmtext > dmmax)
		dmtext = dmmax;


	/*
	 * Count swap devices, and adjust total swap space available.
	 * Some of this space will not be available until a vswapon()
	 * system is issued, usually when the system goes multi-user.
	 */
	if( nswdev == 0 ) {

		nswdev = 0;
		nswap = 0;
		for (swp = swdevt; swp->sw_dev; swp++) {
			nswdev++;
			if (swp->sw_nblks > nswap)
				nswap = swp->sw_nblks;
		}
		if (nswdev == 0)
			panic("binit");
		if (nswdev > 1)
			nswap = ((nswap + dmmax - 1) / dmmax) * dmmax;
		nswap *= nswdev;
		maxpgio *= nswdev;
		swfree(0);
	}
}
