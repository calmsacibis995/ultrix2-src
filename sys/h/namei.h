/* @(#)namei.h	1.3	(ULTRIX)	9/11/86 */

#ifndef _NAMEI_
#define	_NAMEI_

#ifdef KERNEL
#include "uio.h"
#include "dir.h"
#else
#include <sys/uio.h>
#endif

/* "@(#)namei.h	1.2	(ULTRIX)	3/23/86 */

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
 *
 *			Modification History
 *
 *	Koehler 11 Sep 86
 *	gfs namei intreface change
 *
 *	Stephen Reilly, 09-Sept-85
 *	Created for the new 4.3BSD namei code.
 *
 *	@(#)namei.h	6.10 (Berkeley) 6/8/85
 *
 ***********************************************************************/
/*
 * Encapsulation of namei parameters.
 * One of these is located in the u. area to
 * minimize space allocated on the kernel stack.
 */
struct nameidata {
	char 	*ni_dirp;		/* pathname pointer */
	short	ni_nameiop;		/* see below */
	short	ni_error;		/* error return if any */
	off_t	ni_endoff;		/* end of useful stuff in directory */
	u_short	ni_slcnt;		/* symbolic link count */
	struct	gnode *ni_pdir;		/* gnode of parent directory of dirp */
	struct	iovec ni_iovec;		/* MUST be pointed to by ni_iov */
	struct	uio ni_uio;		/* directory I/O parameters */
	struct	direct ni_dent;		/* current directory entry */
	struct	buf *ni_bufp;		/* this is UGLY but I don't know how else to do it */
	char 	*ni_cp;			/* where the name is held */
};

#define	ni_base		ni_iovec.iov_base
#define	ni_count	ni_iovec.iov_len
#define	ni_iov		ni_uio.uio_iov
#define	ni_iovcnt	ni_uio.uio_iovcnt
#define	ni_offset	ni_uio.uio_offset
#define	ni_segflg	ni_uio.uio_segflg
#define	ni_resid	ni_uio.uio_resid

/*
 * namei opertions
 */
#define	LOOKUP		0x0	/* perform name lookup only */
#define	CREATE		0x1	/* setup for file creation */
#define	DELETE		0x2	/* setup for file deletion */
#define	LOCKPARENT	0x10	/* see the top of namei */
#define NOCACHE		0x20	/* name must not be left in cache */
#define FOLLOW		0x40	/* follow symbolic links */
#define NOMOUNT		0x80	/* don't traverse mount points */
#define	NOFOLLOW	0x0	/* don't follow symbolic links (pseudo) */

/*
 * This structure describes the elements in the cache of recent
 * names looked up by namei.
 */
struct	nch {
	struct	nch *nc_forw, *nc_back;	/* hash chain, MUST BE FIRST */
	struct	nch *nc_nxt, **nc_prev;	/* LRU chain */
	struct	gnode *nc_ip;		/* gnode the name refers to */
	ino_t	nc_ino;			/* ino of parent of name */
	dev_t	nc_dev;			/* dev of parent of name */
	dev_t	nc_idev;		/* dev of the name ref'd */
	long	nc_id;			/* referenced gnode's id */
	char	nc_nlen;		/* length of name */
#define	NCHNAMLEN	15	/* maximum name segment length we bother with */
	char	nc_name[NCHNAMLEN];	/* segment name */
};
struct	nch *nch;
int	nchsize;

/*
 * Stats on usefulness of namei caches.
 */
struct	nchstats {
	long	ncs_goodhits;		/* hits that we can reall use */
	long	ncs_badhits;		/* hits we must drop */
	long	ncs_falsehits;		/* hits with id mismatch */
	long	ncs_miss;		/* misses */
	long	ncs_long;		/* long names that ignore cache */
	long	ncs_pass2;		/* names found with passes == 2 */
	long	ncs_2passes;		/* number of times we attempt it */
};
#endif
extern union	nchash	{
	union	nchash	*nch_head[2];
	struct	nch	*nch_chain[2];
} nchash[];


struct	buf *blkatoff();
extern int	dirchk;

/*
 * Structures associated with name cacheing.
 */
#define	NCHHASH		32	/* size of hash table */

#if	((NCHHASH)&((NCHHASH)-1)) != 0
#define	NHASH(h, i, d)	((unsigned)((h) + (i) + 13 * (int)(d)) % (NCHHASH))
#else
#define	NHASH(h, i, d)	((unsigned)((h) + (i) + 13 * (int)(d)) & ((NCHHASH)-1))
#endif

#define	nch_forw	nch_chain[0]
#define	nch_back	nch_chain[1]

extern struct	nch	*nchhead, **nchtail;	/* LRU chain pointers */
extern struct	nchstats nchstats;		/* cache effectiveness statistics */
