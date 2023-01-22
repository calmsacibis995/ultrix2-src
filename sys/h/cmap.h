/* 	@(#)cmap.h	1.9	(ULTRIX)	1/15/87 	*/

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
/*	cmap.h	6.1	83/07/29	*/
/*
 *	Modification History
 *
 * 14 Jan 87 -- rr
 *	performance macros
 *
 * 08 Jan 87 -- depp
 *	Up'ed physical memory limit to 1/2 Gbyte
 *
 * 18 Mar 86 -- depp
 *	Added "u" area free list {ucmap, eucmap, nucmap}
 *
 * 25 Apr 85 -- depp
 *	Removed SHMEM ifdefs
 *
 * 12 Mar 85 -- depp
 *	Expanded c_type field to handle shared memory segment type
 *
 */

/*
 * core map entry
 *
 * Limits imposed by this structure
 *
 *		limit		cur.size	fields
 *
 *	physical memory		512 Mb		c_next, c_prev, c_hlink
 *	mounted file systems	255		c_mdev
 *	size of proc segment	1 Gb		c_page
 *	filesystem size		8 Gb		c_blkno	
 *	proc, text table size	4096		c_ndx
 */
#ifndef LOCORE
struct cmap
{
unsigned int 	c_next:19,	/* index of next free list entry */
		:5,		/* expansion for c_next */
		c_free:1,	/* on the free list */
		c_intrans:1,	/* intransit bit */
		c_gone:1,	/* associated page has been released */
		c_want:1,	/* wanted */
		c_lock:1,	/* locked for raw i/o or pagein */
		c_type:3,	/* type CSYS, CTEXT, CSTACK, CDATA or CSMEM */

		c_prev:19,	/* index of previous free list entry */
		:5,		/* expansion for c_prev */
		c_mdev:8,	/* which mounted dev this is from */

		c_hlink:19,	/* hash link for <blkno,mdev> */
		:1,		/* expansion for c_hlink */
		c_ndx:12,	/* index of owner proc or text */

		c_blkno:24,	/* disk block this is a copy of */
		:8,		/* fill to longword boundary */

		c_page:21,	/* virtual page number in segment */
		:3,		/* expansion for c_page */
		c_refcnt:8;	/* ref count for future use */
};
#else LOCORE
/* 
 * The next four items may need to change as the cmap structure 
 * changes.  They are used in locore.s
 */

#define MAX_MEM		512*1024	/* 512 Mbytes */
#define CMAPSZ		20		/* # of bytes for the cmap structure */
#define CMAP_FREE	24		/* bit displacement to c_free */
#define CMAP_INTRANS	25		/* bit displacement to c_intrans */
#endif LOCORE

#define	CMHEAD	0

/*
 * Shared text pages are not totally abandoned when a process
 * exits, but are remembered while in the free list hashed by <mdev,blkno>
 * off the cmhash structure so that they can be reattached
 * if another instance of the program runs again soon.
 */
#define	CMHSIZ	512		/* SHOULD BE DYNAMIC - MUST BE POWER OF 2 */
#define	CMHASH(bn)	((bn)&(CMHSIZ-1))

#ifndef LOCORE
#ifdef	KERNEL
struct	cmap *cmap;
struct	cmap *ecmap;
int	ncmap;
struct	cmap *mfind();
int	firstfree, maxfree;
int	ecmx;			/* cmap index of ecmap */
int	cmhash[CMHSIZ];

/* temp "u" area free list */
int	ucmap;			/* head */
int	eucmap;			/* tail */
int	nucmap;			/* number of clusters in list */
#endif

/* bits defined in c_type */

#define	CSYS		0		/* none of below */
#define	CTEXT		1		/* belongs to shared text segment */
#define	CDATA		2		/* belongs to data segment */
#define	CSTACK		3		/* belongs to stack segment */
#define CSMEM		4		/* shared memory segment */

/* special fast macros for pgtocm -- shifts avoid an ediv instruction */
#if CLSIZE==1
#define	pgtocm(x)	(((x)-firstfree) + 1)
#define	cmtopg(x)	(((x)-1) + firstfree)
#endif
#if CLSIZE==2 || CLSIZE==4
#define	pgtocm(x)	((((x)-firstfree) >> (CLSIZE>>1) ) + 1)
#define	cmtopg(x)	((((x)-1) << (CLSIZE>>1) ) + firstfree)
#endif
#if CLSIZE!=1 && CLSIZE!=2 && CLSIZE!=4
#define	pgtocm(x)	((((int)(x)-firstfree) / CLSIZE) + 1)
#define	cmtopg(x)	((((x)-1) * CLSIZE) + firstfree)
#endif

extern	int	ucmap;
extern	int	eucmap;
extern	int	nucmap;
#endif LOCORE
