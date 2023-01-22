/*	@(#)buf.h	1.12	(ULTRIX)	1/15/87	*/

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
 * 23 Oct 86 -- chet
 *	add IO_ASYNC, IO_SYNC, and IO_APPEND constants for GRWGP
 *
 * 11 Sep 86 -- koehler
 *	gfs change  align columns change comments for accuracy
 *
 * 11 Mar 86 -- lp
 *	Added flag for n-bufferring (B_RAWASYNC).
 *
 * 25 Apr 85 -- depp
 *	Removed SHMEM ifdefs
 *
 * 01 Mar 85 -- depp
 *	Added Shared memory definition for bproc pointer.
 *
 */

/*
 * The header for buffers in the buffer pool and otherwise used
 * to describe a block i/o request is given here.  The routines
 * which manipulate these things are given in bio.c.
 *
 * Each buffer in the pool is usually doubly linked into 2 lists:
 * hashed into a chain by <dev,blkno> so it can be located in the cache,
 * and (usually) on (one of several) queues.  These lists are circular and
 * doubly linked for easy removal.
 *
 * There are currently three queues for buffers:
 *	one for buffers which must be kept permanently (super blocks)
 * 	one for buffers containing ``useful'' information (the cache)
 *	one for buffers containing ``non-useful'' information
 *		(and empty buffers, pushed onto the front)
 * The latter two queues contain the buffers which are available for
 * reallocation, are kept in lru order.  When not on one of these queues,
 * the buffers are ``checked out'' to drivers which use the available list
 * pointers to keep track of them in their i/o active queues.
 */

/*
 * Bufhd structures used at the head of the hashed buffer queues.
 * We only need three words for these, so this abbreviated
 * definition saves some space.
 */
struct bufhd
{
	long	b_flags;		/* see defines below */
	struct	buf *b_forw, *b_back;	/* fwd/bkwd pointer in chain */
};
struct buf
{
	long	b_flags;		/* too much goes here to describe */
	struct	buf *b_forw, *b_back;	/* hash chain (2 way street) */
	struct	buf *av_forw, *av_back;	/* position on free list if not BUSY */
#define	b_actf	av_forw			/* alternate names for driver queue */
#define	b_actl	av_back			/*    head - isn't history wonderful */
	long	b_bcount;		/* transfer count */
	long	b_bufsize;		/* size of allocated buffer */
#define	b_active b_bcount		/* driver queue head: drive active */
	short	b_error;		/* returned after I/O */
	dev_t	b_dev;			/* major+minor device name */
	union {
	    caddr_t b_addr;		/* low order core address */
	    int	*b_words;		/* words for clearing */
	    struct fs *b_fs;		/* superblocks */
	    struct csum *b_cs;		/* superblock summary information */
	    struct cg *b_cg;		/* cylinder group block */
	    struct ufs_inode *b_dino;	/* ilist */
	    daddr_t *b_daddr;		/* indirect block */
	} b_un;
	daddr_t	b_blkno;		/* block # on device */
	long	b_resid;		/* words not transferred after error */
#define	b_errcnt b_resid		/* while i/o in progress: # retries */
	struct  proc *b_proc;		/* proc doing physical or swap I/O */
	struct	gnode *b_gp;		/* who owns this bp (used remotely)*/
	int	b_gid;			/* gnode id number */
	int	(*b_iodone)();		/* function called by iodone */
	int	b_pfcent;		/* center page when swapping cluster */
};

#define	BQUEUES		4		/* number of free buffer queues */

#define	BQ_LOCKED	0		/* super-blocks &c */
#define	BQ_LRU		1		/* lru, useful buffers */
#define	BQ_AGE		2		/* rubbish */
#define	BQ_EMPTY	3		/* buffer headers with no memory */

#ifdef	KERNEL
#define	BUFHSZ	512
#define RND	(MAXBSIZE/DEV_BSIZE)
#define BHASHARG(dev, gp) ((((gp) == NULL) || ISLOCAL((gp)->g_mp)) ? (int)(dev) : (int)(gp))
#if	((BUFHSZ&(BUFHSZ-1)) == 0)
#define	BUFHASH(dev, dblkno, gp)	\
	((struct buf *)&bufhash[(BHASHARG((dev), (gp)) + (((int)(dblkno))/RND)) & (BUFHSZ-1)])
#else
#define	BUFHASH(dev, dblkno, gp)	\
       ((struct buf *)&bufhash[(BHASHARG((dev), (gp)) + (((int)(dblkno))/RND)) % BUFHSZ])
#endif

struct	buf *buf;		/* the buffer pool itself */
char	*buffers;
int	nbuf;			/* number of buffer headers */
int	bufpages;		/* number of memory pages in the buffer pool */
struct	buf *swbuf;		/* swap I/O headers */
int	nswbuf;
struct	bufhd bufhash[BUFHSZ];	/* heads of hash lists */
struct	buf bfreelist[BQUEUES];	/* heads of available lists */
struct	buf bswlist;		/* head of free swap header list */
struct	buf *bclnlist;		/* head of cleaned page list */

struct	buf *alloc();
struct	buf *realloccg();
struct	buf *baddr();
struct	buf *getblk();
struct	buf *geteblk();
struct	buf *getnewbuf();
struct	buf *bread();
struct	buf *breada();

unsigned minphys();
#endif

/*
 * These flags are kept in b_flags.
 */
#define	B_WRITE		0x00000000	/* non-read pseudo-flag */
#define	B_READ		0x00000001	/* read when I/O occurs */
#define	B_DONE		0x00000002	/* transaction finished */
#define	B_ERROR		0x00000004	/* transaction aborted */
#define	B_BUSY		0x00000008	/* not on av_forw/back list */
#define	B_PHYS		0x00000010	/* physical IO */
#define	B_XXX		0x00000020	/* was B_MAP, alloc UNIBUS on pdp-11 */
#define	B_WANTED	0x00000040	/* issue wakeup when BUSY goes off */
#define	B_AGE		0x00000080	/* delayed write for correct aging */
#define	B_ASYNC		0x00000100	/* don't wait for I/O completion */
#define	B_DELWRI	0x00000200	/* write at exit of avail list */
#define	B_TAPE		0x00000400	/* this is a magtape (no bdwrite) */
#define	B_UAREA		0x00000800	/* add u-area to a swap operation */
#define	B_PAGET		0x00001000	/* page in/out of page table space */
#define	B_DIRTY		0x00002000	/* dirty page to be pushed out async */
#define	B_PGIN		0x00004000	/* pagein op, so swap() can count it */
#define	B_CACHE		0x00008000	/* did bread find us in the cache ? */
#define	B_INVAL		0x00010000	/* does not contain valid info  */
#define	B_LOCKED	0x00020000	/* locked in core (not reusable) */
#define	B_HEAD		0x00040000	/* a buffer header, not a buffer */
#define	B_BAD		0x00100000	/* bad block revectoring in progress */
#define	B_CALL		0x00200000	/* call b_iodone from iodone */
#define B_SMEM		0x00400000	/* b_proc is a ptr to "smem" */
#define B_RAWASYNC	0x00800000	/* buffer involved in raw async I/O */

/*
 * Sync, Async, and Append flags for GRWGP
 */
#define IO_ASYNC	0x0
#define IO_SYNC		0x1
#define IO_APPEND	0x2

/*
 * Insq/Remq for the buffer hash lists.
 */
#define	bremhash(bp) { \
	(bp)->b_back->b_forw = (bp)->b_forw; \
	(bp)->b_forw->b_back = (bp)->b_back; \
}
#define	binshash(bp, dp) { \
	(bp)->b_forw = (dp)->b_forw; \
	(bp)->b_back = (dp); \
	(dp)->b_forw->b_back = (bp); \
	(dp)->b_forw = (bp); \
}

/*
 * Insq/Remq for the buffer free lists.
 */
#define	bremfree(bp) { \
	(bp)->av_back->av_forw = (bp)->av_forw; \
	(bp)->av_forw->av_back = (bp)->av_back; \
}
#define	binsheadfree(bp, dp) { \
	(dp)->av_forw->av_back = (bp); \
	(bp)->av_forw = (dp)->av_forw; \
	(dp)->av_forw = (bp); \
	(bp)->av_back = (dp); \
}
#define	binstailfree(bp, dp) { \
	(dp)->av_back->av_forw = (bp); \
	(bp)->av_back = (dp)->av_back; \
	(dp)->av_back = (bp); \
	(bp)->av_forw = (dp); \
}

/*
 * Take a buffer off the free list it's on and
 * mark it as being use (B_BUSY) by a device.
 */
#define	notavail(bp) { \
	int x = spl6(); \
	bremfree(bp); \
	(bp)->b_flags |= B_BUSY; \
	splx(x); \
}

#define	iodone	biodone
#define	iowait	biowait

/*
 * Zero out a buffer's data portion.
 */
#define	clrbuf(bp) { \
	blkclr(bp->b_un.b_addr, bp->b_bcount); \
	bp->b_resid = 0; \
}

/*
 * Return true if a buffer is not associated with a particular gnode.
 */
#define nomatch(bp, gp) ((gp) && (bp)->b_gp && \
	(((bp)->b_gp != (gp)) || ((bp)->b_gid != (gp)->g_id)))
