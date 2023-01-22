#ifndef lint
static	char	*sccsid = "@(#)ufs_gnode.c	1.11	(ULTRIX)	1/29/87";
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

/* ---------------------------------------------------------------------
 * Modification History: /sys/sys/ufs_inode.c
 *
 * 29 Jan 87 -- chet
 *	add new arg to bdwrite() calls.
 *
 * 03 Nov 86 -- bglover for koehler
 *	change bread call indirtrunc rtn (final parameter NULL)
 *
 * 11 Sep 86 -- koehler
 *	gfs changes -- check locking and do synchronous fs writes
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 * 15 Mar 85 -- funding
 *	Added named pipe support (re. System V named pipes)
 *
 * 16 Oct 84 -- jrs
 *	Fix hash, quota release, add igrab and nami cache support
 *
 * 17 Jul 84 -- jmcg
 *	Inserted code to track lockers and unlockers of inodes for
 *	debugging.  Conditionally compiled by defining RECINODELOCKS.
 *	Changed a blind lock to an GLOCK (based on suspicions of a
 *	bug from the net).
 *
 * 17 Jul 84 --jmcg
 *	Began modification history with sources from ULTRIX-32, Rel. V1.0.
 *	Derived from 4.2BSD, labeled:
 *		ufs_inode.c	6.1	83/07/29
 *
 * ---------------------------------------------------------------------
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mount.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/gnode_common.h"
#include "../ufs/ufs_inode.h"
#include "../h/gnode.h"
#include "../ufs/fs.h"
#include "../h/conf.h"
#include "../h/buf.h"
#ifdef QUOTA
#include "../h/quota.h"
#endif
#include "../h/kernel.h"
#include "../h/fs_types.h"

#ifdef GFSDEBUG
extern short GFS[];
#endif

/*
 * Look up an gnode by device,inumber.
 * If it is in core (in the gnode structure),
 * honor the locking protocol.
 * If it is not in core, read it in from the
 * specified device.
 * If the gnode is mounted on, perform
 * the indicated indirection.
 * In all cases, a pointer to a locked
 * gnode structure is returned.
 *
 * panic: no imt -- if the mounted file
 *	system is not in the mount table.
 *	"cannot happen"
 */
struct gnode *
ufs_gget(dev, mpp, gno, flag)
	dev_t dev;
	register struct mount *mpp;
	gno_t gno;
	int flag;
{
	register struct gnode *gp = (struct gnode *) 0;
	register union  ghead *ih;
	register struct buf *bp;
	register struct ufs_inode *dp;
	register struct	fs *fs;

loop:
#ifdef GFSDEBUG
	if(GFS[5])
		cprintf("ufs_gget: dev 0x%x mp 0x%x, gno %d flag 0%o\n", dev,
		mpp, gno, flag);
#endif
	/*
	 * check to see if we are leaving the fs type 
	 */

	if(gp && (mpp->m_fstype != GT_ULTRIX)) {
		gp = mpp->m_rootgp;
		
		/*
		 * it is necessary to increment the count and try
		 * to lock the gnode since that is the way sfs_namei
		 * expects it...
		 */
		
		gp->g_count++;
		gfs_lock(gp);
		gp->g_flag |= GINCOMPLETE;
#ifdef GFSDEBUG
		if(GFS[5])
			cprintf("ufs_gget: gp 0x%x (%d) transitioning\n",
			gp, gp->g_number);
#endif
		return(gp);
	}

	ih = &ghead[GNOHASH(dev, gno)];

	for (gp = ih->gh_chain[0]; gp != (struct gnode *)ih; gp = gp->g_forw) {
		if (gno == gp->g_number && dev == gp->g_dev) {
			/*
			 * Following is essentially an inline expanded
			 * copy of igrab(), expanded inline for speed,
			 * and so that the test for a mounted on gnode
			 * can be deferred until after we are sure that
			 * the gnode isn't busy.
			 */
#ifdef GFSDEBUG
			if(GFS[5])
				cprintf("ufs_gget: match\n");
#endif
			if ((gp->g_flag&GLOCKED) != 0) {
				gp->g_flag |= GWANT;
				sleep((caddr_t)gp, PINOD);
				goto loop;
			}
#ifdef GFSDEBUG
			if(GFS[5])
				cprintf ("ufs_gget: gp 0x%x (%d) flags 0%o\n",
				gp, gp->g_number, gp->g_flag);
#endif
			if (((gp->g_flag&GMOUNT) != 0) && (flag == 0)) {
#ifdef GFSDEBUG
				if(GFS[5])
					cprintf ("ufs_gget: hit mount point\n");

#endif
#ifdef GFSDEBUG
				if(GFS[5])
					cprintf("ufs_gget: match mp 0x%x dev 0x%x\n", mpp, mpp->m_dev);
#endif
				mpp = gp->g_mpp;
				dev = mpp->m_dev;
				gno = ROOTINO;
				goto loop;
			}
			
			if (gp->g_count == 0)
				gremque(gp);

			gp->g_count++;
			gp->g_flag |= GLOCKED;
			gp->g_flag &= ~GINCOMPLETE;
			gp->g_blocks = G_TO_I(gp)->di_blocks;
#ifdef GFSDEBUG
			if(GFS[5] && flag)
				cprintf("ufs_gget: gp 0x%x (%d) flags 0%o\n",
				gp, gp->g_number, gp->g_flag);
#endif
			return(gp);
		}
	}
	if((gp = getegnode(GNOHASH(dev, gno), dev, gno)) == NULL) {
		uprintf("out of gnodes\n");
		return(NULL);
	}
	gp->g_flag = GLOCKED;
	gp->g_count++;
	gp->g_lastr = 0;
	gp->g_mp = mpp;	
	fs = mpp->m_bufp->b_un.b_fs;
#ifdef QUOTA
	dqrele(gp->g_dquot);
#endif
#ifdef GFSDEBUG
	if(GFS[5])
		cprintf ("ufs_gget: dev 0x%x, fs 0x%x, gno %d\n", dev, fs, gno);
#endif
	bp = bread(dev, fsbtodb(fs, itod(fs, gno)), (int)fs->fs_bsize,
	(struct gnode *) NULL);
	/*
	 * Check I/O errors
	 */
	if ((bp->b_flags&B_ERROR) != 0) {
		brelse(bp);
		/*
		 * the gnode doesn't contain anything useful, so it would
		 * be misleading to leave it on its hash chain.
		 * 'gput' will take care of putting it back on the free list.
		 */
		remque(gp);
		gp->g_forw = gp;
		gp->g_back = gp;
		/*
		 * we also loose its inumber, just in case (as gput
		 * doesn't do that any more) - but as it isn't on its
		 * hash chain, I doubt if this is really necessary .. kre
		 * (probably the two methods are interchangable)
		 */
		gp->g_number = 0;
#ifdef QUOTA
		gp->g_dquot = NODQUOT;
#endif
		gput(gp);
		return(NULL);
	}
	dp = bp->b_un.b_dino;
	dp += itoo(fs, gno);
	G_TO_I(gp)->di_ic = dp->di_ic;
	gp->g_blocks = G_TO_I(gp)->di_blocks;
	gp->g_gennum = G_TO_I(gp)->di_gennum;
#ifdef GFSDEBUG
	if(GFS[5])
		cprintf("ufs_gget: g blocks %d di blocks %d mp 0x%x\n",
		gp->g_blocks, dp->di_blocks, gp->g_mp);
#endif
	brelse(bp);
#ifdef QUOTA
	if (gp->g_mode == 0)
		gp->g_dquot = NODQUOT;
	else 
		gp->g_dquot = inoquota(gp);

#endif
	gp->g_flag &= ~GINCOMPLETE;
	return (gp);
}

ufs_grele(gp)
	register struct gnode *gp;
{
	register int mode;

#ifdef GFSDEBUG
	if(GFS[10]) {
		cprintf("ufs_grele gp 0x%x (%d)\n", gp, gp->g_number);
	}
#endif
	mode = gp->g_mode & GFMT;
	if (gp->g_count == 1) {
		UFS_GLOCK(gp);
		if (gp->g_nlink <= 0) {
#ifdef GFSDEBUG
			if(GFS[10]) {
				char *ft;
				
				switch(mode) {
					case GFBLK:
						ft = "BLOCK";
						break;
					case GFCHR:
						ft = "CHAR";
						break;
					case GFREG:
						ft = "REG";
						break;
					default:
						ft = "UNKNOWN";
					}
				cprintf("ufs_grele: freeing gp 0x%x\n", gp);
				cprintf("ufs_grele: file type is %s\n", ft);
			}
#endif
			if((mode == GFBLK) || (mode == GFCHR))
				gp->g_rdev = G_TO_I(gp)->di_rdev = 0;
			gp->g_nlink = 0;				
			gp->g_mode = 0;
			G_TO_I(gp)->di_gennum = gp->g_gennum;
			/*
			 * gtrunc does the gupdat 
			 */
			ufs_gtrunc(gp, (u_long)0, (struct ucred *) 0);
			ufs_gfree(gp, gp->g_number, mode);
#ifdef QUOTA
			(void) chkiq(gp->g_dev, gp, gp->g_uid, 0);
			dqrele(gp->g_dquot);
			gp->g_dquot = NODQUOT;
#endif
		}
		if(mode == GFPORT){
				/* the return from SOCLOSE	*/
				/* should be checked but I	*/
				/* don't know what to do with	*/
				/* it if it is bad!		*/
			if(gp->g_rso != 0){
				soclose(gp->g_rso);
				gp->g_rso = 0;
			}
			if(gp->g_wso != 0){
				soclose(gp->g_wso);
				gp->g_wso = 0;
			}
			gp->g_size = 0;
		}
		ufs_gupdat(gp, &time, &time, 0, (struct ucred *) 0);
		ufs_gunlock(gp);
		gp->g_flag = 0;
		/*
		 * Put the gnode on the end of the free list.
		 * Possibly in some cases it would be better to
		 * put the gnode at the head of the free list,
		 * (eg: where g_mode == 0 || g_number == 0)
		 * but I will think about that later .. kre
		 * (g_number is rarely 0 - only after an i/o error in ufs_gget,
		 * where g_mode == 0, the gnode will probably be wanted
		 * again soon for an ialloc, so possibly we should keep it)
		 */
		gp->g_count--;
		freegnode(gp);
		return;
	} else if (!(gp->g_flag & GLOCKED))
		ufs_gtimes(gp, &time, &time);
	if((gp->g_count--) < 1) {
		cprintf("ufs_grele: gp 0x%x (%d)\n", gp, gp->g_number);
		panic("ufs_grele: gp count bad");
	}
}

#define	SINGLE	0	/* index of single indirect block */
#define	DOUBLE	1	/* index of double indirect block */
#define	TRIPLE	2	/* index of triple indirect block */
/*
 * Truncate the gnode gp to at most
 * length size.  Free affected disk
 * blocks -- the blocks of the file
 * are removed in reverse order.
 *
 * NB: triple indirect blocks are untested.
 */
ufs_gtrunc(ogp, length, cred)
	register struct gnode *ogp;
	u_long length;
	struct ucred *cred;
{
	register i;
	register daddr_t lastblock;
	register daddr_t bn;
	register struct fs *fs;
	register struct ufs_inode *ufs_ip;
	daddr_t lastiblock[NIADDR];
	struct gnode tip;
	long blocksreleased = 0, nblocks;
	long indirtrunc();
	int level;
	
	if (ogp->g_size <= length) {
		ogp->g_flag |= GCHG|GUPD;
 		ufs_gupdat(ogp, &time, &time, 0, (struct ucred *) 0);
		return;
	}
	/*
	 * Calculate index into gnode's block list of
	 * last direct and indirect blocks (if any)
	 * which we want to keep.  Lastblock is -1 when
	 * the file is truncated to 0.
	 */
	fs = FS(ogp);
	lastblock = lblkno(fs, length + fs->fs_bsize - 1) - 1;
	lastiblock[SINGLE] = lastblock - NDADDR;
	lastiblock[DOUBLE] = lastiblock[SINGLE] - NINDIR(fs);
	lastiblock[TRIPLE] = lastiblock[DOUBLE] - NINDIR(fs) * NINDIR(fs);
	nblocks = btodb(fs->fs_bsize);
#ifdef GFSDEBUG
	if(GFS[2]) {
		cprintf("ufs_gtrunc: lastblock %d nblocks %d\n", lastblock,
		nblocks);
	}
#endif
	/*
	 * Update size of file and block pointers
	 * on disk before we start freeing blocks.
	 * If we crash before free'ing blocks below,
	 * the blocks will be returned to the free list.
	 * lastiblock values are also normalized to -1
	 * for calls to indirtrunc below.
	 * (? fsck doesn't check validity of pointers in indirect blocks)
	 */
#ifdef GFSDEBUG
	if(GFS[2]) {
		cprintf("ufs_gtrunc: direct blocks ");
		for(i = 0; i < NDADDR; i++)
			cprintf("%d ", G_TO_I(ogp)->di_db[i]);
		cprintf("\nufs_gtrunc: indirect blocks ");
		for (i = SINGLE + 1; i <= TRIPLE; i++)
			cprintf("%d ", G_TO_I(ogp)->di_ib[i]);
		cprintf("\n");
	}
#endif
	bcopy(ogp, &tip, sizeof(struct gnode));
	
	ufs_ip = G_TO_I(&tip);
	for (level = TRIPLE; level >= SINGLE; level--)
		if (lastiblock[level] < 0) {
			G_TO_I(ogp)->di_ib[level] = 0;
			lastiblock[level] = -1;
		}
	for (i = NDADDR - 1; i > lastblock; i--) {
		G_TO_I(ogp)->di_db[i] = 0;
	}
	
	ogp->g_size = length;
	ogp->g_flag |= GCHG|GUPD;
	ufs_gupdat(ogp, &time, &time, 1, (struct ucred *) 0);

	/*
	 * Indirect blocks first.
	 */
	
	for (level = TRIPLE; level >= SINGLE; level--) {
		bn = ufs_ip->di_ib[level];
		if (bn != 0) {
#ifdef GFSDEBUG
			if(GFS[2])
				cprintf("ufs_gtrunc: freeing ibn %d\n", bn);
#endif
			blocksreleased +=
			indirtrunc(&tip, bn, lastiblock[level], level);
			if (lastiblock[level] < 0) {
				ufs_ip->di_ib[level] = 0;
				free(&tip, bn, (off_t)fs->fs_bsize);
				blocksreleased += nblocks;
			}
		}
		if (lastiblock[level] >= 0)
			goto done;
	}

	/*
	 * All whole direct blocks or frags.
	 */
	for (i = NDADDR - 1; i > lastblock; i--) {
		register int size;

		bn = ufs_ip->di_db[i];
		if (bn == 0)
			continue;
#ifdef GFSDEBUG
		if(GFS[2])
			cprintf("ufs_gtrunc: freeing dbn %d\n", bn);
#endif
		ufs_ip->di_db[i] = 0;
		size = (off_t)blksize(fs, &tip, i);
		free(&tip, bn, size);
		blocksreleased += btodb(size);
	}
	if (lastblock < 0)
		goto done;

	/*
	 * Finally, look for a change in size of the
	 * last direct block; release any frags.
	 */
	bn = ufs_ip->di_db[lastblock];
	if (bn != 0) {
		int oldspace, newspace;

		/*
		 * Calculate amount of space we're giving
		 * back as old block size minus new block size.
		 */
#ifdef GFSDEBUG
		if(GFS[2])
			cprintf("ufs_gtrunc: freeing dbn %d\n", bn);
#endif
		oldspace = blksize(fs, &tip, lastblock);
		tip.g_size = length;
		newspace = blksize(fs, &tip, lastblock);
		if (newspace == 0)
			panic("ufs_gtrunc: newspace");
		if (oldspace - newspace > 0) {
			/*
			 * Block number of space to be free'd is
			 * the old block # plus the number of frags
			 * required for the storage we're keeping.
			 */
			bn += numfrags(fs, newspace);
			free(&tip, bn, oldspace - newspace);
			blocksreleased += btodb(oldspace - newspace);
		}
	}
done:
/* BEGIN PARANOIA */
	for (level = SINGLE; level <= TRIPLE; level++)
		if (ufs_ip->di_ib[level] != G_TO_I(ogp)->di_ib[level])
			panic("ufs_gtrunc1");
	for (i = 0; i < NDADDR; i++)
		if (ufs_ip->di_db[i] != G_TO_I(ogp)->di_db[i])
			panic("ufs_gtrunc2");
/* END PARANOIA */
	ogp->g_blocks -= blocksreleased;
	G_TO_I(ogp)->di_blocks = ogp->g_blocks;
	if (G_TO_I(ogp)->di_blocks < 0) {		/* sanity */
		ogp->g_blocks = G_TO_I(ogp)->di_blocks = 0;
	}
	
	ogp->g_flag |= GCHG;
#ifdef QUOTA
	(void) chkdq(ogp, -blocksreleased, 0);
#endif
}

/*
 * Release blocks associated with the gnode gp and
 * stored in the indirect block bn.  Blocks are free'd
 * in LIFO order up to (but not including) lastbn.  If
 * level is greater than SINGLE, the block is an indirect
 * block and recursive calls to indirtrunc must be used to
 * cleanse other indirect blocks.
 *
 * NB: triple indirect blocks are untested.
 */
long
indirtrunc(gp, bn, lastbn, level)
	register struct gnode *gp;
	daddr_t bn, lastbn;
	int level;
{
	register int i;
	struct buf *bp, *copy;
	register daddr_t *bap;
	register struct fs *fs = FS(gp);
	register daddr_t nb;
	daddr_t last;
	register long factor;
	int blocksreleased = 0, nblocks;

	/*
	 * Calculate index in current block of last
	 * block to be kept.  -1 indicates the entire
	 * block so we need not calculate the index.
	 */
	factor = 1;
/*
	printf("indirtrunc: fs 0x%x\n", fs);	
*/
	for (i = SINGLE; i < level; i++)
		factor *= NINDIR(fs);
	last = lastbn;
	if (lastbn > 0)
		last /= factor;
	nblocks = btodb(fs->fs_bsize);
	/*
	 * Get buffer of block pointers, zero those 
	 * entries corresponding to blocks to be free'd,
	 * and update on disk copy first.
	 */
	copy = geteblk((int)fs->fs_bsize);
	bp = bread(gp->g_dev, fsbtodb(fs, bn), (int)fs->fs_bsize, 
		(struct gnode *) NULL);
	if (bp->b_flags&B_ERROR) {
		brelse(copy);
		brelse(bp);
		return (0);
	}
	bap = bp->b_un.b_daddr;
	bcopy((caddr_t)bap, (caddr_t)copy->b_un.b_daddr, (u_int)fs->fs_bsize);
	bzero((caddr_t)&bap[last + 1],
	  (u_int)(NINDIR(fs) - (last + 1)) * sizeof (daddr_t));
	bwrite(bp);
	bp = copy, bap = bp->b_un.b_daddr;

	/*
	 * Recursively free totally unused blocks.
	 */
	for (i = NINDIR(fs) - 1; i > last; i--) {
		nb = bap[i];
		if (nb == 0)
			continue;
		if (level > SINGLE)
			blocksreleased +=
			    indirtrunc(gp, nb, (daddr_t)-1, level - 1);
		free(gp, nb, (int)fs->fs_bsize);
		blocksreleased += nblocks;
	}

	/*
	 * Recursively free last partial block.
	 */
	if (level > SINGLE && lastbn >= 0) {
		last = lastbn % factor;
		nb = bap[i];
		if (nb != 0)
			blocksreleased += indirtrunc(gp, nb, last, level - 1);
	}
	brelse(bp);
	return (blocksreleased);
}

/*
 * Lock an gnode. If its already locked, set the WANT bit and sleep.
 */
ufs_glock(gp)
	register struct gnode *gp;
{
	if(gp->g_mp->m_fstype != GT_ULTRIX) {
		printf("ufs_glock: gp 0x%x type %d\n", gp,
		gp->g_mp->m_fstype);
		panic("ufs_glock: gp type not GT_ULTRIX");
	}
#ifdef GFSDEBUG
	if(GFS[4])
		cprintf ("ufs_glock: locking gp 0x%x (%d)\n", gp, gp->g_number);
#endif
	UFS_GLOCK(gp);
}

/*
 * Unlock an gnode.  If WANT bit is on, wakeup.
 */
ufs_gunlock(gp)
	register struct gnode *gp;
{
#ifdef GFSDEBUG
	if(GFS[4])
		cprintf("ufs_gunlock: unlocking gp 0x%x (%d)\n", gp, gp->g_number);
#endif
	if(!(gp->g_flag & GLOCKED)) {
		cprintf("ufs_gunlock: gp unlocked, dev 0x%x gno %d\n",
		gp->g_dev, gp->g_number);
		panic("ufs_gunlock");
	}
	if(gp->g_mp->m_fstype != GT_ULTRIX) {
		printf("ufs_gulock: gp 0x%x type %d\n", gp,
		gp->g_mp->m_fstype);
		panic("ufs_gulock: gp type not GT_ULTRIX");
	}
	UFS_GUNLOCK(gp);
}

/*
 * Check accessed and update flags on
 * an gnode structure.
 * If any is on, update the gnode
 * with the current time.
 * If waitfor is given, then must insure
 * i/o order so wait for write to complete.
 */
ufs_gupdat(gp, ta, tm, waitfor, cred)
	register struct gnode *gp;
	register struct timeval *ta, *tm;
	int waitfor;
	struct ucred *cred;
{
	register struct buf *bp;
	register struct ufs_inode *dp;
	register struct fs *fs;

		
	fs = FS(gp);
	G_TO_I(gp)->di_blocks = gp->g_blocks;
	if(((gp->g_mode & GFMT) == GFCHR) || ((gp->g_mode & GFMT) == GFBLK))
		G_TO_I(gp)->di_rdev = gp->g_rdev;

#ifdef GFSDEBUG
	if(GFS[2]) 
		cprintf("ufs_gupdat: updating gnode 0x%x (%d) mode 0%o rdev 0x%x di_rdev 0x%x\n",
		gp, gp->g_number, gp->g_mode, gp->g_rdev, G_TO_I(gp)->di_rdev);
	if((((gp->g_mode & GFMT) == GFCHR) || ((gp->g_mode & GFMT) == GFBLK))
	&& (gp->g_nlink <= 0) && ((gp->g_rdev != 0) || (G_TO_I(gp)->di_rdev
	!= 0))) {
		gp->g_rdev = G_TO_I(gp)->di_rdev = 0;
		cprintf("dufus plopola\n");
	}
#endif
	if ((gp->g_flag & (GUPD|GACC|GCHG|GMOD)) != 0) {
		if (fs->fs_ronly)
			return(EROFS);
		bp = bread(gp->g_dev, fsbtodb(fs, itod(fs, gp->g_number)),
			(int)fs->fs_bsize, (struct gnode *) NULL);
			
		if (bp->b_flags & B_ERROR) {
			brelse(bp);
			return(EIO);
		}
		if (gp->g_flag & GACC) {
			gp->g_atime.tv_sec = ta->tv_sec;
			gp->g_atime.tv_usec = ta->tv_usec;
		}
		if (gp->g_flag & GUPD) {
			gp->g_mtime.tv_sec = tm->tv_sec;
			gp->g_mtime.tv_usec = tm->tv_usec;
		}
		if (gp->g_flag & GCHG) {
			gp->g_ctime.tv_sec = time.tv_sec;
			gp->g_ctime.tv_usec = time.tv_usec;
		}
		
		gp->g_flag &= ~(GUPD|GACC|GCHG|GMOD);
		dp = bp->b_un.b_dino + itoo(fs, gp->g_number);
		bcopy(&G_TO_I(gp)->di_ic, &dp->di_ic, sizeof(struct ufs_inode));
		if (waitfor)
			bwrite(bp);
		else {
			if(gp->g_mp->m_flags & M_SYNC)
				bwrite(bp);
			else
				bdwrite(bp, (struct gnode *)0);
		}
	}
	return(u.u_error);
}


ufs_gtimes(gp, t1, t2)
	register struct gnode *gp;
	register struct timeval *t1;
	register struct timeval *t2;
{
	UFS_GTIMES(gp, t1, t2);
}
