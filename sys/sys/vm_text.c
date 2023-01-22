#ifndef lint
static	char	*sccsid = "@(#)vm_text.c	1.16	(ULTRIX)	3/3/87";
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
 *   Modification History:
 *
 * 20 Feb 86 -- depp
 *	Added a tracing flag (GTRC) in the gnode to indicate that one
 *	or more processes are tracing this gnode.
 *
 * 15 Dec 86 -- depp
 *	Fixed a number of small problems:
 *	  1. In xalloc(), insured that 0407 processes return normally.
 *	  2. In General, insured that memory unhashing (when a gnode
 *	     is written to or the file system is umounted, occurs in a
 *	     consistent manner by using the new macro X_FLUSH (text.h).
 *	     Currently, X_FLUSH works in the traditional manner, not
 *	     flushing local references, only remote.
 *	  3. (from a previous submit) added macros to place/remove text
 *	     table entries on/from the free list -- X_QFREE/XDQFREE (text.h)
 *	  4. Removed the obsolete (and unused) mlock/munlock/mwait routines.
 *
 * 30 Oct 86 -- depp
 *	Fixed a problem with text table allocation (xalloc()).  The x_flag
 *	field of struct text has never been properly cleared on allocation.
 *	This fact was masked until two changes were maded: 1. the use
 *	of a paging threshold to determine whether to demand page a
 *	process from an inode or read in the entire process prior to 
 *	execution.  2. The reworking of text management for local execution
 *	of remote files.  
 *
 * 11 Sep 86 -- koehler
 *	added text management
 *
 * 29 Apr 86 -- depp
 *	converted to locking macros from calls routines
 *
 * 02 Apr 86 -- depp
 *	Added new parameter to "xalloc", a pointer to the a.out header
 *	data.  This data is now on the stack rather than in the "u" 
 *	structure.
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 * 14 Oct 85 -- reilly
 *	Modified user.h
 *
 * 18 Sep 85 -- depp
 *	Added reset of x_lcount when new text struct created (xalloc)
 *
 */

/*	vm_text.c	6.1	83/07/29	*/

#ifdef GFSDEBUG
extern short GFS[];
#endif

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/text.h"
#include "../h/gnode.h"
#include "../h/mount.h"
#include "../h/seg.h"
#include "../h/vm.h"
#include "../h/cmap.h"
#include "../h/exec.h"

struct xfree freetext;

/*
 * relinquish use of the shared text segment
 * of a process.
 */
xfree()
{
	register struct text *xp;
	register struct gnode *gp;
	register int count;
	
	if((xp=u.u_procp->p_textp) == NULL)
		return;
	X_LOCK(xp);
	gp = xp->x_gptr;
	
	if((count = --xp->x_count) < 0) {
		cprintf("xfree: text 0x%x count bad\n", xp);
		panic("xfree");
	}

	if((count == 0) && ((gp->g_mode & GSVTX) == 0)) {
		xunlink(u.u_procp);
		xp->x_rssize -= vmemfree(tptopte(u.u_procp, 0),	u.u_tsize);
		if (xp->x_rssize != 0) {
			panic("xfree rssize");
		}
		while (xp->x_poip)
			sleep((caddr_t)&xp->x_poip, PSWP+1);
    		if (u.u_procp->p_flag & STRC)
			gp->g_flag &= ~GTRC;
#ifdef GFSDEBUG
		if(GFS[18])
			cprintf("xfree: xp 0x%x gp 0x%x (%d) free %d\n", xp,
			xp->x_gptr, xp->x_gptr->g_number, xp->x_size);
#endif
		vsxfree(xp, (long)xp->x_size);

		/*
		 * place the text slot on the free text list
		 * note that text slots are taken off the rear
		 * and returned to the front
		 */
#ifdef GFSDEBUG
		if(GFS[18])
			cprintf("xfree: freeing %d\n", xp - text);

		if(xp->x_count) {
			cprintf("xfree: xp 0x%x has count\n", xp);
			panic("xfree");
		}
		if(gp->g_mode & GSVTX) {
			cprintf("xfree: xp 0x%x gp 0x%x %d is holdtext\n",
			xp, gp, gp->g_number);
			panic("xfree");
		}
		checktext(xp, 1);
#endif
		X_QFREE(xp);
		X_UNLOCK(xp);
	} 
	else {
		if (count == 0 && u.u_procp->p_flag & STRC)
			gp->g_flag &= ~GTRC;
		xp->x_flag &= ~XLOCK;
		xccdec(xp, u.u_procp);
	}
	u.u_procp->p_textp = NULL;
}


/*
 * Attach to a shared text segment.
 * If there is no shared text, just return.
 * If there is, hook up to it:
 * if it is not currently being used, it has to be read
 * in from the gnode (gp); the written bit is set to force it
 * to be written out as appropriate.
 * If it is being used, but is not currently in core,
 * a swap has to be done to get it back.
 */


xalloc(gp, ep, pagi)
	register struct gnode *gp;
	register struct exec *ep;
	int pagi;
{
	register struct text *xp;
	register size_t ts;
	register struct proc *p = u.u_procp;
	
	/* This accounts for the 0407 case */
	if(ep->a_text == 0)
		return(!NULL);
again:
	if(gp->g_flag & GTEXT) {	/*
					 * We have text pointer lets check it
					 * and run.
					 */
		xp = gp->g_textp;
		if (xp->x_gptr != gp)
			panic("Text Corruption gp != x_gptr");
		if ((xp->x_count > 0) || (xp->x_gptr->g_mode & GSVTX)) {
			if (xp->x_flag&XLOCK) {
				X_WAIT(xp);
				goto again;
			}
			X_LOCK(xp);
			xp->x_count++;
			if (p->p_flag & STRC)
				gp->g_flag |= GTRC;
#ifdef GFSDEBUG
			if(xp->x_freef || xp->x_freeb) {
				cprintf("xalloc: xp 0x%x on free list\n",
				xp->x_freef, xp->x_freeb);
				panic("xfree");
			}
#endif
			p->p_textp = xp;
			xlink(p);
			X_UNLOCK(xp);
			return(!NULL);
		} else {
			X_LOCK(xp);
#ifdef GFSDEBUG
			if(xp->x_freef == NULL) {
				cprintf("xalloc: xp 0x%x not on free list\n", xp);
				cprintf("Skipping panic to doadump...");
				doadump();
			}
			checktext(xp, 0);
#endif
			X_DQFREE(xp);
#ifdef GFSDEBUG
			xp->x_freef = xp->x_freeb = NULL;
#endif
		}
	} 
	else {	/*
		 * find a free text slot, if table is consumed, go
		 * searching for a potentially free slot
		 */
	
getfree:	if((xp = freetext.xun_freeb)  == (struct text *) &freetext) {
			tablefull("text");
			uprintf("text table full\n");
			psignal(p, SIGKILL);
			p->p_textp = NULL;
			return(NULL);
		}
		X_LOCK(xp);
#ifdef GFSDEBUG
		if(GFS[18])
			cprintf("xalloc: reusing %d\n", xp - text);
		checktext(xp, 0);
#endif
		X_DQFREE(xp);
#ifdef GFSDEBUG
		xp->x_freef = xp->x_freeb = NULL;
#endif
		if(xp->x_gptr) {
			struct gnode *xgp;
			extern int lbolt;

			xgp = xp->x_gptr;

			/*
			 * avoid race condition between gp/xp locking
			 */
			
			if(xgp->g_flag & GLOCKED) {
				X_QFREE(xp);
				if(xp->x_freef == (struct text *)
				&freetext) {

					/*
					 * we are here because we are
					 * trying to commit the last
					 * text slot but cannot get all
					 * the resources to fulfill the
					 * exec.  
					 * N.B. the code assumes that the
					 * X_QFREE() puts things on the 
					 * front of the list
					 */
					
					tablefull("text (commit)");
					uprintf("text table full\n");
					psignal(p, SIGKILL);
					p->p_textp = NULL;
					X_UNLOCK(xp)
					return(NULL);
				}
				X_UNLOCK(xp);
				sleep(&lbolt, PZERO); /* avoid spinning */
				goto getfree;
			}
			(void) GLOCK(xgp);
			X_FLUSH(xp,xgp);	/* flush hash lists */
			xgp->g_flag &= ~GTEXT;
			xp->x_gptr = NULL;
			xgp->g_textp = NULL;
			gput(xgp);
		}
		xp->x_flag = 0;
	}
	
	xp->x_flag |= XLOAD;
	if (pagi)
		xp->x_flag |= XPAGI;
	ts = clrnd(btoc(ep->a_text));
	xp->x_size = ts;
	xp->x_gptr = gp;
	gp->g_textp = xp;

#ifdef GFSDEBUG
	if((xp->x_count != 0) && (xp->x_count != 1)) {
		cprintf("xalloc: xp 0x%x bad count on allocation\n", xp);
		panic("xalloc");
	}
	if((gp->g_flag & GLOCKED) == 0)
		cprintf("xalloc: gp 0x%x (%d) not locked\n", gp, gp->g_number);
	if(GFS[18])
		cprintf("xalloc: xp 0x%x gp 0x%x (%d) size %d\n", xp,
		xp->x_gptr, xp->x_gptr->g_number, ts);
#endif
	if (vsxalloc(xp) == NULL) {
		swkill(p, "xalloc: no swap space");

		/*
		 * process is not executable, leave enough
		 * context so that text reclaim may be possible
		 */
		
		xp->x_flag = 0;
		xp->x_size = 0;
		xp->x_gptr = NULL;
		gp->g_textp = NULL;
		gp->g_flag &= ~GTEXT;
		p->p_textp = NULL;		
#ifdef GFSDEBUG
		checktext(xp, 1);
#endif
		X_QFREE(xp);
		X_UNLOCK(xp);
		return(NULL);
	}
	if(!(gp->g_flag & GTEXT)) {   /*
				       * if the flag is set then the gnode
				       * is already allocated and the count
				       * was already bumped. if not set the
				       * flag and bump the count
				       */
		gp->g_flag |= GTEXT;
		gp->g_count++;
	}
#ifdef GFSDEBUG
	if(xp->x_freef || xp->x_freeb) {
		cprintf("xalloc: new xp 0x%x on free list\n", xp);
		panic("xalloc");
	}
#endif

	xp->x_gptr = gp;	
	gp->g_textp = xp;
#ifdef GFSDEBUG
	if(xp->x_count)
		cprintf("xalloc: xp 0x%x count %d\n", xp, xp->x_count);
#endif
	xp->x_count = 1;
	xp->x_ccount = 0;
	xp->x_lcount = 0;
	xp->x_rssize = 0;
	p->p_textp = xp;
	xlink(p);
	if (pagi == 0) {
		settprot(RW);
		p->p_flag |= SKEEP;
		u.u_error = rdwri(UIO_READ, gp, 
		    (caddr_t)ctob(tptov(p, 0)),
		    (int)ep->a_text,
		    (int)(ep->a_magic==0413 ? CLBYTES : sizeof (struct exec)),
		    2, (int *)0);
		if (u.u_error) {
			swkill (p, "xalloc: error reading text");
			X_UNLOCK(xp);
			settprot(RO);
			return(NULL);
		}
		p->p_flag &= ~SKEEP;
	}
	settprot(RO);
	xp->x_flag |= XWRIT;
	xp->x_flag &= ~XLOAD;
	if (p->p_flag & STRC) 
		gp->g_flag |= GTRC;
	X_UNLOCK(xp);
	return(!NULL);
}

/*
 * Decrement the in-core usage count of a shared text segment.
 * When it drops to zero, free the core space.
 */
xccdec(xp, p)
register struct text *xp;
register struct proc *p;
{

	if (xp==NULL || xp->x_ccount==0)
		return;
	X_LOCK(xp);
	if (--xp->x_ccount == 0) {
		if (xp->x_flag & XWRIT) {
			vsswap(p, tptopte(p, 0), CTEXT, 0, xp->x_size,
							(struct dmap *)0);
			if (xp->x_flag & XPAGI)
				swap(p, xp->x_ptdaddr, (caddr_t)tptopte(p, 0),
					xp->x_size * sizeof (struct pte),
					B_WRITE, B_PAGET, swapdev, 0);
			xp->x_flag &= ~XWRIT;
		} 
		else {
			xp->x_rssize -= vmemfree(tptopte(p, 0),xp->x_size);
		}
		if (xp->x_rssize != 0)
			panic("text rssize");
	}
	xunlink(p);
	X_UNLOCK(xp);
}

/*
 * free the swap image of all unused saved-text text segments
 * which are from device dev (used by umount system call).
 */
xumount(dev)
register dev;
{
	register struct text *xp;

	for (xp = text; xp < textNTEXT; xp++) {
		if (xp->x_gptr &&  dev==xp->x_gptr->g_dev) 
		{
				xuntext(xp);
		}
	}

}

/*
 * remove a shared text segment from the text table, if possible.
 */
xrele(gp)
register struct gnode *gp;
{
	register struct text *xp;

	if ((gp->g_flag&GTEXT)==0)
		return;
	xp = gp->g_textp;
	if (gp!=xp->x_gptr)
		panic("xrele");
	xuntext(xp);
}

/*
 * remove text image from the text table.
 * the use count must be zero.
 */
xuntext(xp)
register struct text *xp;
{
	register struct gnode *gp;

	X_LOCK(xp);
	if (xp->x_count) {
#ifdef GFSDEBUG
		if(GFS[18]) {
			gp = xp->x_gptr;
			printf("xuntext: xp 0x%x gp 0x%x (%d) count %d not FREED\n",
				xp, gp, gp->g_number, xp->x_count);
		}
#endif
		X_UNLOCK(xp);
		return;
	}
	gp = xp->x_gptr;
	xp->x_flag &= ~XLOCK;
#ifdef GFSDEBUG
	if(GFS[18])
		printf("xuntext: xp 0x%x gp 0x%x (%d) size %d\n", xp, gp,
		gp->g_number, xp->x_size);
#endif
	if(gp->g_mode & GSVTX) 
		vsxfree(xp, (long)xp->x_size);
	X_FLUSH(xp,gp);	/* flush hash lists */
	gp->g_textp = NULL;
	xp->x_gptr = NULL;	
	gp->g_flag &= ~GTEXT;
	GRELE(gp);
}

/*
 * Add a process to those sharing a text segment by
 * getting the page tables and then linking to x_caddr.
 */
xlink(p)
register struct proc *p;
{
	register struct text *xp = p->p_textp;

	if (xp == 0)
		return;
	vinitpt(p);
	p->p_xlink = xp->x_caddr;
	xp->x_caddr = p;
	xp->x_ccount++;
}

xunlink(p)
register struct proc *p;
{
	register struct text *xp = p->p_textp;
	register struct proc *q;

	if (xp == 0)
		return;
	if (xp->x_caddr == p) {
		xp->x_caddr = p->p_xlink;
		p->p_xlink = 0;
		return;
	}
	for (q = xp->x_caddr; q->p_xlink; q = q->p_xlink)
		if (q->p_xlink == p) {
			q->p_xlink = p->p_xlink;
			p->p_xlink = 0;
			return;
		}
	panic("lost text");
}

/*
 * Replace p by q in a text incore linked list.
 * Used by vfork(), internally.
 */
xrepl(p, q)
register struct proc *p, *q;
{
	register struct text *xp = q->p_textp;

	if (xp == 0)
		return;
	xunlink(p);
	q->p_xlink = xp->x_caddr;
	xp->x_caddr = q;
}

/*
 * Flush the free list of text references for remote images so that
 * they will not be reclaimed when the hash is done.  This is a pain
 * but short of putting a fhandle, or reference there to in each 
 */

xflush_free_text(xp)
	register struct text *xp;
{
	register struct cmap *c1;
	register struct cmap *cptr = xp->x_cmap;
	register int	index = xp - text;
	register int i;

	if((xp->x_dindex != cptr->c_mdev) || (xp->x_blkno != cptr->c_blkno))
		return;


	while((cptr->c_type == CTEXT) && (cptr->c_ndx == index)) {
		if (cptr->c_blkno > 0) {
		/* if blkno is zero it was already chgprot-ed for tracing */
		/* or it hasn't been written to swap yet, so no blkno yet */
			i = CMHASH(cptr->c_blkno);
			c1 = &cmap[cmhash[i]];
			/* This entry must be unhashed */
			if(c1 == ecmap)
				panic("pseudo munhash1");
			if(c1 == cptr) {
				cmhash[i] = cptr->c_hlink;
			}
			else {
				while(&cmap[c1->c_hlink] != cptr){
					if((c1 = &cmap[c1->c_hlink]) == ecmap)
						panic("pseudo munhash2");
				}
				c1->c_hlink = cptr->c_hlink;
			}
		}
		cptr->c_mdev = (u_char) NODEV;
		cptr->c_blkno = 0;
		cptr->c_hlink = 0;
		if ((cptr = &cmap[cptr->c_prev]) == CMHEAD)
			return;
	}
}

textinit() {
	
	/* 
	 * set up the doubly linked list of free text structures
	 */
	
	register struct text *xp;
	
	freetext.xun_freef = freetext.xun_freeb = (struct text *) &freetext;
	
	for(xp = &text[ntext - 1]; xp >= text; xp--) 
		X_QFREE(xp);
}

#ifdef GFSDEBUG
checktext(xp, in)
	register struct text *xp;
	register int in;
{
	register struct text *txp;
	register int ret = 0;
	
	if (!(xp->x_flag & XLOCK)) {
		cprintf("checktext: xp 0x%x in = %d",xp,in);
		panic("unlocked xp");
	}
	if(xp->x_count) {
		cprintf("checktext: %s xp 0x%x has count of %d\n",
		in ? "insertion" : "deletion", xp, xp->x_count);
		panic("checktext::");
	}
	txp = freetext.xun_freef;
	while(txp != (struct text *) &freetext) {
		if(txp == xp) {
			if(in) {
				cprintf("checktext: xp 0x%x on insertion freelist\n",
				xp);
				panic("checktext");
			} else
				ret++;
		}
		txp = txp->x_freef;
	}
	if(!in && (ret != 1)) {
		cprintf("checktext: xp 0x%x deletion %s on freelist\n", xp,
		ret == 0 ? "not" : "multiply");
		panic("checktext");
	}
}
#endif
