#ifndef lint
static	char	*sccsid = "@(#)gfs_bio.c	1.16	(ULTRIX)	6/4/87";
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


/***********************************************************************
 *
 *		Modification History
 *
 * 13 Feb 87 -- prs
 *	Changed bhalt to wait on busy buffers
 *
 * 12 Feb 87 -- chet
 *	match buffer and gnode before bwrite() GIOSTRATEGY call
 *
 * 29 Jan 87 -- chet
 *	add new arg to bdwrite()
 *
 * 22 Jan 87 -- koehler
 *	changed bhalt so that busy buffers aren't flushed
 *
 * 31 Oct 86 -- koehler
 *	changed bwrite so that gp isn't needed
 *
 * 11 Sep 86 -- koehler
 *	changes for synchronous filesystems
 *
 ***********************************************************************/


#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/proc.h"
#include "../h/seg.h"
#include "../h/vm.h"
#include "../h/trace.h"
#include "../h/gnode.h"
#include "../h/mount.h"

#ifdef GFSDEBUG
extern short GFS[];
#endif

/*
 * Read in (if necessary) the block and return a buffer pointer.
 */
struct buf *
bread(dev, blkno, size, gp)
	dev_t dev;
	register daddr_t blkno;
	register int size;
	register struct gnode *gp;
{
	register struct buf *bp;

	if (size == 0)
		panic("bread: size 0");
Loop:
#ifdef GFSDEBUG
	if(GFS[12])
		cprintf("bread: gp 0x%x (%d) trying to read blkno %d size %d\n",
		gp, gp->g_number, blkno, size);
#endif
	bp = getblk(dev, blkno, size, gp);
	if (bp->b_flags&B_DONE) {
		trace(TR_BREADHIT, dev, blkno);
		return(bp);
	}
	/*
	 *	Before we go read the block make sure that the block
	 *	that is to be read in is not delayed write.  
	 */
	if (bp->b_flags&B_DELWRI) {
		bwrite(bp);
		goto Loop;
	}
	bp->b_flags |= B_READ;
	if (bp->b_bcount > bp->b_bufsize)
		panic("bread");

	if (gp && GIOSTRATEGY(gp) && ((gp->g_mode & GFBLK) != GFBLK) && ((gp->g_mode & GFCHR) != GFCHR)) {
#ifdef GFSDEBUG
		if(GFS[19])
			cprintf("bread: calling alternate strat routine at 0x%x\n",
			GIOSTRATEGY(gp));
#endif
		(*GIOSTRATEGY(gp))(bp);
	}
	else
		(*bdevsw[major(dev)].d_strategy)(bp);
	trace(TR_BREADMISS, dev, blkno);
	u.u_ru.ru_inblock++;		/* pay for read */
	biowait(bp);
	return(bp);
}

/*
 * Read in the block, like bread, but also start I/O on the
 * read-ahead block (which is not allocated to the caller)
 */
struct buf *
breada(dev, blkno, size, rablkno, rabsize, gp)
	dev_t dev;
	register daddr_t blkno; 
	register int size;
	register daddr_t rablkno;
	register struct gnode *gp;
	int rabsize;
{
	register struct buf *bp, *rabp;
	
#ifdef GFSDEBUG
	if(GFS[12])
		cprintf("breada: gp 0x%x (%d) trying to read blkno %d size %d\n",
		gp, gp->g_number, blkno, size);
#endif

	bp = NULL;
	/*
	 * If the block isn't in core, then allocate
	 * a buffer and initiate i/o (getblk checks
	 * for a cache hit).
	 */
	if (!incore(dev, blkno, gp)) {
		bp = getblk(dev, blkno, size, gp);
		if ((bp->b_flags&B_DONE) == 0) {
			bp->b_flags |= B_READ;
			if (bp->b_bcount > bp->b_bufsize)
				panic("breada");
			if (gp && GIOSTRATEGY(gp) && ((gp->g_mode & GFBLK) != GFBLK) && ((gp->g_mode & GFCHR) != GFCHR))
				(*GIOSTRATEGY(gp))(bp);
			else
				(*bdevsw[major(dev)].d_strategy)(bp);
			trace(TR_BREADMISS, dev, blkno);
			u.u_ru.ru_inblock++;		/* pay for read */
		} else
			trace(TR_BREADHIT, dev, blkno);
	}

	/*
	 * If there's a read-ahead block, start i/o
	 * on it also (as above).
	 */
	if (rablkno && !incore(dev, rablkno, gp)) {
		rabp = getblk(dev, rablkno, rabsize, gp);
		if (rabp->b_flags & B_DONE) {
			brelse(rabp);
			trace(TR_BREADHITRA, dev, blkno);
		} else {
			rabp->b_flags |= B_READ|B_ASYNC;
			if (rabp->b_bcount > rabp->b_bufsize)
				panic("breadrabp");
			if (gp && GIOSTRATEGY(gp) && ((gp->g_mode & GFBLK) != GFBLK) && ((gp->g_mode & GFCHR) != GFCHR))
				(*GIOSTRATEGY(gp))(rabp);
			else
				(*bdevsw[major(dev)].d_strategy)(rabp);
			trace(TR_BREADMISSRA, dev, rablock);
			u.u_ru.ru_inblock++;		/* pay in advance */
		}
	}

	/*
	 * If block was in core, let bread get it.
	 * If block wasn't in core, then the read was started
	 * above, and just wait for it.
	 */
	if (bp == NULL)
		return (bread(dev, blkno, size, gp));
	biowait(bp);
	return (bp);
}

/*
 * Write the buffer, waiting for completion.
 * Then release the buffer.
 */
bwrite(bp)
	register struct buf *bp;
{
	register flag;
	register struct gnode *gp;
	register struct mount *mp;
	
#ifdef GFSDEBUG
	if(GFS[12])
		cprintf("bwrite: gp 0x%x (%d) trying to write blkno %d count %d\n",
		bp->b_gp, bp->b_gp ? bp->b_gp->g_number : -1, bp->b_blkno,
		bp->b_bcount);
#endif
	
	flag = bp->b_flags;
	bp->b_flags &= ~(B_READ | B_DONE | B_ERROR | B_DELWRI);
	if ((flag&B_DELWRI) == 0)
		u.u_ru.ru_oublock++;		/* noone paid yet */
	trace(TR_BWRITE, bp->b_dev, bp->b_blkno);
	if (bp->b_bcount > bp->b_bufsize)
		panic("bwrite");

#ifdef GFSDEBUG
	if(GFS[19]) {
		cprintf("bwrite: bp 0x%x dev 0x%x bp->b_gp 0x%x gp 0x%x mp 0x%x\n", bp,
		bp->b_dev, bp->b_gp, gp, mp);
		cprintf("bwrite: (%d) mp 0x%x\n", gp ? gp->g_number : -1,
		gp ? gp->g_mp : 0);
	}
#endif
	gp = bp->b_gp;
	if (gp && GIOSTRATEGY(gp) &&
		((gp->g_mode & GFBLK) != GFBLK) &&
		((gp->g_mode & GFCHR) != GFCHR) &&
		bp->b_gid == gp->g_id)
	{
		(*GIOSTRATEGY(gp))(bp);
	} else
		(*bdevsw[major(bp->b_dev)].d_strategy)(bp);

	/*
	 * If the write was synchronous, then await i/o completion.
	 * If the write was "delayed", then we put the buffer on
	 * the q of blocks awaiting i/o completion status.
	 */
	if ((flag&B_ASYNC) == 0) {
		biowait(bp);
		brelse(bp);
	} else if (flag & B_DELWRI)
		bp->b_flags |= B_AGE;
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 * This can't be done for magtape, since writes must be done
 * in the same order as requested.
 */
bdwrite(bp, gp)
	register struct buf *bp;
	register struct gnode *gp;
{
	register int flags = 0;
	register struct mount *mp;

	mp = bp->b_gp ? bp->b_gp->g_mp : NULL;

#ifdef GFSDEBUG
	if(GFS[12])
		cprintf("bdwrite: gp 0x%x (%d) trying to write blkno %d count %d\n",
		bp->b_gp, bp->b_gp ? bp->b_gp->g_number : -1, bp->b_blkno,
		bp->b_bcount);
#endif

	bp->b_gp = gp;
	if ((bp->b_flags&B_DELWRI) == 0)
		u.u_ru.ru_oublock++;		/* noone paid yet */
	if(bp->b_dev < nblkdev + nchrdev)
		flags = bdevsw[major(bp->b_dev)].d_flags;

	if(flags & B_TAPE) {
		bawrite(bp);
		return;
	}

	if(mp && (mp->m_flags & M_SYNC)) {
		bwrite(bp);
		return;
	}
	
	bp->b_flags |= B_DELWRI | B_DONE;
	brelse(bp);
}

#ifdef notdef
/* this is now a macro */
/*
 * Release the buffer, start I/O on it, but don't wait for completion.
 */
bawrite(bp)
	register struct buf *bp;
{
	register struct mount *mp;

	mp = bp->b_gp ? bp->b_gp->g_mp : NULL;

	/* check to see if this is a synchronous filesystem */

	if((mp == NULL) || ((mp->m_flags & M_SYNC) == NULL)) {
		bp->b_gp = NULL;
		bp->b_flags |= B_ASYNC;
	}
	bwrite(bp);
}
#endif notdef

/*
 * Release the buffer, with no I/O implied.
 */
brelse(bp)
	register struct buf *bp;
{
	register struct buf *flist;
	register s;
#ifdef GFSDEBUG
	register struct buf *mbp;
	register struct buf *hp;
#define dp ((struct buf *)hp)

	for (hp = bfreelist; hp < &bfreelist[BQ_EMPTY]; hp++)
		for (mbp = dp->av_forw; mbp != hp; mbp = mbp->av_forw)
			if (mbp == bp) {
				cprintf("brelse: bp 0x%x dev 0x%x gp 0x%x (%d) already on list\n",
				bp, bp->b_dev, bp->b_gp, bp->b_gp ? 
				bp->b_gp->g_number : -1);
				panic("brelse: bletch");
			}
#undef dp
#endif
	/*
	 * If someone's waiting for the buffer, or
	 * is waiting for a buffer wake 'em up.
	 */
	if (bp->b_flags&B_WANTED)
		wakeup((caddr_t)bp);
	if (bfreelist[0].b_flags&B_WANTED) {
		bfreelist[0].b_flags &= ~B_WANTED;
		wakeup((caddr_t)bfreelist);
	}
	if (bp->b_flags&B_ERROR)
		if (bp->b_flags & B_LOCKED)
			bp->b_flags &= ~B_ERROR;	/* try again later */
		else
			bp->b_dev = NODEV;  		/* no assoc */

	/*
	 * Stick the buffer back on a free list.
	 */
	s = spl6();
	if (bp->b_bufsize <= 0) {
		/* block has no buffer... put at front of unused buffer list */
		flist = &bfreelist[BQ_EMPTY];
		binsheadfree(bp, flist);
	} else if (bp->b_flags & (B_ERROR|B_INVAL)) {
		/* block has no info ... put at front of most free list */
		flist = &bfreelist[BQ_AGE];
		binsheadfree(bp, flist);
	} else {
		if (bp->b_flags & B_LOCKED)
			flist = &bfreelist[BQ_LOCKED];
		else if (bp->b_flags & B_AGE)
			flist = &bfreelist[BQ_AGE];
		else
			flist = &bfreelist[BQ_LRU];
		binstailfree(bp, flist);
	}
	bp->b_flags &= ~(B_WANTED|B_BUSY|B_ASYNC|B_AGE);
	splx(s);
}

/*
 * See if the block is associated with some buffer
 * (mainly to avoid getting hung up on a wait in breada)
 */
incore(dev, blkno, gp)
	dev_t dev;
	register daddr_t blkno;
	register struct gnode *gp;
{
	register struct buf *bp;
	register struct buf *dp;

	dp = BUFHASH(dev, blkno, gp);
	if(gp)
		gp = ((gp->g_mode & GFMT) == GFBLK) ? NULL : gp;
	for (bp = dp->b_forw; bp != dp; bp = bp->b_forw) {
		if ((bp->b_blkno != blkno) || (bp->b_dev != dev) ||
		    (bp->b_flags & B_INVAL) || nomatch(bp, gp))
			continue;
		return(1);
	}
	return (0);
}

struct buf *
baddr(dev, blkno, size, gp)
	dev_t dev;
	register daddr_t blkno;
	register int size;
	register struct gnode *gp;
{

	if (incore(dev, blkno, gp))
		return (bread(dev, blkno, size, gp));
	return (0);
}


/*
 * Assign a buffer for the given block.  If the appropriate
 * block is already associated, return it; otherwise search
 * for the oldest non-busy buffer and reassign it.
 *
 * We use splx here because this routine may be called
 * on the interrupt stack during a dump, and we don't
 * want to lower the ipl back to 0.
 */
struct buf *
getblk(dev, blkno, size, gp)
	dev_t dev;
	register daddr_t blkno;
	register int size;
	register struct gnode *gp;
{
	register struct buf *bp, *dp;
	register int s;

	gp = (gp && ((gp->g_mode & GFMT) == GFBLK)) ? NULL : gp;
	
	if ((unsigned)blkno >= 1 << (sizeof(int)*NBBY-PGSHIFT))	/* XXX */
		blkno = 1 << ((sizeof(int)*NBBY-PGSHIFT) + 1);
	/*
	 * Search the cache for the block.  If we hit, but
	 * the buffer is in use for i/o, then we wait until
	 * the i/o has completed.
	 */
	dp = BUFHASH(dev, blkno, gp);
loop:
	for (bp = dp->b_forw; bp != dp; bp = bp->b_forw) {
		if ((bp->b_blkno != blkno) || (bp->b_dev != dev) ||
		    (bp->b_flags&B_INVAL) || nomatch(bp, gp))
			continue;
		s = spl6();
		if (bp->b_flags&B_BUSY) {
			bp->b_flags |= B_WANTED;
			sleep((caddr_t)bp, PRIBIO+1);
			splx(s);
			goto loop;
		}
		splx(s);
		notavail(bp);
		/*
		 * optimize NOT calling brealloc since 96% of the time
		 * we return immediately (i.e., size is the same!!!)
		 */
		if (size != bp->b_bcount) {
			if (bp->b_flags & B_DELWRI) {
				bwrite(bp);
				goto loop;
			}
			if (brealloc(bp, size) == 0)
				goto loop;
		}

#ifdef GFSDEBUG
		if(GFS[19])
			cprintf("getblk: (hashed) bp 0x%x dev 0x%x gp 0x%x (%d)\n", bp,
			bp->b_dev, bp->b_gp, bp->b_gp ? bp->b_gp->g_number
			: -1);
#endif
		bp->b_flags |= B_CACHE;
		return(bp);
	}
#ifdef notdef
/* getpdev() creates pseudo devices with major = nblkdev+nchrdev+1 */
	if (major(dev) >= nblkdev)
		panic("blkdev");
#endif notdef
	bp = getnewbuf();
	bp->b_bcount = 0;
	bremhash(bp);
	binshash(bp, dp);
	bp->b_dev = dev;
	bp->b_blkno = blkno;
	bp->b_error = 0;
	bp->b_resid = 0;
	bp->b_gp = gp;
	if(gp)
		bp->b_gid = gp->g_id;
	if(brealloc(bp, size) == 0)
		goto loop;
#ifdef GFSDEBUG
	if(GFS[19])
		cprintf("getblk: bp 0x%x dev 0x%x gp 0x%x (%d)\n", bp,
		bp->b_dev, gp, gp ? gp->g_number : -1);
#endif
	return(bp);
}

/*
 * get an empty block,
 * not assigned to any particular device
 */
struct buf *
geteblk(size)
	register int size;
{
	register struct buf *bp, *flist;

#ifdef GFSDEBUG
	if(size == 0)
		panic("geteblk: zero length buffer");
#endif
loop:
	bp = getnewbuf();
	bp->b_flags |= B_INVAL;
	bp->b_bcount = 0;
	bremhash(bp);
	flist = &bfreelist[BQ_AGE];
	binshash(bp, flist);
	bp->b_dev = (dev_t)NODEV;
	bp->b_resid = 0;
	bp->b_error = 0;
	bp->b_gp = NULL;
	bp->b_gid = 0;
	if (brealloc(bp, size) == 0)
		goto loop;
#ifdef GFSDEBUG
	if(GFS[19])
		cprintf("geteblk: bp 0x%x dev 0x%x gp 0x%x\n", bp,
		bp->b_dev, bp->b_gp);
#endif
	return(bp);
}

/*
 * Allocate space associated with a buffer.
 * If can't get space, buffer is released
 */
brealloc(bp, size)
	register struct buf *bp;
	register int size;
{
	register daddr_t start, last;
	register struct buf *ep;
	register struct buf *dp;
	int s;
	struct gnode *gp;
	/*
	 * First need to make sure that all overlaping previous I/O
	 * is dispatched with.
	 */
	if (size == bp->b_bcount)
		return (1);
	if (size < bp->b_bcount) { 
		if (bp->b_flags & B_DELWRI) {
			bwrite(bp);
			return (0);
		}
		if (bp->b_flags & B_LOCKED)
			panic("brealloc");
		return (allocbuf(bp, size));
	}
	bp->b_flags &= ~B_DONE;
	if (bp->b_dev == NODEV)
		return (allocbuf(bp, size));

	/*
	 * Search cache for any buffers that overlap the one that we
	 * are trying to allocate. Overlapping buffers must be marked
	 * invalid, after being written out if they are dirty. (indicated
	 * by B_DELWRI) A disk block must be mapped by at most one buffer
	 * at any point in time. Care must be taken to avoid deadlocking
	 * when two buffer are trying to get the same set of disk blocks.
	 */
	start = bp->b_blkno;
	last = start + btodb(size) - 1;
	if(gp = bp->b_gp)
		gp = ((gp->g_mode & GFMT) == GFBLK) ? NULL : gp;	
	dp = BUFHASH(bp->b_dev, bp->b_blkno, gp);
loop:
	for (ep = dp->b_forw; ep != dp; ep = ep->b_forw) {
		if ((ep == bp) || (ep->b_dev != bp->b_dev) ||
		    (ep->b_flags&B_INVAL) || nomatch(ep, gp))
			continue;
		/* look for overlap */
		if (ep->b_bcount == 0 || ep->b_blkno > last ||
		    ep->b_blkno + btodb(ep->b_bcount) <= start)
			continue;
		s = spl6();
		if (ep->b_flags&B_BUSY) {
			ep->b_flags |= B_WANTED;
			sleep((caddr_t)ep, PRIBIO+1);
			splx(s);
			goto loop;
		}
		splx(s);
		notavail(ep);
		if (ep->b_flags & B_DELWRI) {
			bwrite(ep);
			goto loop;
		}
		ep->b_flags |= B_INVAL;
		brelse(ep);
	}
	return (allocbuf(bp, size));
}

/*
 * Find a buffer which is available for use.
 * Select something from a free list.
 * Preference is to AGE list, then LRU list.
 */
struct buf *
getnewbuf()
{
	register struct buf *bp, *dp;
	register int s;

loop:
	s = spl6();

	for (dp = &bfreelist[BQ_AGE]; dp > bfreelist; dp--)
		if (dp->av_forw != dp)
			break;
	if (dp == bfreelist) {		/* no free blocks */
		dp->b_flags |= B_WANTED;
		sleep((caddr_t)dp, PRIBIO+1);
		splx(s);
		goto loop;
	}
	splx(s);
	bp = dp->av_forw;
	notavail(bp);
	if (bp->b_flags & B_DELWRI) {
		bp->b_flags |= B_ASYNC;
		bwrite(bp);
		goto loop;
	}
	trace(TR_BRELSE, bp->b_dev, bp->b_blkno);
	bp->b_flags = B_BUSY;
	return (bp);
}

/*
 * Wait for I/O completion on the buffer; return errors
 * to the user.
 */
biowait(bp)
	register struct buf *bp;
{
	register int s;

	s = spl6();
	while ((bp->b_flags&B_DONE)==0)
		sleep((caddr_t)bp, PRIBIO);
	splx(s);
	if (u.u_error == 0)			/* XXX */
		u.u_error = geterror(bp);
}

/*
 * Mark I/O complete on a buffer.
 * If someone should be called, e.g. the pageout
 * daemon, do so.  Otherwise, wake up anyone
 * waiting for it.
 */
biodone(bp)
	register struct buf *bp;
{

	if (bp->b_flags & B_DONE)
		panic("dup biodone");
	bp->b_flags |= B_DONE;
	if (bp->b_flags & B_CALL) {
		bp->b_flags &= ~B_CALL;
		(*bp->b_iodone)(bp);
		return;
	}
	if (bp->b_flags&B_ASYNC)
		brelse(bp);
	else {
		bp->b_flags &= ~B_WANTED;
		wakeup((caddr_t)bp);
	}
}

/*
 * Insure that no part of a specified block is in an incore buffer.
 */
blkflush(dev, blkno, size, gp)
	dev_t dev;
	register daddr_t blkno;
	long size;
	register struct gnode *gp;	
{
	register struct buf *ep;
	register struct buf *dp;
	register daddr_t start, last;
	int s;
	
	start = blkno;
	last = start + btodb(size) - 1;
	dp = BUFHASH(dev, blkno, gp);
loop:
	for (ep = dp->b_forw; ep != dp; ep = ep->b_forw) {
		if ((ep->b_dev != dev) || (ep->b_flags & B_INVAL) ||
		     nomatch(ep, gp))
			continue;
		/* look for overlap */
		if (ep->b_bcount == 0 || ep->b_blkno > last ||
		    ep->b_blkno + btodb(ep->b_bcount) <= start)
			continue;
		s = spl6();
		if (ep->b_flags&B_BUSY) {
			ep->b_flags |= B_WANTED;
			sleep((caddr_t)ep, PRIBIO+1);
			splx(s);
			goto loop;
		}
		if (ep->b_flags & B_DELWRI) {
			splx(s);
			notavail(ep);
			bwrite(ep);
			goto loop;
		}
		splx(s);
	}
}

/*
 * Make sure all write-behind blocks
 * on dev (or NODEV for all)
 * are flushed out.
 * (from umount and update)
 */

bflush(dev)
	dev_t dev;
{
	register struct buf *bp;
	register struct buf *flist;
	register int s;
#ifdef GFSDEBUG
	if(GFS[9])
		cprintf("bflush: bfreelist 0x%x forw 0x%x back 0x%x\n",
		bfreelist, bfreelist[0].av_forw, bfreelist[0].av_back);
#endif
loop:
	s = spl6();
	for (flist = bfreelist; flist < &bfreelist[BQ_EMPTY]; flist++) {
#ifdef GFSDEBUG
		if(GFS[9])
			cprintf("bflush: flist 0x%x list %d\n", flist,
			&bfreelist[BQ_EMPTY] - flist);
#endif
		for (bp = flist->av_forw; bp != flist; bp = bp->av_forw) {
			if ((bp->b_flags & B_DELWRI) == 0) {
				continue;
			}				
			if (dev == NODEV || dev == bp->b_dev) {
#ifdef GFSDEBUG
				if(GFS[9])
					cprintf("bflush: bp 0x%x dev 0x%x\n",
					bp, bp->b_dev);
#endif

				bp->b_flags |= B_ASYNC;
				notavail(bp);
				bwrite(bp);
				splx(s);
				goto loop;
			}
		}
	}
	splx(s);
}

/*
 * bflushgp is just like bflush, but it limits its activity to blocks
 * associated with a particular gnode.  The two should be merged, for
 * now they are left separate to minimize interference with other code
 * that actually works.
 */

bflushgp(dev, gp)
	dev_t dev;
	struct gnode *gp;
{
	register struct buf *bp;
	register struct buf *flist;
	register int s;

#ifdef GFSDEBUG
	if(GFS[9])
		cprintf("bflushgp: bfreelist 0x%x forw 0x%x back 0x%x\n",
		bfreelist, bfreelist[0].av_forw, bfreelist[0].av_back);
#endif

loop:
	s = spl6();
	for (flist = bfreelist; flist < &bfreelist[BQ_EMPTY]; flist++) {

#ifdef GFSDEBUG
		if(GFS[9])
			cprintf("bflushgp: flist 0x%x list %d\n", flist,
			&bfreelist[BQ_EMPTY] - flist);
#endif

		for (bp = flist->av_forw; bp != flist; bp = bp->av_forw) {
			if ((bp->b_flags & B_DELWRI) == 0) {
				continue;
			}				
			if (nomatch(bp, gp)) {
				continue;
			}
			if (dev == NODEV || dev == bp->b_dev) {

#ifdef GFSDEBUG
				if(GFS[9])
					cprintf("bflushgp: bp 0x%x dev 0x%x gp 0x%x (%d)\n",
					bp, bp->b_dev, gp, gp ? gp->g_number : -1);
#endif

				bp->b_flags |= B_ASYNC;
				notavail(bp);
				bwrite(bp);
				splx(s);
				goto loop;
			}
		}
	}
	splx(s);
}

/*
 * flush all dirty buffers back to disk.  should only be done as an
 * end condition (i.e., halting the system)
 */

bhalt()
{
	register struct buf *bp;
	int s;

	s = spl6();
	for(bp = &buf[0]; bp < &buf[nbuf]; bp++) {
		if(bp->b_dev == NODEV)
			continue;
		if((bp->b_flags & B_BUSY) && ((bp->b_flags & B_READ) == 0))
		   	if((bp->b_flags & B_DELWRI) == 0) {
				if ((bp->b_flags & B_DONE) == 0)
					biowait(bp);
				continue;
			}
 		if (bp->b_flags & B_ASYNC || bp->b_flags & B_DELWRI) {
			bp->b_flags &= ~B_ASYNC;
			bwrite(bp);
		}
	}
	splx(s);
}

/*
 * Pick up the device's error number and pass it to the user;
 * if there is an error but the number is 0 set a generalized
 * code.  Actually the latter is always true because devices
 * don't yet return specific errors.
 */
geterror(bp)
	register struct buf *bp;
{
	register int error = 0;

	if (bp->b_flags&B_ERROR)
		if ((error = bp->b_error)==0)
			return (EIO);
	return (error);
}

/*
 * Invalidate in core blocks belonging to closed or umounted filesystem
 *
 * This is not nicely done at all - the buffer ought to be removed from the
 * hash chains & have its dev/blkno fields clobbered, but unfortunately we
 * can't do that here, as it is quite possible that the block is still
 * being used for i/o. Eventually, all disc drivers should be forced to
 * have a close routine, which ought ensure that the queue is empty, then
 * properly flush the queues. Until that happy day, this suffices for
 * correctness.						... kre
 */
binval(dev, gp)
	dev_t dev;
	register struct gnode *gp;
{
	register struct buf *bp;
	register struct bufhd *hp;
#define dp ((struct buf *)hp)

	for (hp = bufhash; hp < &bufhash[BUFHSZ]; hp++)
		for (bp = dp->b_forw; bp != dp; bp = bp->b_forw)
			if ((bp->b_dev == dev) || (gp && (gp == bp->b_gp)))
				bp->b_flags |= B_INVAL;
}
