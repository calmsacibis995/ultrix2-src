#ifndef lint
static char *sccsid = "@(#)vm_mem.c	1.20	ULTRIX	1/15/87";
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
/*
 *
 *   Modification history:
 *
 * 12 Jan 86 -- depp
 *	Added changes to 2 routines, memfree() and vcleanu().  Memfree()
 *	will now check to see if the "u" pages list (see 11 Mar 86 comment
 *	below) is getting too long, if so, the list is flushed before new
 *	pages are added to it.	Vclearu() now does a wakeup() if memory 
 *	has been low. 
 *
 * 15 Dec 86 -- depp
 *	Changed kmemall() so that if the resource map is exhaused and
 *	KM_SLEEP set, then the process will sleep (and cycle) on lbolt.
 *	This means that kmemall() is guaranteed to return successfully
 *	if KM_SLEEP is set.
 *
 * 11 Sep 86 -- koehler
 *	gnode name change and more informative printf
 *
 * 27 Aug 86 -- depp
 *	Moved common code in kmemall and memall into a new routine pfclear
 *
 * 29 Apr 86 -- depp
 *	converted to locking macros from calls routines.  Plus added 
 *	KM_CONTG option to kmemall() {see that routine for more information}.
 *
 *	mlock/munlock/mwait have been converted to macros MLOCK/MUNLOCK/MWAIT
 *	and are now defined in /sys/h/vmmac.h.
 *
 *
 * 11 Mar 86 -- depp
 *	Fixed stale kernel stack problem by having "vrelu" and "vrelpt"
 *	indicate to [v]memfree to place "u" pages on a temporary list, 
 *	to be cleared by a new routine "vcleanu" (called by pagein).
 *
 * 24 Feb 86 -- depp
 *	Added 6 new routines to this file:
 *		pfalloc/pffree		physical page allocator/deallocator
 *		kmemall/kmemfree	System virtual cluster alloc/dealloc
 *		km_alloc/km_free	System virtual block alloc/dealloc
 *	Also, to insure proper sequencing of memory requests, "vmemall" now
 *	raises the IPL whenever "freemem" is referenced.
 *
 * 13 Nov 85 -- depp
 *	Added "cm" parameter to distsmpte call.  This parameter indicates that
 *	the "pg_m" bit is to be cleared in the processes PTEs that are sharing
 *	a data segment.  This replaces the "pg_cm" definition of "pg_alloc"
 *	which could cause a conflict.
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 * 001 - March 11 1985 - Larry Cohen
 *     disable mapped in files so NOFILE can be larger than 32
 *
 *
 * 11 Mar 85 -- depp
 *	Added in System V shared memory support.
 *
 */

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/cmap.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/vm.h"
#include "../h/file.h"
#include "../h/gnode.h"
#include "../h/buf.h"
#include "../h/mount.h"
#include "../h/trace.h"
#include "../h/map.h"
#include "../h/kernel.h"
#include "../h/ipc.h"
#include "../h/shm.h"
#include "../h/types.h"
#include "../h/kmalloc.h"
#include "../machine/mtpr.h"
#include "../machine/psl.h"
extern struct smem smem[];

#ifdef GFSDEBUG
extern short GFS[];
#endif

#ifdef KM_STATS
struct km_stats km_stats;
#endif KM_STATS

/*
 * Allocate memory, and always succeed
 * by jolting page-out daemon
 * so as to obtain page frames.
 * To be used in conjunction with vmemfree().
 */
vmemall(pte, size, p, type)
	register struct pte *pte;
	register int size;
	register struct proc *p;
{
	register int m;
	register int s;

	if (size <= 0 || size > maxmem)
		panic("vmemall size");

	s = splimp();
	while (size > 0) {
		if (freemem < desfree)
			outofmem();
		while (freemem == 0) {
			sleep((caddr_t)&freemem, PSWP+2);
		}
		m = freemem;
		if (m > size) m = size;	/* m = min of freemem and size */
		(void) memall(pte, m, p, type);
		size -= m;
		pte += m;
	}
	if (freemem < desfree)
		outofmem();

	splx(s);
	/*
	 * Always succeeds, but return success for
	 * vgetu and vgetpt (e.g.) which call either
	 * memall or vmemall depending on context.
	 */
	return (1);
}

/*
 * Free valid and reclaimable page frames belonging to the
 * count pages starting at pte.  If a page is valid
 * or reclaimable and locked (but not a system page), then
 * we simply mark the page as c_gone and let the pageout
 * daemon free the page when it is through with it.
 * If a page is reclaimable, and already in the free list, then
 * we mark the page as c_gone, and (of course) don't free it.
 *
 * Determines the largest contiguous cluster of
 * valid pages and frees them in one call to memfree.
 */
vmemfree(pte, count)
	register struct pte *pte;
	register int count;
{
	register struct cmap *c;
	register struct pte *spte;
	register int j;
	int size, pcnt;
	register int flg = MF_DETACH;
#ifdef notdef /* 001 */
	int fileno;
#endif
#ifdef GFSDEBUG
	int lock = 0;
#endif

	/* Are we deallocating "u" pages or it's PTEs ? */
	if (count < 0) {
		flg = MF_UAREA;
		count = -count;
	}

	if (count % CLSIZE)
		panic("vmemfree");

	for (size = 0, pcnt = 0; count > 0; pte += CLSIZE, count -= CLSIZE) {
		if (pte->pg_fod == 0 && pte->pg_pfnum) {
			c = &cmap[pgtocm(pte->pg_pfnum)];
			if (c->c_lock && c->c_type != CSYS) {
				for (j = 0; j < CLSIZE; j++)
					*(int *)(pte+j) &= PG_PROT;
				c->c_gone = 1;
				pcnt += CLSIZE;			
				goto free;
			}
			if (c->c_free) {
				for (j = 0; j < CLSIZE; j++)
					*(int *)(pte+j) &= PG_PROT;
				if (c->c_type == CTEXT)
					distpte(&text[c->c_ndx],
						(int)c->c_page, pte);
				/* SHMEM */
				else if (c->c_type == CSMEM)
					distsmpte(&smem[c->c_ndx],
						(int)c->c_page, pte,
						PG_NOCLRM);
				c->c_gone = 1;
				goto free;
			}
			pcnt += CLSIZE;			
			if (size == 0)
				spte = pte;
			size += CLSIZE;
			continue;
		}
#ifdef notdef /* 001 */
		/* Don't do anything with mapped ptes */
		if (pte->pg_fod && pte->pg_v)
			goto free;
#endif
		if (pte->pg_fod) {
#ifdef notdef /* 001 */
			fileno = ((struct fpte *)pte)->pg_fileno;
			if (fileno < NOFILE)
				panic("vmemfree vread");
#endif notdef
			for (j = 0; j < CLSIZE; j++)
				*(int *)(pte+j) &= PG_PROT;
		}
free:
		if (size) {
			memfree(spte, size, flg);
			size = 0;
		}
	}
	if (size)
		memfree(spte, size, flg);
	return (pcnt);
}

/*
 * Unlink a page frame from the free list -
 *
 * Performed if the page being reclaimed
 * is in the free list.
 */
munlink(pf)
	unsigned pf;
{
	register int next, prev;

	next = cmap[pgtocm(pf)].c_next;
	prev = cmap[pgtocm(pf)].c_prev;
	cmap[prev].c_next = next;
	cmap[next].c_prev = prev;
	cmap[pgtocm(pf)].c_free = 0;
	if (freemem < minfree)
		outofmem();
	freemem -= CLSIZE;
}

/*
 *****************************************************************************
 *****************************************************************************
 *
 * Function:
 *
 *	pfclear -- clears CMAP entry (on the free list) of encumberances
 *		   so that it may be reallocated.
 *
 * Function description:
 *
 *	This function is used by two routines {memall() and kmemall()} to
 *	provide a common mechanism to clear CMAP entries prior to 
 *	reallocation. 
 *
 * Interface:
 *
 *	PFCLEAR(c);
 *	  struct cmap *c;	 CMAP entry to be cleared
 *
 * Return Value:
 *
 *	None
 *
 * Error Handling:
 *
 *	Panics only
 *
 * Panics:
 *
 *
 *	"pfclear: ecmap"
 *		The cmap entry should be on a hash list, but isn't.
 *
 *	"pfclear: mfind"
 *		Mfind indicates (by non-0 return) that this cmap entry has
 *		not been unhashed.
 *	
 *****************************************************************************
 *****************************************************************************
 */

pfclear(c) 
register struct cmap *c;
{
	register struct cmap *c1, *c2;
	register int j;
	register int index = c->c_ndx;
	register struct proc *rp;
	struct pte *rpte;
	struct gnode *gp;

	/* If reclaimable, then clear associated PTEs */
	if (c->c_gone == 0 && c->c_type != CSYS) {
		if (c->c_type == CTEXT)
			rp = text[index].x_caddr;
		else
			rp = &proc[index];
		if(c->c_type != CSMEM)
			while (rp->p_flag & SNOVM)
				rp = rp->p_xlink;
		switch (c->c_type) {
		case CTEXT:
			rpte = tptopte(rp, c->c_page);
			break;
		case CDATA:
			rpte = dptopte(rp, c->c_page);
			break;
		case CSMEM: /* SHMEM */
			rpte = smem[index].sm_ptaddr +
						c->c_page;
			break;
		case CSTACK:
			rpte = sptopte(rp, c->c_page);
			break;
		}
		zapcl(rpte, pg_pfnum) = 0;
		if (c->c_type == CTEXT)
			distpte(&text[index], (int)c->c_page,
							rpte);
		else if (c->c_type == CSMEM)
			distsmpte(&smem[index],
					(int)c->c_page, rpte,
					PG_NOCLRM);
	}

	/* If on CMAP hash lists; then remove using pseudo munhash() below */
	if (c->c_blkno) {
		j = CMHASH(c->c_blkno);
		c1 = &cmap[cmhash[j]];
		if (c1 == c)
			cmhash[j] = c1->c_hlink;
		else {
			for (;;) {
				if (c1 == ecmap)
					panic("pfclear ecmap");
				c2 = c1;
				c1 = &cmap[c2->c_hlink];
				if (c1 == c)
					break;
			}
			c2->c_hlink = c1->c_hlink;
		}
#ifdef GFSDEBUG	
		if(GFS[18])
			cprintf(
			  "pfclear: pseudo munhash bn %d dev %d	\n",
			c1->c_blkno, c1->c_mdev);
#endif
		switch(c->c_type) {
			case CTEXT:
				gp = text[index].x_gptr;
				break;
			case CSMEM:
				gp = NULL;
				break;
			default:
				gp = proc[index].p_textp->x_gptr;
		}
		if (mfind(c->c_mdev==MSWAPX?swapdev:mount[c->c_mdev].m_dev,
		      (daddr_t)c->c_blkno, gp)) {
			printf(" pfclear: mdev 0x%x blkno %d x_gptr 0x%x\n",
			  c->c_mdev== MSWAPX?swapdev:mount[c->c_mdev].m_dev,
			  c->c_blkno, gp);
			panic("pfclear: mfind");
		}
		c1->c_mdev = (u_char) NODEV;
		c1->c_blkno = 0;
		c1->c_hlink = 0;
	}
}


/*
 * Allocate memory -
 *
 * The free list appears as a doubly linked list
 * in the core map with cmap[0] serving as a header.
 */
memall(pte, size, p, type)
	register struct pte *pte;
	int size;
	struct proc *p;
{
	register struct cmap *c;
	register int i, j;
	register unsigned pf;
	register int next, curpos;
	int s;
	
	if (size % CLSIZE)
		panic("memall");
	s = splimp();

	/* Insure that enough free memory exists to make allocation */
	if (size > freemem) {
		splx(s);
		return (0);
	}
	trace(TR_MALL, size, u.u_procp->p_pid);

	/* Allocation loop: by page cluster */
	for (i = size; i > 0; i -= CLSIZE) {

		/* Retrieve next free entry from TOP of list */
		curpos = cmap[CMHEAD].c_next;
		c = &cmap[curpos];
		freemem -= CLSIZE;
		next = c->c_next;
		cmap[CMHEAD].c_next = next;
		cmap[next].c_prev = CMHEAD;
		if(c->c_free == 0)
			panic("dup mem alloc");

		if (cmtopg(curpos) > maxfree)
			panic("bad mem alloc");

		/*
		 * If reclaimable, then clear encumberances
		 */
		pfclear(c);


		/* 
		 * Initialize CMAP entry
		 */
		switch (type) {

		case CSYS:
			c->c_ndx = p->p_ndx;
			break;

		case CTEXT:
			c->c_page = vtotp(p, ptetov(p, pte));
			c->c_ndx = p->p_textp - &text[0];
			break;

		case CDATA:
			c->c_page = vtodp(p, ptetov(p, pte));
			c->c_ndx = p->p_ndx;
			break;

		case CSMEM: /* SHMEM */
			c->c_page = pte - ((struct smem *)p)->sm_ptaddr;
			c->c_ndx = (struct smem *)p - &smem[0];
			break;

		case CSTACK:
			c->c_page = vtosp(p, ptetov(p, pte));
			c->c_ndx = p->p_ndx;
			break;
		}
		
		pf = cmtopg(curpos);
		for (j = 0; j < CLSIZE; j++)
			*(int *)pte++ = pf++;
		c->c_free = 0;
		c->c_gone = 0;
		if (c->c_intrans || c->c_want)
			panic("memall intrans|want");
		c->c_lock = 1;
		c->c_type = type;
	}
	splx(s);
	return (size);
}

/*
 * Free memory -
 *
 * The page frames being returned are inserted
 * to the head/tail of the free list depending
 * on whether there is any possible future use of them,
 * unless "flg" indicates that the page frames should be 
 * temporily stored on the "u" list until after the
 * context switch occurs.  In this case, the cmap entries
 * are deallocated as free, but place on the list {ucmap,eucmap}
 * until "vcleanu" is called to push them onto the free list.
 *
 * If the freemem count had been zero,
 * the processes sleeping for memory
 * are awakened.
 */

memfree(pte, size, flg)
	register struct pte *pte;
	register int size;
{
	register int i, j, prev, next;
	register struct cmap *c;
	int s;
	void vcleanu();

	if (size % CLSIZE)
		panic("memfree");
	if (freemem < CLSIZE * KLMAX)
		wakeup((caddr_t)&freemem);
	while (size > 0) {
		size -= CLSIZE;
		i = pte->pg_pfnum;
		if (i < firstfree || i > maxfree)
			panic("bad mem free");
		i = pgtocm(i);
		c = &cmap[i];
		if (c->c_free)
			panic("dup mem free");
		if (flg && c->c_type != CSYS) {
			for (j = 0; j < CLSIZE; j++)
				*(int *)(pte+j) &= PG_PROT;
			c->c_gone = 1;
		}
		s = splimp();

		/*
		 * If	deallocating "u" pages, place on temp "u" list 
		 *	to be cleared by "vcleanu" routine
		 * else place either at the head or tail of freelist
		 *	depending on whether it may be reclaimed
		 */
		if (flg == MF_UAREA) {
			/* insure that list doesn't get too long */
			if (nucmap >= UCLEAR)
				vcleanu();
			if (nucmap == 0)
				ucmap = eucmap = i;
			else {
				cmap[eucmap].c_next = i;
				c->c_prev = eucmap;
				eucmap = i;
			}
			nucmap++;
		} else if (flg == MF_DETACH && c->c_blkno == 0) {
			next = cmap[CMHEAD].c_next;
			cmap[next].c_prev = i;
			c->c_prev = CMHEAD;
			c->c_next = next;
			cmap[CMHEAD].c_next = i;
			freemem += CLSIZE;
		} else {
			prev = cmap[CMHEAD].c_prev;
			cmap[prev].c_next = i;
			c->c_next = CMHEAD;
			c->c_prev = prev;
			cmap[CMHEAD].c_prev = i;
			freemem += CLSIZE;
		}
		if(c->c_type == CTEXT) {
			struct text *xp;
			
			xp = &text[c->c_ndx];
			xp->x_cmap = c;
			xp->x_blkno = c->c_blkno;
			xp->x_dindex = c->c_mdev;
		}
		c->c_free = 1;
		splx(s);
		pte += CLSIZE;
	}
}

/*
 *****************************************************************************
 *****************************************************************************
 *
 * Function:
 *
 *	vcleanu -- kernel routine
 *
 * Function description:
 *
 *	This function will remove all of the page frames on the "u" list
 *	to the free list.  Since the cmap entries have been properly 
 *	initialized and linked in memall, all that must be done is to
 *	move the list intact onto the top of the free list.
 *
 *	NOTE:  This routine is currently only being called in "pagein".  
 *	If additional calls become necessary, this routine must only be
 *	called if "nucmap != 0" (see "panic").
 *
 * Return Value:
 *
 *	None
 *
 * Interface:
 *
 *	void vcleanu()
 *
 * Errors:
 *
 *	None
 *
 * Panics:
 *
 *	"vcleanu"
 *		This list is empty.  This routine should not have been called,
 *		which is one indication that the list may be corrupted.
 *	
 *****************************************************************************
 *****************************************************************************
 */

void
vcleanu() {
register s, next;

	s = splimp();
	if (ucmap == -1)
		panic("vcleanu");
	if (freemem < CLSIZE * KLMAX)
		wakeup((caddr_t)&freemem);
	next = cmap[CMHEAD].c_next;
	cmap[next].c_prev = eucmap;
	cmap[ucmap].c_prev = CMHEAD;
	cmap[eucmap].c_next = next;
	cmap[CMHEAD].c_next = ucmap;
	freemem += nucmap * CLSIZE;
	ucmap = eucmap = -1;
	nucmap = 0;
	splx(s);
}

/*
 * Allocate wired-down (non-paged) pages in kernel virtual memory.
 */
caddr_t
wmemall(pmemall, n)
	int (*pmemall)(), n;
{
	register int npg;
	register caddr_t va;
	register int a;

	npg = clrnd(btoc(n));
	a = rmalloc(kernelmap, (long)npg);
	if (a == 0)
		return (0);
	if ((*pmemall)(&Usrptmap[a], npg, &proc[0], CSYS) == 0) {
		rmfree(kernelmap, (long)npg, (long)a);
		return (0);
	}
	va = (caddr_t) kmxtob(a);
	vmaccess(&Usrptmap[a], va, npg);
	return (va);
}

/*
 * Allocate wired-down (non-paged) pages in kernel virtual memory.
 * (and clear them)
 */
caddr_t
zmemall(pmemall, n)
	int (*pmemall)(), n;
{
	register int npg;
	register caddr_t va;
	register int a;

	npg = clrnd(btoc(n));
	a = rmalloc(kernelmap, (long)npg);
	if (a == 0)
		return (0);
	if ((*pmemall)(&Usrptmap[a], npg, &proc[0], CSYS) == 0) {
		rmfree(kernelmap, (long)npg, (long)a);
		return (0);
	}
	va = (caddr_t) kmxtob(a);
	vmaccess(&Usrptmap[a], va, npg);
	while (--npg >= 0)
		clearseg((unsigned)(PG_PFNUM & *(int *)&Usrptmap[a++]));
	return (va);
}

wmemfree(va, n)
	caddr_t va;
	int n;
{
	register int a, npg;

	a = btokmx((struct pte *) va);
	npg = clrnd(btoc(n));
		(void) memfree(&Usrptmap[a], npg, MF_NODETACH);
	rmfree(kernelmap, (long)npg, (long)a);
}


/*
 *****************************************************************************
 *****************************************************************************
 *
 * Function:
 *
 *	pfalloc -- kernel routine
 *
 * Function description:
 *
 *	This function will allocate page frame clusters.  These
 *	physical pages are not coupled in any manner to system page table
 *	space.  It's up to the requesting routine to allocate and map the
 *	returned PFNs into the system page table.
 *
 *	As a rule, this routine should only be used to allocate single
 *	page frame clusters (npfc == 1).  It should only be used for 
 *	multiple clusters at startup/configuration time, as the free
 *	list will get out of order, and the overhead will become unbearable.
 *
 * Interface:
 *
 *	unsigned int pfalloc (type, npfc)
 *	  int type;		type must = CSYS ( ../h/cmap.h)
 *	  int npfc;		number of page frame clusters requested
 *				(normally, should == 1)
 *
 * Return Value:
 *
 *	= 0	Error -- no memory to allocate
 *	> 0	Normal -- PFN of first physical page allocated in
 *
 *	In a normal return, the first PFN allocated is returned, the balance
 *	of the PFNs in the block are easily determined since they are 
 *	contiguous.
 *
 * Error Handling:
 *
 *	If return value = 0, then either:
 *		1. the "cmap" free list is empty.
 *		2. a block of size "npgf" contiguous page frames could not
 *		   be located.
 *
 * Panics:
 *
 *	"pfalloc: type"
 *		Type must equal CSYS.  User types are not permitted.  In the
 *		future, other system types may exist and be permitted to use
 *		this routine.
 *
 *	"pfalloc: dup mem alloc"
 *		The cmap entry was on the free list, but the c_free flag
 *		indicates that the entry is NOT free.  This indicates 
 *		corruption of the free list.
 *
 *	"pfalloc: bad mem alloc"
 *		The index into the cmap structures is too high for this 
 *		memory configuration.  This indicates that the index is
 *		corrupted, and memory can not be reliably allocated.
 *
 *	"pfalloc: intrans|want"
 *		At this point, the physical cluster should be free of 
 *		encumbrances, but the cmap entry indicates that the
 *		physical cluster is intransient or wanted by an user proc.
 *
 *	
 *****************************************************************************
 *****************************************************************************
 */

unsigned
pfalloc (type, npfc)
int type;	/* cluster type */
int npfc;	/* number of page frame clusters */
{
	register struct cmap *c;
	register next, curpos, count;
	int end, head;
	unsigned pf;
	int s;

	if (type != CSYS)
		panic("pfalloc: type");
	s = splimp();
	if (freemem < npfc * CLSIZE) {
		splx(s);
		return(0);
	}

	/*
	 * if only one cluster requested, bypass contiguous lookup
	 * else look for contiguous page frames to pass back
	 */
	if (npfc == 1) {
		head = CMHEAD;
		pf = cmtopg(cmap[CMHEAD].c_next);
	}
	else {
		next = CMHEAD;
		end = cmap[CMHEAD].c_prev;
		count = npfc;

		while ((curpos = next = cmap[next].c_next) != end) {
			while (--count && (cmap[next].c_next == next + 1))
				next++;
			if (count == 0) {
				head = cmap[curpos].c_prev;
				pf = cmtopg(curpos);
				goto allocate;
			}
			count = npfc;
		}
		return(0);	/* sorry, not a large enough block */

	}
allocate:	
	while (npfc--) {
		curpos = cmap[head].c_next;
		c = &cmap[curpos];
		freemem -= CLSIZE;
		next = c->c_next;
		cmap[head].c_next = next;
		cmap[next].c_prev = head;
		if (c->c_free == 0)
			panic("pfalloc: dup mem alloc");
		if (cmtopg(curpos) > maxfree)
			panic("pfalloc: bad mem alloc");
		
		/* clear CMAP entry */
		pfclear(c);

		/* intialize CMAP entry */
		c->c_ndx = 0;
		c->c_free = 0;
		c->c_gone = 0;
		if (c->c_intrans || c->c_want)
			panic("pfalloc: intrans|want");
		c->c_type = type;
	}
#ifdef KM_STATS
	km_stats.tot_pfalloc++;
#endif KM_STATS
	splx(s);
	return (pf);
}


/*
 *****************************************************************************
 *****************************************************************************
 *
 * Function:
 *
 *	pffree -- kernel routine
 *
 * Function description:
 *
 *	This routine will free physical page clusters by placing them on the
 *	"cmap" free list.
 *
 * Interface:
 *
 *	void pffree (pfn, npfc)
 *	  unsigned int pfn		First PFN in cluster to be deallocated
 *	  int npfc			must equal 1 (for now)
 *
 * Return Value:
 *
 *	None
 *
 * Error Handling:
 *
 *	None
 *
 * Panics:
 *
 *	"pffree: bad mem free"
 *		The PFN is outside the valid range for this memory 
 *		configuration or npfc not = 1
 *
 *	"pffree: dup mem free"
 *		The cmap entry is marked as being free.  This indicates that
 *		either the free list is corrupted or that the input PFN has
 *		been currupted.
 *	
 *****************************************************************************
 *****************************************************************************
 */

void
pffree (pfn, npfc)
unsigned pfn;
int npfc;
{
	register int next;
	register struct cmap *c;
	register int s, i;
	
	if (pfn < firstfree || pfn > maxfree || npfc != 1)
		panic("pffree: bad mem free");
	i = pgtocm(pfn);
	c = &cmap[i];
	if (c->c_free)
		panic("pffree: dup mem free");
	if (freemem < CLSIZE * KLMAX)
		wakeup((caddr_t)&freemem);
	s = splimp();
	next = cmap[CMHEAD].c_next;
	cmap[next].c_prev = i;
	c->c_prev = CMHEAD;
	c->c_next = next;
	cmap[CMHEAD].c_next = i;
	c->c_free = 1;
	freemem += CLSIZE;
#ifdef KM_STATS
	km_stats.tot_pffree++;
#endif KM_STATS
	splx(s);
}


/*
 *****************************************************************************
 *****************************************************************************
 *
 * Function:
 *
 *	kmemall -- kernel routine
 *
 * Function description:
 *
 *	This function allocates blocks of system page clusters.  It differs
 *	from zmemall and wmemall in that a block of system page table space
 *	is dedicated to this mechanism, rather than using the system page
 *	table space reserved for user page tables.  Also, this mechanism 
 *	permits some options.  
 *
 *	UNLESS PAGE OR CLUSTER ALIGNMENT IS REQUIRED, OR CONTIGUOUS PHYSICAL
 *	MEMORY REQUIRED, KM_ALLOC SHOULD BE USED!
 *
 * Return Value:
 *
 *	> 0	Normal -- System address of the beginning of the alloc'd block
 *	= 0	Error 
 *
 * Interface:
 *
 *	caddr_t kmemall (nbytes, zflg)
 *	  int nbytes;		number of bytes requested (will be rounded up
 *				  to cluster boundary
 *	  int options;		Flags to indicate
 *				  KM_CLRSG	zero fill block
 *				  KM_SLEEP	sleep if no memory
 *				  KM_CONTG	contiguous physical memory
 *						required (do not use with
 *						KM_SLEEP)
 *
 *
 * Errors:
 *
 *	If the KM_SLEEP option HAS NOT been set, then
 *	a zero will be returned under the follow circumstances:
 *
 *		1. rmalloc () indicates that the map (dmemmap) is exhausted.
 *		2. the cmap free list is exhausted.
 *		3. if KM_CONTG is set, pfalloc() indicates that a large
 *		   enough contiguous block is not available
 *
 *	If the KM_SLEEP option HAS been set, then a successful return is
 *	guaranteed, because the currently running process will be put to
 *	sleep to wait for memory.
 *
 * Panics:
 *
 *	"kmemall: Invalid Mode"
 *		The "options" parameter was set to KM_SLEEP when execution
 *		was on the interrupt stack.
 *
 *	"kmemall: pfalloc"
 *		Pfalloc did not return a page frame number, indicating that
 *		there is no memory.  This shouldn't happen since memory
 *		checks were made prior to the call to pfalloc.
 *
 *
 * Notes:
 *
 *	1. The KM_CONTG option becomes VERY EXPENSIVE after the system has
 *	   begun to run processes, so it should only be used during
 *	   system startup and configuration.
 *
 *	2. If KM_SLEEP and KM_CONTG are set, KM_SLEEP is ignored.
 *	
 *****************************************************************************
 *****************************************************************************
 */


#ifdef KM_DEBUG
struct kmemdebug {
	caddr_t va;
	int npg;
} kmemdebug[KM_DEBUG_S];
#endif KM_DEBUG

caddr_t
kmemall(nbytes,options)
	int nbytes;	/* number of bytes requested */
	int options;	/* options flag */
{
	register int i, j;
	register struct pte *pte;	/* pointer into dmemptmap */
	register int npg;		/* number of pages */
	register int a;			/* index into dmemptmap */
	register unsigned pfn;		/* Page frame number being alloc'ed */
	int s;				/* current IPL */
	caddr_t beg_va;			/* pointer to beginning of block */
	int clrsg = options & KM_CLRSG;
	int proc_sleep = options & KM_SLEEP;
	void kmemfree ();
	extern int lbolt;

	/* sleep option is not valid on interrupt stack */
	if (proc_sleep && (movpsl() & PSL_IS))
		panic("kmemall: Invalid Mode");

	/*
 	 *  allocate system page table space
 	 */
	npg = clrnd(rbtop(nbytes));
	s = splimp();
	while ((a = rmalloc(dmemmap, (long)npg)) == 0) {
		if (!proc_sleep) {
			splx(s);
			return (NULL_PTR);
		}
		sleep((caddr_t)&lbolt, PSWP+2);
	}

	pte = &dmemptmap[a];
	beg_va = (caddr_t) dmxtob(a);

	/*
	 * If options indicate contiguous physical memory required,
 	 * then make a single call to pfalloc() to allocate,
	 * else use pfalloc() to allocate single page frame clusters
	 */
	if (options & KM_CONTG) {
		if ((pfn = pfalloc(CSYS,npg/CLSIZE)) == 0)
			goto bad;
		for (i = npg; i-- ; pte++, pfn++) {
			*(int *) pte = ((u_int)(PG_V|PG_KW) | pfn);
			if (clrsg)
				clearseg(pfn);
		}
	}
	else {
		/* check to insure that enough free memory is available */
		while (npg > freemem) {
			outofmem();
			if (!proc_sleep)
				goto bad;
			sleep((caddr_t)&freemem, PSWP+2);
		}

		/*
		 * Allocate physical pages, map PTES, and clear pages if
		 * requested.  At this point, there should be memory available
		 */
		for (i = 0; i < npg; i += CLSIZE, pte += CLSIZE) {
			if ((pfn = pfalloc(CSYS,1)) == 0) 
				panic("kmemall: pfalloc");
			for (j = 0; j < CLSIZE; j++, pfn++) {
			      *(int *)(pte + j) = ((u_int)(PG_V|PG_KW) | pfn);
			      if (clrsg)
					clearseg(pfn);
			}
		}
	}

#ifdef KM_DEBUG
	INS_KMEMDEBUG(beg_va,npg);
#endif KM_DEBUG
#ifdef KM_STATS
	km_stats.tot_kmemall++;
#endif KM_STATS
	splx(s);
	mtpr(TBIA,0);
	tbsync();
	return (beg_va);
bad:
	rmfree(dmemmap, (long)npg, (long)a);
	splx(s);
	return (NULL_PTR);
}


/*
 *****************************************************************************
 *****************************************************************************
 *
 * Function:
 *
 *	kmemfree -- kernel routine
 *
 * Function description:
 *
 *	This function will deallocate any memory block that was allocated
 *	via "kmemall".
 *
 * Interface:
 *
 *	void kmemfree (va, nbytes)
 *	  caddr_t va		Virtual address of beginning of memory block
 *	  int nbytes		number of bytes requested (rounded up to 
 *					page cluster boundary)
 *
 * Return Value:
 *
 *	None
 *
 * Error Handling:
 *
 *	None other than "panics"
 *
 * Panics:
 *
 *	"kmemfree: Invalid PTE"
 *		Indicates that during deallocation, the block contained
 *		invalid memory.
 *
 *	
 *****************************************************************************
 *****************************************************************************
 */

void
kmemfree(va, nbytes)
	caddr_t va;
	int nbytes;
{
	register int i, j;
	register int npg;	/* number of pages rounded to CLSIZE bound */
	register int a;		/* index into dmemptmap */
	struct pte *pte;	/* pointer to system page table space */
	int s;

	/*
	 * find first PTE in block and calculate number of NBPG pages
	 */
	a = btodmx((struct pte *) va);
	pte = &dmemptmap[a];
	npg = clrnd(rbtop(nbytes));
#ifdef KM_DEBUG
	CHK_KMEMDEBUG(va,npg);
#endif KM_DEBUG

	/* deallocate valid pages, clearing associated PTES */
	for (i = 0; i < npg; i += CLSIZE, pte += CLSIZE) {
		if (pte->pg_v == 0)
			panic("kmemfree: Invalid PTE");
		pffree(pte->pg_pfnum,1);
		for (j = 0; j < CLSIZE; j++)
			*(int *) (pte +j) = 0;		
	}

	/* deallocate resource map */
	s = splimp();
	rmfree(dmemmap, (long)npg, (long)a);
#ifdef KM_STATS
	km_stats.tot_kmemfree++;
#endif KM_STATS
	splx(s);
	mtpr(TBIA,0);
	tbsync();
	return;
}



/*
 * Enter clist block c on the hash chains.
 * It contains file system block bn from device dev.
 * Dev must either be a mounted file system or the swap device
 */
mhash(c, dev, bn)
	register struct cmap *c;
	dev_t dev;
	daddr_t bn;
{
	register int i = CMHASH(bn);
	register struct mount *mp;
	

#ifdef GFSDEBUG 

	register struct cmap *c1 = &cmap[cmhash[CMHASH(bn)]];
	dev_t mdev;
	
	if(GFS[18]) {
		cprintf("mhash: bn %d dev 0x%x type %d c 0x%x index %d\n", bn,
		dev, c->c_type, c, c->c_ndx);
	}
	GETMP(mp, dev);
	mdev = (mp == (struct mount *) MSWAPX) ? MSWAPX : mp - mount;
#ifndef NFS	
	while(c1 != ecmap) {
		if((c1->c_blkno == bn) && (c1->c_mdev == mdev)) {
			printf("mhash:\tbn %d dev 0x%x\n", bn, dev);
			printf("\tc1 0x%x bn %d dev 0x%x type %d index %d\n",
			c1, c1->c_blkno, c1->c_mdev, c1->c_type, c1->c_ndx);
			printf("\tc 0x%x bn %d dev 0x%x type %d index %d\n",
			c, bn, mdev, c->c_type, c->c_ndx);
			panic("mhash ^&%& kernel");
		}
		c1 = &cmap[c1->c_hlink];
	}
#endif NFS
#endif GFSDEBUG
	c->c_hlink = cmhash[i];
	cmhash[i] = c - cmap;
	c->c_blkno = bn;
	GETMP(mp, dev);	
	if(mp == NULL)
		panic("mhash: no mp");
	c->c_mdev = (mp == (struct mount *) MSWAPX) ? MSWAPX : mp - mount;
}

/*
 * Pull the clist entry of <dev,bn> off the hash chains.
 * We have checked before calling (using mfind) that the
 * entry really needs to be unhashed, so panic if we can't
 * find it (can't happen).
 *
 * N.B. if dev == swapdev, gp is NULL since the block on the swap
 * device may not be associated with any active text or data segment
 */
munhash(dev, bn, gp)
	register dev_t dev;
	register daddr_t bn;
	register struct gnode *gp;
{
	register struct cmap *c1, *c2;
	register int index;
	register struct gnode *gpproto = NULL;
	int i = CMHASH(bn);
	struct mount *mp;
	int needgp = 1;
	int si = splimp();
#ifdef GFSDEBUG
	if (GFS[18]) {
		cprintf("munhash: dev %x bn %d gp (%d) %x\n",
					dev,bn,gp,gp?gp->g_number:-1);
	}
#endif GFSDEBUG
	
	c1 = &cmap[cmhash[i]];
	if (c1 == ecmap)
		panic("munhash");
	
	if (gp && gp->g_dev == dev) {	/* handle simple case to save */
		index = gp->g_mp - mount;
		mp = gp->g_mp;
	}
	else {
		GETMP(mp, dev)
		index = (mp == (struct mount *) MSWAPX) ? MSWAPX : mp - mount;
	}
	
	/*
	 * it is sufficient for the local case to use just dev and bn
	 * since locally we use lbn's rather than vbn's
	 */
	
	if((index == MSWAPX) || (mp->m_flags & M_LOCAL)) {
		gpproto = gp;	/* so matches work out */
		needgp = 0;	/* local doesn't need gp */
	}
	else  {
		switch(c1->c_type) {
			case CTEXT :
				gpproto = text[c1->c_ndx].x_gptr;
				break;
			case CSMEM :
				/*
				 * shared memory is not necessarily attached
				 * to any given process.  match on device and
				 * bn (as always)
				 */
				break;
			default :
				gpproto = proc[c1->c_ndx].p_textp->x_gptr;
		}
	}

#ifdef GFSDEBUG
#ifndef NFS
	if((c1->c_blkno == bn) && (index == c1->c_mdev) && 
	(gpproto != gp)) {
		cprintf("munhash: c1 bn %d index %d type %d\n", c1->c_blkno,
		c1->c_mdev, c1->c_type);
		cprintf("munhash: gp 0x%x (%d) gpproto 0x%x (%d)\n", gp,
		gp->g_number, gpproto, gpproto ? gpproto->g_number : -1);
	}
#endif
#endif
	if ((c1->c_blkno) == bn && (index == c1->c_mdev) && (gp == gpproto)) {
		cmhash[i] = c1->c_hlink;
	}
	else {
		for (;;) {
			c2 = c1;
			c1 = &cmap[c2->c_hlink];
			if (c1 == ecmap)
				panic("munhash: ecmap");
			if(needgp) {
				switch(c1->c_type) {
				case CTEXT :
					gpproto = text[c1->c_ndx].x_gptr;
					break;
				case CSMEM :
					gpproto = NULL;
					break;
				default :
					gpproto =
						proc[c1->c_ndx].p_textp->x_gptr;
				}
			}
#ifdef GFSDEBUG
#ifndef NFS
			if((c1->c_blkno == bn) && (index == c1->c_mdev) &&
					(gpproto != gp)) {
				cprintf("munhash: c1 bn %d index %d type %d\n",
				c1->c_blkno, c1->c_mdev, c1->c_type);
				cprintf("munhash: gp 0x%x (%d) gpproto 0x%x (%d)\n",
				gp, gp->g_number, gpproto, gpproto ?
				gpproto->g_number : -1);
			}
#endif
#endif
				
			if ((c1->c_blkno == bn) && (index == c1->c_mdev) &&
				(gp == gpproto)) {
				break;
			}
		}
		c2->c_hlink = c1->c_hlink;
	}
#ifdef GFSDEBUG
		if(GFS[18])
			cprintf("munhash: unhash bn %d dev %d gp 0x%x (%d)\n",
			bn, index, gp, gp ? gp->g_number : -1);
#endif	
	if (mfind(dev, bn, gp)) panic("munhash mfind");
	c1->c_mdev = (u_char) NODEV;
	c1->c_blkno = 0;
	c1->c_hlink = 0;
	splx(si);
}

/*
 * Look for block bn of device dev in the free pool.
 * Currently it should not be possible to find it unless it is
 * c_free and c_gone, although this may later not be true.
 * (This is because active texts are locked against file system
 * writes by the system.)
 *
 * N.B. if dev == swapdev, gp is NULL since the block on the swap
 * device may not be associated with any active text or data segment
 */
struct cmap *
mfind(dev, bn, gp)
	register dev_t dev;
	register daddr_t bn;
	register struct gnode *gp;
{
	register struct cmap *c1 = &cmap[cmhash[CMHASH(bn)]];
	register int index;
	register struct gnode *gpproto = NULL;
	struct mount *mp;
	int needgp = 1;
	int si = splimp();
	
#ifdef GFSDEBUG
	if(GFS[18])
		cprintf("mfind: dev 0x%x bn %d gp 0x%x (%d) proc 0x%x",
			dev, bn, gp, gp ? gp->g_number : -1, u.u_procp);
#endif GFSDEBUG

	if (gp && gp->g_dev == dev) {	/* handle simple case to save */
		if ((mp = gp->g_mp) == NULL)
			return((struct cmap *) 0);
		index = gp->g_mp - mount;
	}
	else {
		GETMP(mp, dev)
		if (mp == NULL)
			return((struct cmap *) 0);
		index = (mp == (struct mount *) MSWAPX) ? MSWAPX : mp - mount;
	}
	
	/*
	 * it is sufficient for the local case to use just dev and bn
	 * since locally we use lbn's rather than vbn's
	 * this includes the swap device
	 */
	if((index == MSWAPX) || (mp->m_flags & M_LOCAL)) {
		gpproto = gp;	/* so matches work out */
		needgp = 0;	/* local doesn't need the gp */
	}
	/* now search the core map */
	while (c1 != ecmap) {
		if(needgp) {
			switch(c1->c_type) {
				case CTEXT :
					gpproto = text[c1->c_ndx].x_gptr;
					break;
				case CSMEM :
					/*
					 * shared memory is not necessarily
					 * attached to any given process. 
					 * match on device and bn (as always)
					 */
					gpproto = NULL;
					break;
				default :
					gpproto=proc[c1->c_ndx].p_textp->x_gptr;
			}
		}

#ifdef GFSDEBUG 
#ifndef NFS
		if((bn == c1->c_blkno) && (index == c1->c_mdev) &&
		(gp != gpproto)) {
			cprintf("mfind: c1 0x%x bn %d dev 0x%x   bn %d index 0x%x\n",
			c1, c1->c_blkno, c1->c_mdev, bn, c1->c_ndx);
			cprintf("mfind: MISMATCH gpproto 0x%x gp 0x%x\n",
			gpproto, gp);
			panic("#$%$^&^$%## vm");
		}
#endif NFS
#endif GFSDEBUG
		if((c1->c_blkno == bn) && (c1->c_mdev == index) &&
		(gp == gpproto)) {
			splx(si);
#ifdef GFSDEBUG
			if(GFS[18]) {
				cprintf("MATCHED gpproto 0x%x (%d)\n",
				gpproto, gpproto ? gpproto->g_number : -1);
				cprintf("mfind: c 0x%x index %x\n", c1, 
				c1->c_ndx);
			}
#endif GFSDEBUG
			return (c1);
		}
		c1 = &cmap[c1->c_hlink];
	}
	splx(si);
#ifdef GFSDEBUG
	if(GFS[18])
		cprintf(" NO MATCH\n");
#endif
	return ((struct cmap *)0);
}

/*
 * Purge blocks from device dev from incore cache
 * before umount().
 */
mpurge(mdev)
	int mdev;
{
	register struct cmap *c1, *c2;
	register int i;
	int si = splimp();

	for (i = 0; i < CMHSIZ; i++) {
more:
		c1 = &cmap[cmhash[i]];
		if (c1 == ecmap)
			continue;
		if (c1->c_mdev == mdev)
			cmhash[i] = c1->c_hlink;
		else {
			for (;;) {
				c2 = c1;
				c1 = &cmap[c1->c_hlink];
				if (c1 == ecmap)
					goto cont;
				if (c1->c_mdev == mdev)
					break;
			}
			c2->c_hlink = c1->c_hlink;
		}
		c1->c_mdev = (u_char) NODEV;
		c1->c_blkno = 0;
		c1->c_hlink = 0;
		goto more;
cont:
		;
	}
	splx(si);
}

/*
 * Initialize core map
 */
meminit(first, last)
	int first, last;
{
	register int i;
	register struct cmap *c;

	firstfree = clrnd(first);
	maxfree = clrnd(last - (CLSIZE - 1));
	freemem = maxfree - firstfree;
	ecmx = ecmap - cmap;
	if (ecmx < freemem / CLSIZE)
		freemem = ecmx * CLSIZE;
	for (i = 1; i <= freemem / CLSIZE; i++) {
		cmap[i-1].c_next = i;
		c = &cmap[i];
		c->c_prev = i-1;
		c->c_free = 1;
		c->c_gone = 1;
		c->c_type = CSYS;
		c->c_mdev = (u_char) NODEV;
		c->c_blkno = 0;
	}
	cmap[freemem / CLSIZE].c_next = CMHEAD;
	for (i = 0; i < CMHSIZ; i++)
		cmhash[i] = ecmx;
	cmap[CMHEAD].c_prev = freemem / CLSIZE;
	cmap[CMHEAD].c_type = CSYS;
	avefree = freemem;
	hand = 0;
	ucmap = eucmap = -1;
	nucmap = 0;
}

/* 
 * Lock a virtual segment.
 *
 * For each cluster of pages, if the cluster is not valid,
 * touch it to fault it in, otherwise just lock page frame.
 * Called from physio to ensure that the pages 
 * participating in raw i/o are valid and locked.
 */
vslock(base, count)
	caddr_t base;
{
	register unsigned v;
	register int npf;
	register struct pte *pte;
	register struct cmap *c;

	v = btop(base);
	pte = vtopte(u.u_procp, v);
	npf = btoc(count + ((int)base & CLOFSET));
	while (npf > 0) {
		if (pte->pg_v) {
			c = &cmap[pgtocm(pte->pg_pfnum)];
			MLOCK(c);
		} 
		else
			pagein(ctob(v), 1);	/* return it locked */
		pte += CLSIZE;
		v += CLSIZE;
		npf -= CLSIZE;
	}
}

/* 
 * Unlock a virtual segment.
 */
vsunlock(base, count, rw)
	caddr_t base;
{
	register struct pte *pte;
	register int npf;
	register struct cmap *c;

	pte = vtopte(u.u_procp, btop(base));
	npf = btoc(count + ((int)base & CLOFSET));
	while (npf > 0) {
		c = &cmap[pgtocm(pte->pg_pfnum)];
		MUNLOCK(c);
		if (rw == B_READ)	/* Reading from device writes memory */
			pte->pg_m = 1;
		pte += CLSIZE;
		npf -= CLSIZE;
	}
}


/*
 *****************************************************************************
 *****************************************************************************
 *
 * Function:
 *
 *	km_alloc -- kernel routine
 *
 * Function description:
 *
 * 	This memory allocator will allocate any sized block of memory.  For
 *	efficiency sake, memory blocks are considered to be small blocks
 *	or large blocks.  Small blocks are defined as a request of up to 
 *	MAX_SB_SIZE bytes and large blocks are defined as a request of 
 *	greater than MAX_SB_SIZE bytes.  Small block allocation is 
 *	accomplished via a modified form of the "malloc" library 
 *	function from 4.2BSD.  Large blocks are simply passed on to the 
 *	routine kmemall for allocation/deallocation.
 * 
 * 	The amount of small block memory that is requested has the overhead 
 *	added on and is rounded up to the next power of 2 within the range
 *	of MIN_POWER_2 and INT_POWER_2.  If the requested size is falls 
 *	between INT_POWER_2 and MAX_POWER_2, then an extra page cluster is
 *	added on to account for the header.  This implies that most of these
 *	requests will be sized on even powers of 2.  The range MIN_POWER_2
 *	to MAX_POWER_2 controls the number of hash buckets needed to store 
 *	information about allocated memory.
 *
 *	Allocated memory blocks are guaranteed to start on octa-word
 *	boundaries, but not on page boundaries.
 *
 *	The constant information is contained in the file ../h/kmalloc.h
 *
 * Interface:
 *
 *	caddr_t km_alloc (nbytes, options)
 *	  unsigned int nbytes		number of bytes to allocate
 *	  int options			options
 *					  KM_CLRSG	clear block
 *					  KM_SLEEP	sleep if no memory
 *
 * Return Value:
 *
 *	= NULL		Error - No resources
 *	!= NULL		Normal - virtual address of beginning of block
 *
 * Error Handling:
 *
 *	For small buffers, a NULL is returned if kmemall returns a NULL && 
 *	there are no free buffers in higher bins.
 *
 *	For large buffers, a NULL is returned if kmemall returns a NULL.
 *
 * Panics:
 *
 *	None from this routine, though the routines that this routine calls
 *	do have panic conditions.
 *
 *	
 *****************************************************************************
 *****************************************************************************
 */


/*
 * nextf[i] is the pointer to the next free block of size 2^(i+MIN_POWER_2).
 * The smallest allocatable block is 2^MIN_POWER_2 bytes.  The overhead
 * information precedes the data area returned to the user.
 */
union overhead *nextf[NBUCKETS];



caddr_t
km_alloc(nbytes, options)
	register unsigned nbytes;	/* number of bytes requested */
	int options;
{
  	register union overhead *p;
  	register int bucket;
  	register unsigned shiftr;
	register s;
	void morecore();

	/*
 	 * Check for large block allocation.  If large block, just pass the
	 * request to kmemall.
	 */
	if (nbytes > MAX_SB_SIZE) {
#ifdef KM_STATS
		caddr_t tp;
		if((tp = kmemall(nbytes,options)) != NULL_PTR)
			km_stats.tot_km_alloc++;
		return(tp);
#else  KM_STATS
		return(kmemall(nbytes,options));
#endif KM_STATS
	}

	/* Find the appropriate bucket for this block size, after adding
	 * in enough room for the header and alignment.  If nbytes is large,
	 * then don't account for header, as morecore() will add an extra
	 * page.
	 */
	bucket = 0;
	if (nbytes > INT_SB_SIZE)
	  	shiftr = (nbytes - 1) >> (MIN_POWER_2 - 1);
	else
	  	shiftr = (nbytes + ALIGN_MASK) >> (MIN_POWER_2 - 1);
  	while (shiftr >>= 1)
  		bucket++;

	/* Check the appropriate hash bucket.  If there is nothing in it,
	 * then first attempt to allocate more memory from the system, and
	 * if that fails try to grab memory from higher buckets as a last
	 * resort. 
	 */
	s = splimp();
  	if ((p = nextf[bucket]) == NULL) {
  		morecore(bucket,options);
		while ((p = nextf[bucket]) == NULL)
			if ( ++bucket == NBUCKETS ) {
				splx(s);
				return(NULL_PTR);
			}
	}

	/* Remove from linked list
	 */
  	nextf[bucket] = p->ov_next;
	p->ov_magic = MAGIC;
	p->ov_index= bucket;

#ifdef KM_STATS
	km_stats.tot_km_alloc++;
	km_stats.tot_sb_allocs[bucket]++;
#endif KM_STATS
	splx(s);

	/* calculate beginning of buffer on ALIGN_MASK boundary */
	p = (union overhead *)(((u_int) (++p) + ALIGN_MASK) & ~ALIGN_MASK);

	/* Zero fill block? */
	if (options & KM_CLRSG)
		bzero((caddr_t) p, nbytes);

  	return ((caddr_t)(p));
}

/*
 * Allocate more memory to the indicated bucket.  Only used by km_alloc.
 */
void
morecore(bucket,options)
	register int bucket;	/* bucket to attach free list to */
	int options;		/* options to pass through to kmemall */
{
  	register union overhead *op;
	register unsigned allocation_size ;/* amount of memmory to allocate */
	register unsigned block_size ;	   /* size of blocks to allocate    */
	register int n_blocks ;		   /* # of blocks to allocate       */
	register int power_of_2 ;

	/* Since the minimum block size which can be allocated is 
	 * (1 << MIN_POWER_2), the actual power of 2 which a bucket
	 * represents is MIN_POWER_2 greater than the bucket number.
	 */
	power_of_2 = bucket + MIN_POWER_2;
	block_size = 1 << power_of_2;

	/* Assume the minimum allocation size but if the block size is
	 * greater, use it.  Then see how many blocks we will allocate.
	 */
	allocation_size = MIN_ALLOC_SIZE;
	if (block_size > allocation_size)
		allocation_size = block_size ;

	/* If size in bytes is large enough, add a page cluster to permit
 	 * header to be added
 	 */
	if (allocation_size > INT_SB_SIZE)
		allocation_size += CLBYTES;
	n_blocks = allocation_size >> power_of_2 ;

	/* Now allocate the needed memory and if the allocation didn't
	 * succeed just return.
	 */
	if ( (int)(op = (union overhead *)
			kmemall(allocation_size,options & ~KM_CLRSG)) == 0)
  		return;

	/*
	 * Put new memory allocated  on free list for this hash bucket.
	 */
  	nextf[bucket] = op;
  	while (--n_blocks > 0) {
		op->ov_next = (union overhead *)((int)op + block_size);
		op = (union overhead *)((int)op + block_size);
  	}

	/* Make sure final block points to NULL
	 */
	op->ov_next = NULL ;
#ifdef KM_STATS
	km_stats.tot_morecore++;
#endif KM_STATS
	return;
}


/*
 *****************************************************************************
 *****************************************************************************
 *
 * Function:
 *
 *	km_free -- kernel routine
 *
 * Function description:
 *
 *	This memory deallocator is the companion to "km_alloc", and is to
 *	be used to deallocation memory allocated via km_alloc.
 *
 *	This function is based on a modified form of the "kfree" library
 *	function from 4.2BSD.  Just like km_alloc, this routine will 
 *	deallocate blocks using two different methods, depending on the
 *	block size.  Large blocks will be passed on to "kmemfree", and 
 *	small blocks will be placed on the appropriate free list.
 *
 * Interface:
 *
 *	void km_free (va, nbytes)
 *	  caddr_t va			virtual address of beginning of block
 *	  unsigned int nbytes		number of bytes to allocate
 *
 * Return Value:
 *
 *	None
 *
 * Error Handling:
 *
 *	None
 *
 * Panics:
 *
 *	"km_free: Null pointer"
 *		A null pointer was passed into km_free.
 *
 *	"km_free: bad dealloc"
 *		This panic will occur if the magic number or the bucket index
 *		in the small buffer header is invalid.
 *
 *	
 *****************************************************************************
 *****************************************************************************
 */

void
km_free(cp,nbytes)
	register caddr_t cp;	/* pointer to buffer */
	register int nbytes;	/* only used for large block deallocation */
{   
  	register int bucket;
	register union overhead *op;
	register s;

  	if (cp == NULL_PTR)
  		panic("km_free: Null pointer\n");

	/*
 	 * Check for large block deallocation.  If large block, just pass the
	 * request to kmemfree.
	 */
	if (nbytes > MAX_SB_SIZE) {
		kmemfree(cp, nbytes);
		return;
	}

	/* back up to buffer header */
	op = (union overhead *) (((u_int) (cp - 1)) & ~ALIGN_MASK);

	/* Insure buffer integrity */
	if (op->ov_magic != MAGIC || op->ov_index >= NBUCKETS)
		panic("km_free: bad dealloc");

	/* Push buffer on to appropriate free list */
	s = splimp();
  	bucket = op->ov_index;
	op->ov_next = nextf[bucket];
  	nextf[bucket] = op;
#ifdef KM_STATS
	km_stats.tot_km_free++;
	km_stats.tot_sb_deallocs[bucket]++;
#endif KM_STATS
	splx(s);
}
