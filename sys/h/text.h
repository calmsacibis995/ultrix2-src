/* 	@(#)text.h	1.8	(ULTRIX)	12/16/86 	*/

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
/*	text.h	6.1	83/07/29	*/

/***********************************************************************
 *
 *		Modification History
 *
 * 15 Dec 86 -- depp
 *	added text table queue/dequeuing macros
 *
 * 11 Sep 86 -- koehler
 *	added text table management
 *
 ***********************************************************************/

/*
 * Text structure.
 * One allocated per pure
 * procedure on swap device.
 * Manipulated by text.c
 */
#define	NXDAD	12		/* param.h:MAXTSIZ / dmap.h:DMTEXT */

struct xfree {
	struct text  *xun_freef;  /* free text pointer */
	struct text  *xun_freeb;  /* free text pointer */
};

struct text
{
	struct xfree x_free;	/* linked list of free text structures */
	swblk_t	x_daddr[NXDAD];	/* disk addresses of DMTEXT-page segments */
	swblk_t	x_ptdaddr;	/* disk address of page table */
	size_t	x_size;		/* size (clicks) */
	struct proc *x_caddr;	/* ptr to linked proc, if loaded */
 	struct gnode *x_gptr;	/* gnode of prototype */
	short	x_rssize;
	short	x_swrss;
	short	x_count;	/* reference count */
	short	x_ccount;	/* number of loaded references */
	short	x_lcount;	/* number of processes locking segment */
	char	x_flag;		/* traced, written flags */
	char	x_slptime;
	short	x_poip;		/* page out in progress count */
	struct cmap *x_cmap;
	u_int	x_blkno;	/* blkno for validation of cmap hash chain */
	u_int	x_dindex;	/* device index for cmap validation as above */
};

#define x_freef x_free.xun_freef
#define x_freeb x_free.xun_freeb

#ifdef	KERNEL
extern struct xfree freetext;
struct	text *text, *textNTEXT;
int	ntext;
#endif

#define	XTRC	01		/* Text may be written, exclusive use */
#define	XWRIT	02		/* Text written into, must swap out */
#define	XLOAD	04		/* Currently being read from file */
#define	XLOCK	010		/* Being swapped in or out */
#define	XWANT	020		/* Wanted for swapping */
#define	XPAGI	040		/* Page in on demand from inode */
#define XNOSW	0100		/* Lock segment in memory */
#define XFREE	0200		/* Text table on free list */

/*
 * Macros to queue and dequeue text table entry on free list
 * Also, a macro to check whether a text entry is free
 */

#define	X_QFREE(xp)	{			\
	if ((xp)->x_flag & XFREE)		\
		panic("Freeing free text");	\
	insque(&(xp)->x_free, &freetext);	\
	(xp)->x_flag |= XFREE;			\
}

#define	X_DQFREE(xp)	{				\
	if (((xp)->x_flag & XFREE) == 0)		\
		panic("Dequeuing non-free text");	\
	remque(&(xp)->x_free);				\
	(xp)->x_flag &= ~XFREE;				\
}
	
#define X_ISFREE(xp)	((xp)->x_flag & XFREE)

/*
 *	X_FLUSH macro will flush memory hash list if required
 */
#define X_FLUSH(xp,gp)	{				\
	if (!ISLOCAL((gp)->g_mp))			\
		xflush_free_text(xp);			\
}
