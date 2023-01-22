/* 	@(#)gnode.h	1.8	(ULTRIX)	3/3/87 	*/

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
 * Modification History: /sys/h/inode.h
 *
 * 20 Feb 87 -- depp
 *	Added GTRC flag for indicating that one or more processes are
 *	tracing.
 *
 * 11 Sep 86 -- koehler 
 *	added flags for update change
 *
 * 11 Mar 86 -- lp
 *	Add flag to mark an inode as using n-bufferring. Actually just
 *	reuse ISYNC flag to mean this (since ISYNC is never used to a
 *	raw device anyway - done this way because flags is a short and
 *	not much room left).
 *
 * 24 Dec 85 -- Shaughnessy
 *	Added syncronous write flag.
 *
 * 09 Sep 85 -- Reilly
 *	Modified to handle the new 4.3BSD namei code.
 * 
 * 19 Jul 85 -- depp
 *	Removed #ifdef NPIPE.  
 *
 * 4  April 85 -- Larry Cohen
 *	Add IINUSE flag to support open block if in use capability
 *
 * 15 Mar 85 -- funding
 *	Added named pipe support (re. System V named pipes)
 *
 * 23 Oct 84 -- jrs
 *	Add definitions for nami cacheing
 *
 * 17 Jul 84 -- jmcg
 *	Insert code to keep track of lockers and unlockers as a debugging
 *	aid.  Conditionally compiled with option RECINODELOCKS.
 *
 * 17 Jul 84 --jmcg
 *	Derived from 4.2BSD, labeled:
 *		inode.h	6.1	83/07/29
 *
 * ---------------------------------------------------------------------
 */


/*
 *	Any specific file systems that are to be include must
 *	include the definition of gnode_common within their defintion
 *	of the file system specific stuff.  The structure must be the
 *	first item within the definition since g<fs will use fields
 *	from gnode_common irrespective of fs type. This is the only way I
 *	can think of to keep from adding two copies of gnode_common
 *	around since the common stuff is in the on-disk format for
 *	an ufs inode.
 */

#ifndef __GNODE__
#define __GNODE__
#include "../h/gnode_common.h"
#define PADLEN 128
#define G_FREEBYTES PADLEN - sizeof(struct gnode_common)

struct gnode {
	struct	gnode_req {
		struct	gnode *gr_chain[2];	/* must be first */
		u_long	gr_flag;
		u_short	gr_count;	/* reference count */
		dev_t	gr_dev;		/* device where gnode resides */
		dev_t	gr_rdev;	/* for special files */
		u_short	gr_shlockc;	/* count of shared locks on gnode */
		u_short	gr_exlockc;	/* count of exclusive locks on gnode */
		gno_t	gr_number;    /* i number, 1-to-1 with device address */
		long	gr_id;		/* unique identifier */
		struct	mount *gr_mp;	/* where my mount structure is */
		struct	dquot *gr_dquot; /* hope this can stay ?? */
		u_long	gr_blocks;	/* added to aid matt */
		u_long	gr_gennum;	/* incarnation of the gnode */
		union {
			daddr_t	gf_lastr;	/* last read (read-ahead) */
			struct {
			    	int pad;	/* may need read-ahead */
			    	struct text	*gf_text;
			} g_txt;
			struct	socket *gs_socket;
			struct	{
				int pad;	/* need read-ahead for nfs */
				struct gnode  *gf_freef; /* free list forward */
				struct gnode  **gf_freeb;/* free list back */
			} g_fr;
			struct {
				struct socket *gs_rso;	/* port read socket */
				struct socket *gs_wso;	/* port write socket */
				u_short gs_freadcount;
				u_short gs_fwritecount;
			} g_so;
			struct {
				int pad; /* may need readahead */
				struct mount *gm_mp; /* this is a mount point */
			} g_pmp;
		} gr_un;
	} g_req;
	union {
		char pad[PADLEN];	/* 128 - should be a sizeof */
		struct gnode_common gn;	/* to make the defines easier */
		struct {
			struct gnode_common _x;
			char *free;
		} _freespace;
	} g_in;
};


#define	g_chain	g_req.gr_chain
#define	g_flag	g_req.gr_flag
#define	g_count	g_req.gr_count
#define	g_dev	g_req.gr_dev
#define g_rdev	g_req.gr_rdev		/* until something better comes up*/
#define g_gennum	g_req.gr_gennum
#define g_blocks	g_req.gr_blocks /* see above */
#define	g_shlockc	g_req.gr_shlockc
#define	g_exlockc	g_req.gr_exlockc
#define	g_number	g_req.gr_number
#define	g_id		g_req.gr_id
#define	g_mp		g_req.gr_mp
#define	g_dquot		g_req.gr_dquot
#define	g_mode		g_in.gn.gc_mode
#define	g_nlink		g_in.gn.gc_nlink
#define	g_uid		g_in.gn.gc_uid
#define	g_gid		g_in.gn.gc_gid
#define g_freespace	g_in._freespace.free
/* ugh! -- must be fixed */
#ifdef vax
#define	g_size		g_in.gn.gc_size.val[0]
#endif
#define	g_atime		g_in.gn.gc_atime
#define	g_mtime		g_in.gn.gc_mtime
#define	g_ctime		g_in.gn.gc_ctime
#define	g_lastr		g_req.gr_un.gf_lastr
#define	g_socket	g_req.gr_un.gs_socket
#define	g_forw		g_chain[0]
#define	g_back		g_chain[1]
#define	g_freef		g_req.gr_un.g_fr.gf_freef
#define	g_freeb		g_req.gr_un.g_fr.gf_freeb
#define	g_rso		g_req.gr_un.g_so.gs_rso
#define	g_wso		g_req.gr_un.g_so.gs_wso
#define	g_frcnt		g_req.gr_un.g_so.gs_freadcount
#define	g_fwcnt		g_req.gr_un.g_so.gs_fwritecount
#define g_mpp		g_req.gr_un.g_pmp.gm_mp
#define g_textp		g_req.gr_un.g_txt.gf_text
/* flags */

/* g_flag is now a u_long */

#define	GLOCKED		0x00000001	/* gnode is locked */
#define	GUPD		0x00000002	/* file has been modified */
#define	GACC		0x00000004	/* gnode access time to be updated */
#define	GMOUNT		0x00000008	/* gnode is mounted on */
#define	GWANT		0x00000010	/* some process waiting on lock */
#define	GTEXT		0x00000020	/* gnode is pure text prototype */
#define	GCHG		0x00000040	/* gnode has been changed */
#define	GSHLOCK		0x00000080	/* file has shared lock */
#define	GEXLOCK		0x00000100	/* file has exclusive lock */
#define	GLWAIT		0x00000200	/* someone waiting on file lock */
#define	GMOD		0x00000400	/* gnode has been modified */
#define GINUSE		0x00000800	/* line turnaround semaphore */
#define	GRENAME		0x00001000	/* gnode is being renamed */
#define GSYNC		0x00002000
#define GINCOMPLETE	0x00004000	/* gnode transitioning between sfs's*/
#define	GXMOD		0x00008000	/* gnode is text, but impure (XXX) */
#define GCMODE		0x00010000	/* permissions of file has changed */
#define GCLINK		0x00020000	/* number of links has changed */
#define GCID		0x00040000	/* owner or group has changed */
#define GTRC		0x00080000	/* one or more procs tracing */

#define ASYNC		GSYNC

/* modes */
#define	GFMT		0170000		/* type of file */

#define	GFPORT		0010000		/* port (named pipe) */
#define	GFCHR		0020000		/* character special */
#define	GFDIR		0040000		/* directory */
#define	GFBLK		0060000		/* block special */
#define	GFREG		0100000		/* regular */
#define	GFLNK		0120000		/* symbolic link */
#define	GFSOCK		0140000		/* socket */

#define	GSUID		04000		/* set user id on execution */
#define	GSGID		02000		/* set group id on execution */
#define	GSVTX		01000		/* save swapped text even after use */
#define	GREAD		0400		/* read, write, execute permissions */
#define	GWRITE		0200
#define	GEXEC		0100

/* maybe this should be in ufs_inode.h */
/*
 * Invalidate an gnode. Used by the namei cache to detect stale
 * information. At an absurd rate of 100 calls/second, the gnode
 * table invalidation should only occur once every 16 months.
 */
#define cacheinval(gp)	\
	(gp)->g_id = ++nextgnodeid; \
	if (nextgnodeid == 0) \
		cacheinvalall();

/*
 * Release the buffer, start I/O on it, but don't wait for completion.
 */
#define bawrite(bp) {							\
	/* check to see if this is a synchronous filesystem */		\
	if (bp->b_gp == NULL ||						\
	   (bp->b_gp && (bp->b_gp->g_mp->m_flags & M_SYNC) == NULL)) {	\
		bp->b_gp = NULL;					\
		bp->b_flags |= B_ASYNC;					\
	}								\
	bwrite(bp);							\
}

#ifdef KERNEL
struct gnode *gnode;		/* the gnode table itself */
struct gnode *gnodeNGNODE;	/* the end of the gnode table */
int	ngnode;			/* number of slots in the table */
long	nextgnodeid;		/* unique id generator */

struct	gnode *rootdir;		/* pointer to gnode of root directory */

struct	gnode *galloc();
struct	gnode *gfs_gget();
#ifdef notdef
struct	inode *gfind();
#endif
struct	gnode *owner();
struct	gnode *maknode();
struct	gnode *gfs_namei();
struct	gnode *getegnode();
void	gremque();
void	gfs_grele();

gno_t	dirpref();
extern union ghead {			/* gnode LRU cache, Chris Maltby */
	union  ghead *gh_head[2];
	struct gnode *gh_chain[2];
} ghead[];

/*
 * GNODE cacheing stuff 
 */
#define GNOHSZ 512

#if	((GNOHSZ&(GNOHSZ-1)) == 0)
#define	GNOHASH(dev,gno)	(((int)((dev)+(gno)))&(GNOHSZ-1))
#else
#define	GNOHASH(dev,gno)	(((int)((dev)+(gno)))%GNOHSZ)
#endif

extern struct gnode *gfreeh, **gfreet;

#endif
#endif __GNODE__
