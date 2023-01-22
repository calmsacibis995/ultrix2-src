
/* 	@(#)vmmac.h	1.4	(ULTRIX)	4/30/86 	*/

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
 * 29 Apr 86 -- depp
 *	Added new locking macros to replace the associated kernel functions.
 *	New lock/unlock/wait macros have been added for page locking,
 *	shared segment locking and text segment locking:
 *		MLOCK/MUNLOCK/MWAIT	  replace  mlock/munlock/mwait
 *		SM_LOCK/SM_UNLOCK/SM_WAIT replace  smlock/smunlock/smwait
 *		X_LOCK/X_UNLOCK/X_WAIT	  replace  xlock/xunlock/xwait
 *
 * 24 Feb 86 -- depp
 *	Added new macros "dmxtob" and "btodmx" for kernel memory allocation
 *	Plus, I added "rbtop" "ispgbnd" "isclbnd" for general use.  NOTE:
 *	"rbtop" SHOULD be used in place of "btoc" EXCEPT where it's known
 *	that clicks are used (a click may not always equal a page!!).
 *
 * 	Also added new macro to convert system virtual addresses to physical
 *	"svtophy"
 */

/*	vmmac.h	6.1	83/07/29	*/

/*
 * Virtual memory related conversion macros
 */

/* Core clicks to number of pages of page tables needed to map that much */
#define	ctopt(x)	(((x)+NPTEPG-1)/NPTEPG)

/* Virtual page numbers to text|data|stack segment page numbers and back */
#define	vtotp(p, v)	((int)(v)-LOWPAGES)
#define	vtodp(p, v)	((int)((v) - stoc(ctos((p)->p_tsize)) - LOWPAGES))
#define	vtosp(p, v)	((int)(btop(USRSTACK) - 1 - (v)))
#define	tptov(p, i)	((unsigned)(i) + LOWPAGES)
#define	dptov(p, i)	((unsigned)(stoc(ctos((p)->p_tsize)) + (i) + LOWPAGES))
#define	sptov(p, i)	((unsigned)(btop(USRSTACK) - 1 - (i)))

/* Tell whether virtual page numbers are in text|data|stack segment */
#define	isassv(p, v)	((v) >= btop(USRSTACK) - (p)->p_ssize)
#define	isatsv(p, v)	(((v) - LOWPAGES) < (p)->p_tsize)
#define	isadsv(p, v)	(((v) - LOWPAGES) >= stoc(ctos((p)->p_tsize)) && \
				!isassv(p, v))

/* Tell whether pte's are text|data|stack */
#define	isaspte(p, pte)		((pte) > sptopte(p, (p)->p_ssize))
#define	isatpte(p, pte)		((pte) < dptopte(p, 0))
#define	isadpte(p, pte)		(!isaspte(p, pte) && !isatpte(p, pte))

/* Text|data|stack pte's to segment page numbers and back */
#define	ptetotp(p, pte)		((pte) - (p)->p_p0br)
#define	ptetodp(p, pte)		((pte) - ((p)->p_p0br + (p)->p_tsize))
#define	ptetosp(p, pte)	\
	(((p)->p_p0br + (p)->p_szpt*NPTEPG - UPAGES - 1) - (pte))

#define	tptopte(p, i)		((p)->p_p0br + (i))
#define	dptopte(p, i)		((p)->p_p0br + (p)->p_tsize + (i))
#define	sptopte(p, i) \
	(((p)->p_p0br + (p)->p_szpt*NPTEPG - UPAGES - 1) - (i))

/* Bytes to pages with and without rounding, and back */
#define rbtop(x)	(((unsigned)(x) + (NBPG - 1)) >> PGSHIFT)
#define	btop(x)		(((unsigned)(x)) >> PGSHIFT)
#define	ptob(x)		((caddr_t)((x) << PGSHIFT))

/* Does the address/size fall on a page/cluster boundary ? */
#define ispgbnd(x)	(((unsigned)(x) & (NBPG - 1)) == 0)
#define isclbnd(x)	(((unsigned)(x) & (CLBYTES - 1)) == 0)

/* Turn virtual addresses into kernel map indices */
#define	kmxtob(a)	(usrpt + (a) * NPTEPG)
#define	btokmx(b)	(((b) - usrpt) / NPTEPG)

/* Turn Virtual addresses into dynamic system map indices */
#define	dmxtob(a)	(dmempt + (a) * NPTEPG)
#define	btodmx(b)	(((b) - dmempt) / NPTEPG)

/* User area address and pcb bases */
#define	uaddr(p)	(&((p)->p_p0br[(p)->p_szpt * NPTEPG - UPAGES]))
#ifdef vax
#define	pcbb(p)		((p)->p_addr[0].pg_pfnum)
#endif

/* Average new into old with aging factor time */
#define	ave(smooth, cnt, time) \
	smooth = ((time - 1) * (smooth) + (cnt)) / (time)

/* Abstract machine dependent operations */
#ifdef vax
#define	setp0br(x)	(u.u_pcb.pcb_p0br = (x), mtpr(P0BR, x))
#define	setp0lr(x)	(u.u_pcb.pcb_p0lr = \
			    (x) | (u.u_pcb.pcb_p0lr & AST_CLR), \
			 mtpr(P0LR, x))
#define	setp1br(x)	(u.u_pcb.pcb_p1br = (x), mtpr(P1BR, x))
#define	setp1lr(x)	(u.u_pcb.pcb_p1lr = (x), mtpr(P1LR, x))
#define	initp1br(x)	((x) - P1PAGES)

/* convert system VA to physical */
#define	svtophy(v)	((int)(ptob((Sysmap[btop((int)(v) & ~VA_SYS)]).pg_pfnum)) | ((int) (v) & VA_BYTEOFFS))

#endif

#define	outofmem()	wakeup((caddr_t)&proc[2]);

/*
 * Page clustering macros.
 * 
 * dirtycl(pte)			is the page cluster dirty?
 * anycl(pte,fld)		does any pte in the cluster has fld set?
 * zapcl(pte,fld) = val		set all fields fld in the cluster to val
 * distcl(pte)			distribute high bits to cluster; note that
 *				distcl copies everything but pg_pfnum,
 *				INCLUDING pg_m!!!
 *
 * In all cases, pte must be the low pte in the cluster, even if
 * the segment grows backwards (e.g. the stack).
 */
#define	H(pte)	((struct hpte *)(pte))

#if CLSIZE==1
#define	dirtycl(pte)	dirty(pte)
#define	anycl(pte,fld)	((pte)->fld)
#define	zapcl(pte,fld)	(pte)->fld
#define	distcl(pte)
#endif

#if CLSIZE==2
#define	dirtycl(pte)	(dirty(pte) || dirty((pte)+1))
#define	anycl(pte,fld)	((pte)->fld || (((pte)+1)->fld))
#define	zapcl(pte,fld)	(pte)[1].fld = (pte)[0].fld
#endif

#if CLSIZE==4
#define	dirtycl(pte) \
    (dirty(pte) || dirty((pte)+1) || dirty((pte)+2) || dirty((pte)+3))
#define	anycl(pte,fld) \
    ((pte)->fld || (((pte)+1)->fld) || (((pte)+2)->fld) || (((pte)+3)->fld))
#define	zapcl(pte,fld) \
    (pte)[3].fld = (pte)[2].fld = (pte)[1].fld = (pte)[0].fld
#endif

#ifndef distcl
#define	distcl(pte)	zapcl(H(pte),pg_high)
#endif


/************************************************************************
 ************************************************************************
 *
 * 		LOCKING MACROS
 *
 ************************************************************************
 ************************************************************************/


/*
 * lock/unlock/wait on a PAGE FRAME
 */

#define MLOCK(c) { \
	while ((c)->c_lock) { \
		(c)->c_want = 1; \
		sleep((caddr_t)(c), PSWP+1); \
	} \
	(c)->c_lock = 1; \
}

#define MUNLOCK(c) { \
	if ((c)->c_lock == 0) \
		panic("MUNLOCK: dup page unlock"); \
	if ((c)->c_want) { \
		wakeup((caddr_t)c); \
		(c)->c_want = 0; \
	} \
	(c)->c_lock = 0; \
}

#define MWAIT(c) { \
	MLOCK(c); \
	MUNLOCK(c); \
}




/* 
 * lock/ulock/wait on a SHARED MEMORY SEGMENT
 */

#define SM_LOCK(s) { \
	while((s)->sm_flag & SMLOCK){ \
		(s)->sm_flag |= SMWANT; \
		sleep((caddr_t)(s), PSWP); \
	} \
	(s)->sm_flag |= SMLOCK; \
}

#define SM_UNLOCK(s) { \
	if((s)->sm_flag & SMWANT) \
		wakeup((caddr_t)(s)); \
	(s)->sm_flag &= ~(SMLOCK|SMWANT); \
}

#define SM_WAIT(s) { \
	SM_LOCK(s); \
	SM_UNLOCK(s); \
}

/*
 * lock/unlock/wait on TEXT SEGMENT
 */

#define X_LOCK(x) { \
	while ((x)->x_flag&XLOCK) { \
		(x)->x_flag |= XWANT; \
		sleep((caddr_t)(x), PSWP); \
	} \
	(x)->x_flag |= XLOCK; \
}

#define X_UNLOCK(x) { \
	if ((x)->x_flag&XWANT) \
		wakeup((caddr_t)(x)); \
	(x)->x_flag &= ~(XLOCK|XWANT); \
}

#define X_WAIT(x) { \
	X_LOCK(x); \
	X_UNLOCK(x); \
}
