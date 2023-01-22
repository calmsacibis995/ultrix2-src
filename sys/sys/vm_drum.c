#ifndef lint
static char *sccsid = "@(#)vm_drum.c	1.11	ULTRIX	1/15/87";
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
 * 15 Jan 86 -- depp
 *	Fixed SM bug in vtod().
 *
 * 27 Aug 86 -- depp
 *	Fixed bug in vsswap that caused the swapper to hang
 *
 * 29 Apr 86 -- depp
 *	converted to locking macros from calls routines
 *
 * 17 Mar 86 -- depp
 *	Added dynamic allocation/deallocation of sm_daddr array to 
 *	vssmalloc and vssmfree
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 * 11 Mar 85 -- depp
 *	Added System V shared memory support.  Two new routines added:
 *		vssmalloc	Allocate shared memory swap area
 *		vssmfree	Deallocate shared memory swap area
 *
 */

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/text.h"
#include "../h/map.h"
#include "../h/vm.h"
#include "../h/cmap.h"
#include "../h/kernel.h"
#include "../h/ipc.h"
#include "../h/shm.h"
#include "../h/kmalloc.h"
extern struct sminfo sminfo;

/* macro to calculate sm_daddr size in bytes (vssmalloc, vssmfree) */
#define DADDR_SIZE(npg)	(((ctod(npg) + (dmtext - 1)) / dmtext) * 4)

/*
 * Expand the swap area for both the data and stack segments.
 * If space is not available for both, retract and return 0.
 */
swpexpand(ds, ss, dmp, smp)
	size_t ds, ss;
	register struct dmap *dmp, *smp;
{
	register struct dmap *tmp;
	register int ts;
	size_t ods;

	/*
	 * If dmap isn't growing, do smap first.
	 * This avoids anomalies if smap will try to grow and
	 * fail, which otherwise would shrink ds without expanding
	 * ss, a rather curious side effect!
	 */
	if (dmp->dm_alloc > ds) {
		tmp = dmp; ts = ds;
		dmp = smp; ds = ss;
		smp = tmp; ss = ts;
	}
	ods = dmp->dm_size;
	if (vsexpand(ds, dmp, 0) == 0)
		goto bad;
	if (vsexpand(ss, smp, 0) == 0) {
		(void) vsexpand(ods, dmp, 1);
		goto bad;
	}
	return (1);

bad:
	u.u_error = ENOMEM;
	return (0);
}

/*
 * Expand or contract the virtual swap segment mapped
 * by the argument diskmap so as to just allow the given size.
 *
 * FOR NOW CANT RELEASE UNLESS SHRINKING TO ZERO, SINCE PAGEOUTS MAY
 * BE IN PROGRESS... TYPICALLY NEVER SHRINK ANYWAYS, SO DOESNT MATTER MUCH
 */
vsexpand(vssize, dmp, canshrink)
	register size_t vssize;
	register struct dmap *dmp;
{
	register long blk = dmmin;
	register int vsbase = 0;
	register swblk_t *ip = dmp->dm_map;
	size_t oldsize = dmp->dm_size;
	size_t oldalloc = dmp->dm_alloc;

	vssize = ctod(vssize);
	while (vsbase < oldalloc || vsbase < vssize) {
		if (ip - dmp->dm_map >= NDMAP)
			panic("vmdrum NDMAP");
		if (vsbase >= oldalloc) {
			*ip = rmalloc(swapmap, blk);
			if (*ip == 0) {
				dmp->dm_size = vsbase;
				if (vsexpand(dtoc(oldsize), dmp, 1) == 0)
					panic("vsexpand");
				return (0);
			}
			dmp->dm_alloc += blk;
		} else if (vssize == 0 ||
		    vsbase >= vssize && canshrink) {
			rmfree(swapmap, blk, *ip);
			*ip = 0;
			dmp->dm_alloc -= blk;
		}
		vsbase += blk;
		if (blk < dmmax)
			blk *= 2;
		ip++;
	}
	dmp->dm_size = vssize;
	return (1);
}

/*
 * Allocate swap space for a text segment,
 * in chunks of at most dmtext pages.
 */
vsxalloc(xp)
	struct text *xp;
{
	register long blk;
	register swblk_t *dp;
	swblk_t vsbase;

	if (ctod(xp->x_size) > NXDAD * dmtext)
		return (0);
	dp = xp->x_daddr;
	for (vsbase = 0; vsbase < ctod(xp->x_size); vsbase += dmtext) {
		blk = ctod(xp->x_size) - vsbase;
		if (blk > dmtext)
			blk = dmtext;
		if ((*dp++ = rmalloc(swapmap, blk)) == 0) {
			vsxfree(xp, vsbase);
			return (0);
		}
	}
	if (xp->x_flag & XPAGI) {
		xp->x_ptdaddr = rmalloc(swapmap,
				(long)ctod(clrnd(ctopt(xp->x_size))));
		if (xp->x_ptdaddr == 0) {
			vsxfree(xp, (long)xp->x_size);
			return (0);
		}
	}
	return (1);
}


/* VSSMALLOC - Allocate swap space for a shared memory segment,	*/
/*	in chunks of at most dmtext pages.			*/
/* LeRoy Fundingsland    1/22/85    DEC				*/

vssmalloc(sp)
    register struct smem *sp;{
	register long blk;
	register swblk_t *dp;
	register int smsize;		/* in clicks		*/
	swblk_t vsbase;

	smsize = clrnd((int)btoc(sp->sm_size));

	/* allocate space for shared memory segment swap blocks */
	if ((dp = sp->sm_daddr = (swblk_t *) 
	   km_alloc(DADDR_SIZE(smsize),KM_SLEEP|KM_CLRSG)) == (swblk_t *)NULL)
		return (0);

	/* allocate swap space in at most dmtext chunks */
	for (vsbase=0; vsbase < ctod(smsize); vsbase+=dmtext) {
		blk = ctod(smsize) - vsbase;
		if (blk > dmtext)
			blk = dmtext;
		if ((*dp++ = rmalloc(swapmap, blk)) == 0) {
			vssmfree(sp, vsbase);
			return (0);
		}
	}

	sp->sm_ptdaddr = rmalloc(swapmap,
				(long)ctod(clrnd(ctopt(smsize))));
	if (sp->sm_ptdaddr == 0) {
		vssmfree(sp, (long)smsize);
		return(0);
	}

	return(1);
}

/*
 * Free the swap space of a text segment which
 * has been allocated ts pages.
 */
vsxfree(xp, ts)
	struct text *xp;
	long ts;
{
	register long blk;
	register swblk_t *dp;
	swblk_t vsbase;

	ts = ctod(ts);
	dp = xp->x_daddr;
	for (vsbase = 0; vsbase < ts; vsbase += dmtext) {
		blk = ts - vsbase;
		if (blk > dmtext)
			blk = dmtext;
		rmfree(swapmap, blk, *dp);
		*dp++ = 0;
	}
	if ((xp->x_flag&XPAGI) && xp->x_ptdaddr) {
		rmfree(swapmap, (long)ctod(clrnd(ctopt(xp->x_size))),
		    xp->x_ptdaddr);
		xp->x_ptdaddr = 0;
	}
}

/* VSSMFREE - virtual swap shared memory free. Free the swap	*/
/*	space of a shared memory segment which has been		*/
/*	allocated size pages.					*/
/* LeRoy Fundingsland    1/16/85    DEC				*/
vssmfree(sp, size)
    register struct smem *sp;	/* pointer to shared memory header */
    long size;{
	register long blk;
	register swblk_t *dp;
	register int smsize;		/* in clicks		*/
	swblk_t vsbase;

	size = ctod(size);
	smsize = clrnd((int)btoc(sp->sm_size));

	if ((dp = sp->sm_daddr) == 0)
		panic("vssmfree: sm_daddr");

	/* free swap space for segment		*/
	for (vsbase=0; vsbase < size; vsbase+=dmtext) {
		blk = size - vsbase;
		if (blk > dmtext)
			blk = dmtext;
		rmfree(swapmap, blk, *dp);
		dp++;
	}

	/* free the daddr space */
	km_free(sp->sm_daddr,DADDR_SIZE(smsize));
	sp->sm_daddr = 0;

	/* free swap space for segment PTEs	*/
	if (sp->sm_ptdaddr) {
		rmfree(swapmap, (long)ctod(clrnd(ctopt(smsize))),
						sp->sm_ptdaddr);
		sp->sm_ptdaddr = 0;
	}
}

/*
 * Swap a segment of virtual memory to disk,
 * by locating the contiguous dirty pte's
 * and calling vschunk with each chunk.
 */
vsswap(p, pte, type, vsbase, vscount, dmp)
	struct proc *p;
	register struct pte *pte;
	int type;
	register int vsbase, vscount;
	struct dmap *dmp;
{
	register int size = 0;
	register struct cmap *c;

	if (vscount % CLSIZE)
		panic("vsswap");
	if (vscount == 0)
		return;
	for (;;) {
		if (pte->pg_fod == 0 && pte->pg_pfnum) {
			c = &cmap[pgtocm(pte->pg_pfnum)];
			MWAIT(c);
		}
		if (!dirtycl(pte)) {
			if (size) {
				vschunk(p, vsbase, size, type, dmp);
				vsbase += size;
				size = 0;
			}
			vsbase += CLSIZE;
			if (pte->pg_fod == 0 && pte->pg_pfnum)
				if (type == CTEXT)
					p->p_textp->x_rssize -= 
						vmemfree(pte, CLSIZE);
				/* SHMEM */
				else if (type == CSMEM)
					((struct smem *)p)->sm_rssize -=
						vmemfree(pte, CLSIZE);
				else
					p->p_rssize -= vmemfree(pte, CLSIZE);
		} else {
			size += CLSIZE;
		}

		if ((vscount -= CLSIZE) == 0) {
			if (size)
				vschunk(p, vsbase, size, type, dmp);
			return;
		}

		if (type == CSTACK)
			pte -= CLSIZE;
		else
			pte += CLSIZE;
	}
}

vschunk(p, base, size, type, dmp)
	register struct proc *p;
	register int base, size;
	int type;
	struct dmap *dmp;
{
	register struct pte *pte;
	struct dblock db;
	unsigned v;

	base = ctod(base);
	size = ctod(size);
	if (type == CTEXT) {
		while (size > 0) {
			db.db_size = dmtext - base % dmtext;
			if (db.db_size > size)
				db.db_size = size;
			swap(p, p->p_textp->x_daddr[base/dmtext] + base%dmtext,
			    ptob(tptov(p, dtoc(base))), (int)dtob(db.db_size),
			    B_WRITE, 0, swapdev, 0);
			pte = tptopte(p, dtoc(base));
			p->p_textp->x_rssize -=
			    vmemfree(pte, (int)dtoc(db.db_size));
			base += db.db_size;
			size -= db.db_size;
		}
		return;
	}
	/* begin SHMEM */
	if (type == CSMEM) {
		while (size > 0) {
			db.db_size = dmtext - base % dmtext;
			if (db.db_size > size)
				db.db_size = size;
			swap(p, ((struct smem *)p)->
				sm_daddr[base/dmtext] + base%dmtext,
				ptob(dtoc(base)), (int)dtob(db.db_size),
				B_WRITE, B_SMEM, swapdev, 0);
			pte = ((struct smem *)p)->sm_ptaddr +
							dtoc(base);
			((struct smem *)p)->sm_rssize -=
				vmemfree(pte, (int)dtoc(db.db_size));
			base += db.db_size;
			size -= db.db_size;
		}
		return;
	}
	/* end SHMEM */
	do {
		vstodb(base, size, dmp, &db, type == CSTACK);
		v = type==CSTACK ?
		    sptov(p, dtoc(base+db.db_size)-1) :
		    dptov(p, dtoc(base));
		swap(p, db.db_base, ptob(v), (int)dtob(db.db_size),
		    B_WRITE, 0, swapdev, 0);
		pte = type==CSTACK ?
		    sptopte(p, dtoc(base+db.db_size)-1) :
		    dptopte(p, dtoc(base));
		p->p_rssize -= vmemfree(pte, (int)dtoc(db.db_size));
		base += db.db_size;
		size -= db.db_size;
	} while (size != 0);
}

/*
 * Given a base/size pair in virtual swap area,
 * return a physical base/size pair which is the
 * (largest) initial, physically contiguous block.
 */
vstodb(vsbase, vssize, dmp, dbp, rev)
	register int vsbase, vssize;
	struct dmap *dmp;
	register struct dblock *dbp;
{
	register int blk = dmmin;
	register swblk_t *ip = dmp->dm_map;

	if (vsbase < 0 || vssize < 0 || vsbase + vssize > dmp->dm_size)
		panic("vstodb");
	while (vsbase >= blk) {
		vsbase -= blk;
		if (blk < dmmax)
			blk *= 2;
		ip++;
	}
	if (*ip + blk > nswap)
		panic("vstodb *ip");
	dbp->db_size = imin(vssize, blk - vsbase);
	dbp->db_base = *ip + (rev ? blk - (vsbase + dbp->db_size) : vsbase);
}

/*
 * Convert a virtual page number 
 * to its corresponding disk block number.
 * Used in pagein/pageout to initiate single page transfers.
 */
swblk_t
vtod(p, v, dmap, smap)
	register struct proc *p;
	unsigned v;
	struct dmap *dmap, *smap;
{
	struct dblock db;
	int tp;

	if (isatsv(p, v)) {
		tp = ctod(vtotp(p, v));
		return (p->p_textp->x_daddr[tp/dmtext] + tp%dmtext);
	}
	if (isassv(p, v))
		vstodb(ctod(vtosp(p, v)), ctod(1), smap, &db, 1);
	else
		/* begin SHMEM */
		if(vtodp(p, v) >= p->p_dsize){
			register int i, xp, smp;

			xp = vtotp(p, v);
			for(i=0; i < sminfo.smseg; i++){
				if(p->p_sm[i].sm_p == NULL)
					continue;
				if(xp >= p->p_sm[i].sm_spte  &&
					xp < p->p_sm[i].sm_spte +
					btoc(p->p_sm[i].sm_p->sm_size))

					break;
			}
			if(i >= sminfo.smseg)
				panic("pagin SMEM");
			smp = xp - p->p_sm[i].sm_spte;
			return(p->p_sm[i].sm_p->sm_daddr[smp/dmtext] +
							smp%dmtext);
		}
		/* end SHMEM */
		else
			vstodb(ctod(vtodp(p, v)), ctod(1), dmap, &db, 0);
	return (db.db_base);
}
