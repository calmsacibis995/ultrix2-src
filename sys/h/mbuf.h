/* sccsid  =  @(#)mbuf.h	1.8	ULTRIX	12/16/86 */

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
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
 *			Modification History				*
 *									*
 *									*
 *									*
 *	Larry Cohen -10/09/86						*
 *		fix MFREE - wakeup on address of mfree instead of mfree *
 *									*
 *	Jeff Chase - 03/12/86						*
 *		Changes for "type 2" mbufs:				*
 *			New MCLGET macro with different usage		*
 *			Changes to MFREE macro				*
 *									*
 *	Larry Cohen  -	09/16/85					*
 * 		Add 43bsd alpha tape changes  				*
 *									*
 ************************************************************************/

/*
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)mbuf.h	6.5 (Berkeley) 6/8/85
 */

/*
 * Constants related to memory allocator.
 */
#define	MSIZE		128			/* size of an mbuf */
#define	MMINOFF		12			/* mbuf header length */
#define	MTAIL		4
#define	MMAXOFF		(MSIZE-MTAIL)		/* offset where data ends */
#define	MLEN		(MSIZE-MMINOFF-MTAIL)	/* mbuf data length */
#define	NMBCLUSTERS	256
#define	NMBPCL		(CLBYTES/MSIZE)		/* # mbufs per cluster */

/*
 * Macros for type conversion
 */

/* network cluster number to virtual address, and back */
#define	cltom(x) ((struct mbuf *)((int)mbutl + ((x) << CLSHIFT)))
#define	mtocl(x) (((int)x - (int)mbutl) >> CLSHIFT)

/* address in mbuf to mbuf head */
#define	dtom(x)		((struct mbuf *)((int)x & ~(MSIZE-1)))

/* mbuf head, to typed data */
#define	mtod(x,t)	((t)((int)(x) + (x)->m_off))

struct mbuf {
	struct	mbuf *m_next;		/* next buffer in chain */
	u_long	m_off;			/* offset of data */
	short	m_len;			/* amount of data in this mbuf */
	short	m_type;			/* mbuf type (0 == free) */
	union {
		u_char	mun_dat[MLEN];	/* data storage */
		struct {
			short	mun_cltype;	/* "cluster" type */
			int	(*mun_clfun)();
			int	mun_clarg;
			int	(*mun_clswp)();
		} mun_cl;
	} m_un;
	struct	mbuf *m_act;		/* link in higher-level mbuf list */
};

#define	m_dat	m_un.mun_dat
#define	m_cltype m_un.mun_cl.mun_cltype
#define	m_clfun	m_un.mun_cl.mun_clfun
#define	m_clarg	m_un.mun_cl.mun_clarg
#define	m_clswp	m_un.mun_cl.mun_clswp

/* mbuf types */
#define	MT_FREE		0	/* should be on free list */
#define	MT_DATA		1	/* dynamic (data) allocation */
#define	MT_HEADER	2	/* packet header */
#define	MT_SOCKET	3	/* socket structure */
#define	MT_PCB		4	/* protocol control block */
#define	MT_RTABLE	5	/* routing tables */
#define	MT_HTABLE	6	/* IMP host tables */
#define	MT_ATABLE	7	/* address resolution tables */
#define	MT_SONAME	8	/* socket name */
#define	MT_ZOMBIE	9	/* zombie proc status */
#define	MT_SOOPTS	10	/* socket options */
#define	MT_FTABLE	11	/* fragment reassembly header */
#define	MT_RIGHTS	12	/* access rights */
#define MT_OPT		13	/* DECnet optional data */
#define	MT_IFADDR	14	/* interface address */
#define	MT_ACCESS	15	/* access control info */

/* flags to m_get */
#define	M_DONTWAIT	0
#define	M_WAIT		1

/* flags to m_pgalloc */
#define	MPG_MBUFS	0		/* put new mbufs on free list */
#define	MPG_CLUSTERS	1		/* put new clusters on free list */
#define	MPG_SPACE	2		/* don't free; caller wants space */

/* length to m_copy to copy all */
#define	M_COPYALL	1000000000

#define	MGET(m, i, t) \
	{ int ms = splimp(); \
	  if ((m)=mfree) \
		{ if ((m)->m_type != MT_FREE) panic("mget"); (m)->m_type = t; \
		  mbstat.m_mtypes[MT_FREE]--; mbstat.m_mtypes[t]++; \
		  mfree = (m)->m_next; (m)->m_next = 0; \
		  (m)->m_off = MMINOFF; } \
	  else \
		(m) = m_more(i, t); \
	  splx(ms); }

/* If no more clusters try to get some more */

#define	MCLGET(m, p) \
	{ int ms = splimp(); \
	  if (mclfree == 0) \
		(void)m_clalloc(1, MPG_CLUSTERS, M_DONTWAIT); \
	  if ((p)=mclfree) \
	     {++mclrefcnt[mtocl(p)];mbstat.m_clfree--; \
		mclfree = (p)->m_next; (m)->m_cltype = 1; \
		m->m_off = (int)p - (int)m;} \
	  splx(ms); }

#define	MFREE(m, n) \
	{ int ms = splimp(); \
	  if ((m)->m_type == MT_FREE) panic("mfree"); \
	  mbstat.m_mtypes[(m)->m_type]--; mbstat.m_mtypes[MT_FREE]++; \
	  (m)->m_type = MT_FREE; \
	  if ((m)->m_off >= MSIZE) { \
		switch (m->m_cltype) { \
		case 1: \
			(n) = (struct mbuf *)(mtod(m, int)&~CLOFSET); \
			if (--mclrefcnt[mtocl(n)] == 0) { \
				(n)->m_next = mclfree;mclfree = (n); \
				mbstat.m_clfree++; \
			} \
			break; \
		case 2: \
			(*m->m_clfun)(m->m_clarg); \
			break; \
		default: \
			panic("m_free has bad m_cltype"); \
		} \
	  } \
	  (n) = (m)->m_next; (m)->m_next = mfree; \
	  (m)->m_off = 0; (m)->m_act = 0; mfree = (m); \
	  splx(ms); \
	  if (m_want) { \
		  m_want = 0; \
		  wakeup((caddr_t)&mfree); \
	  } \
	}

/*
 * Mbuf statistics.
 */
struct mbstat {
	short	m_mbufs;	/* mbufs obtained from page pool */
	short	m_clusters;	/* clusters obtained from page pool */
	short	m_clfree;	/* free clusters */
	short	m_drops;	/* times failed to find space */
	short	m_mtypes[256];	/* type specific mbuf allocations */
};

#ifdef	KERNEL
extern	struct mbuf mbutl[];		/* virtual address of net free mem */
extern	struct pte Mbmap[];		/* page tables to map Netutl */
struct	mbstat mbstat;
int	nmbclusters;
struct	mbuf *mfree, *mclfree;
extern	char	*mclrefcnt;
int	m_want;
struct	mbuf *m_get(),*m_getclr(),*m_free(),*m_more(),*m_copy(),*m_pullup();
caddr_t	m_clalloc();
#endif
