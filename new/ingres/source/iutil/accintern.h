/*
 *		@(#)accintern.h	1.1	(ULTRIX)	1/8/85
 */

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
**  ACCINTERN.H -- internal declarations for the access methods
**
**	Nothing in here should be needed for the outside world.
**
**	Version:
**		@(#)accintern.h	7.1	2/5/81
*/

# include	<access.h>


# define	NACCBUFS	3		/* number of access method buffers */


/* the following is the access methods buffer */
struct accbuf
{
	/* this stuff is actually stored in the relation */
	long		mainpg;		/* next main page (0 - eof) */
	long		ovflopg;	/* next ovflo page (0 - none) */
	short		nxtlino;	/* next avail line no for this page */
	char		firstup[PGSIZE - 12];	/* tuple space */
	short		linetab[1];	/* line table at end of buffer - grows down */
					/* linetab[lineno] is offset into
					** the buffer for that line; linetab[nxtlino]
					** is free space pointer */

	/* this stuff is not stored in the relation */
	long		rel_tupid;	/* unique relation id */
	long		thispage;	/* page number of the current page */
	int		filedesc;	/* file descriptor for this reln */
	struct accbuf	*modf;		/* use time link list forward pointer */
	struct accbuf	*modb;		/* back pointer */
	int		bufstatus;	/* various bits defined below */
};

/* The following assignments are status bits for accbuf.bufstatus */
# define	BUF_DIRTY	001	/* page has been changed */
# define	BUF_LOCKED	002	/* page has a page lock on it */
# define	BUF_DIRECT	004	/* this is a page from isam direct */

/* access method buffer typed differently for various internal operations */
struct
{
	char	acc_buf[NACCBUFS];
};

/* pointers to maintain the buffer */
extern struct accbuf	*Acc_head;	/* head of the LRU list */
extern struct accbuf	*Acc_tail;	/* tail of the LRU list */
extern struct accbuf	Acc_buf[NACCBUFS];	/* the buffers themselves */


/*
**  PGTUPLE -- btree index key (a tid and an index key)
*/

struct pgtuple
{
	struct tup_id	childtid;		/* the pointer comes before */
	char		childtup[MAXTUP];
};


/*
**	Global values used by everything
*/

extern char	*Acctuple;		/* pointer to canonical tuple */
extern char	Accanon[MAXTUP];	/* canonical tuple buffer */

/*
**  In-line expansion of trace flags.
*/

extern short	*tT;
# ifndef PDP11
# ifndef tTf
# define tTf(a, b)	((b < 0) ? tT[a] : (tT[a] & (1 << b)))
# endif tTf
# endif PDP11
