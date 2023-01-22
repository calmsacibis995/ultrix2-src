#ifndef lint
static char *sccsid = "@(#)kern_mman.c	1.11	ULTRIX	10/3/86";
#endif

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
 *   Modification history:
 *
 * 24 Feb 86 -- depp
 *	Added new system call "mprotect".
 *
 * 12-Feb-86 -- jrs
 *	Added calls to tbsync() for mp translation buffer control
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 * 18 Sep 85 -- depp
 *	Added "plock" system call
 *
 * 26 Jul 85 -- depp
 *	Put check into ovadvise to insure that if the calling process
 *	has shared memory segments attached, that an error is returned
 *	unless the input parameter indicates normal processing.
 *
 * 19 Jul 85 -- depp
 *	Corrected calculation in first arg of smexpand in obreak. 
 *
 * 09 Apr 85 -- depp
 *	Added System V Shared memory support to obreak
 *
 */

#include "../machine/reg.h"
#include "../machine/psl.h"
#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/gnode.h"
#include "../h/seg.h"
#include "../h/acct.h"
#include "../h/wait.h"
#include "../h/vm.h"
#include "../h/text.h"
#include "../h/file.h"
#include "../h/vadvise.h"
#include "../h/cmap.h"
#include "../h/trace.h"
#include "../h/mman.h"
#include "../h/conf.h"
#include "../h/lock.h"
#include "../machine/mtpr.h"

sbrk()
{

}

sstk()
{

}

getpagesize()
{

	u.u_r.r_val1 = NBPG * CLSIZE;
}

smmap()
{
#ifdef notdef
	struct a {
		caddr_t	addr;
		int	len;
		int	prot;
		int	share;
		int	fd;
		off_t	pos;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
	register struct gnode *gp;
	register struct fpte *pte;
	int off;
	int fv, lv, pm;
	dev_t dev;
	int (*mapfun)();
	extern struct file *getgnode();

	fp = getgnode(uap->fd);
	if (fp == NULL)
		return;
	gp = (struct gnode *)fp->f_data;
	if ((gp->g_mode & GFMT) != GFCHR) {
		u.u_error = EINVAL;
		return;
	}
	dev = gp->g_rdev;
	mapfun = cdevsw[major(dev)].d_mmap;
	if (mapfun == NULL) {
		u.u_error = EINVAL;
		return;
	}
	if (((int)uap->addr & CLOFSET) || (uap->len & CLOFSET) ||
	    (uap->pos & CLOFSET)) {
		u.u_error = EINVAL;
		return;
	}
	if ((uap->prot & PROT_WRITE) && (fp->f_flag&FWRITE) == 0) {
		u.u_error = EINVAL;
		return;
	}
	if ((uap->prot & PROT_READ) && (fp->f_flag&FREAD) == 0) {
		u.u_error = EINVAL;
		return;
	}
	if (uap->share != MAP_SHARED) {
		u.u_error = EINVAL;
		return;
	}
	fv = btop(uap->addr);
	lv = btop(uap->addr + uap->len - 1);
	if (lv < fv || !isadsv(u.u_procp, fv) || !isadsv(u.u_procp, lv)) {
		u.u_error = EINVAL;
		return;
	}
	for (off=0; off<uap->len; off += NBPG) {
		if ((*mapfun)(dev, uap->pos+off, uap->prot) == -1) {
			u.u_error = EINVAL;
			return;
		}
	}
	if (uap->prot & PROT_WRITE)
		pm = PG_UW;
	else
		pm = PG_URKR;
	for (off = 0; off < uap->len; off += NBPG) {
		pte = (struct fpte *)vtopte(u.u_procp, fv);
		u.u_procp->p_rssize -= vmemfree(pte, 1);
		*(int *)pte = pm;
		pte->pg_v = 1;
		pte->pg_fod = 1;
		pte->pg_fileno = uap->fd;
		pte->pg_blkno = (*mapfun)(dev, uap->pos+off, uap->prot);
		fv++;
	}
	u.u_procp->p_flag |= SPTECHG;
	u.u_pofile[uap->fd] |= UF_MAPPED;
#endif
}

mremap()
{

}

munmap()
{
#ifdef notdef
	register struct a {
		caddr_t	addr;
		int	len;
	} *uap = (struct a *)u.u_ap;
	int off;
	int fv, lv;
	register struct pte *pte;

	if (((int)uap->addr & CLOFSET) || (uap->len & CLOFSET)) {
		u.u_error = EINVAL;
		return;
	}
	fv = btop(uap->addr);
	lv = btop(uap->addr + uap->len - 1);
	if (lv < fv || !isadsv(u.u_procp, fv) || !isadsv(u.u_procp, lv)) {
		u.u_error = EINVAL;
		return;
	}
	for (off = 0; off < uap->len; off += NBPG) {
		pte = vtopte(u.u_procp, fv);
		u.u_procp->p_rssize -= vmemfree(pte, 1);
		*(int *)pte = (PG_UW|PG_FOD);
		((struct fpte *)pte)->pg_fileno = PG_FZERO;
		fv++;
	}
	u.u_procp->p_flag |= SPTECHG;
#endif
}

munmapfd(fd)
{
#ifdef notdef
	register struct fpte *pte;
	register int i;

	for (i = 0; i < u.u_dsize; i++) {
		pte = (struct fpte *)dptopte(u.u_procp, i);
		if (pte->pg_v && pte->pg_fod && pte->pg_fileno == fd) {
			*(int *)pte = (PG_UW|PG_FOD);
			pte->pg_fileno = PG_FZERO;
		}
	}
#endif
	u.u_pofile[fd] &= ~UF_MAPPED;
	
}

/*
 * Function:
 *
 *	mprotect -- system call
 *
 * Function description:
 *
 *	This system call permits a user process to change the protection
 *	of a block of the caller's virtual memory.  Currently, only private
 *	data memory can be so protected.  Protection is performed on a cluster
 *	and a cluster size may increase in future ULTRIX versions.
 *
 * Interface:
 *
 *	mprotect (addr, len, prot)
 *	  caddr_t addr;		address of the beginning of the block
 *	  int len;	  	length of the block in bytes
 *	  int prot;	  	protection flags (see <mman.h> for values )
 *
 *	"addr" must fall on a page cluster boundary.
 *	"len" will be rounded up to a cluster boundary.
 *
 * Return Value:
 *
 *	> 0	Normal -- size of protected block (in bytes)
 *	< 0	Error - Errno set
 *
 * Error Handling:
 *
 *	ERRNO value	Description
 *
 *	  
 *	  EALIGN	Input address not cluster aligned
 *	  EINVAL	Invalid protection mask (parameter "prot")
 *	  EACCES	Memory block not within private data space
 *
 * Side Effects:
 *
 *	If the user handles a SIGBUS signal, the signal handler MUST either
 *	abort the process or correct the condition that caused the protection
 *	fault (SIGBUS).  If some corrective action is not taken, an infinite
 *	loop will result.
 */

#define PROT_MASK	(PROT_READ | PROT_WRITE | PROT_EXEC)

/* used to map "prot" parameter to VAX protection */
u_int vaxmprot[] = {
	PG_KW,		/* No Access */
	PG_URKW,	/* Read only */
	PG_UW,		/* Write only */
	PG_UW,		/* Read/Write */
	PG_URKW,	/* Execute */
	PG_URKW,	/* Execute/Read */
	PG_UW,		/* Execute/Write */
	PG_UW,		/* Execute/Read/Write */
};

mprotect()
{
	register struct a {
		caddr_t	addr;
		int	len;
		int	prot;
	} *uap = (struct a *)u.u_ap;
	register int npages;			/* number of full pages */
	register struct pte *pte;		/* pointer to page table */
	register u_int vaxprot;			/* VAX protect for block */
	register int v;				/* VA of beginning of block */
	int ve;					/* VA of end of block */
	register caddr_t addr = uap->addr;	/* "addr" */
	struct proc *p = u.u_procp;		/* proc pointer */
	int nbytes;				/* number of bytes protected */

	/* is address on cluster boundary? */
	if (!isclbnd(addr)) {
		u.u_error = EALIGN;
		return;
	}

	/* Retrieve VAX protection code */
	if (uap->prot & ~PROT_MASK) {
		u.u_error = EINVAL;
		return;
	}
	vaxprot = vaxmprot[uap->prot];

	/* convert to pages on cluster boundary */
	npages = clrnd(rbtop(uap->len));
	nbytes = (int) ptob(npages);

	/* calulate first and last address of block */
	v = btop(addr);
	ve = v + npages - 1;

	/* Only permit user to change protection on a private data block */
	if (isadsv(p, v) && (vtodp(p,ve) < p->p_dsize))
		pte = dptopte(p, vtodp(p, v));
	else {
		u.u_error = EACCES;
		return;
	}

	/* change protection on PTEs */	
	while (npages--) {
		pte->pg_prot = 0;
		*(u_int *) pte |= vaxprot;
		pte++;
	}
	mtpr(TBIA, 0);
	u.u_r.r_val1 = nbytes;
	return;
}

madvise()
{

}

mincore()
{

}

/* BEGIN DEFUNCT */
obreak()
{
	struct a {
		char	*nsiz;
	};
	register int n, d;

	/*
	 * set n to new data size
	 * set d to new-old
	 */

	n = btoc(((struct a *)u.u_ap)->nsiz) - ctos(u.u_tsize) * stoc(1);
	if (n < 0)
		n = 0;
	if ((d = clrnd(n - u.u_dsize)) == 0)
		return;
	if (ctob(u.u_dsize+d) > u.u_rlimit[RLIMIT_DATA].rlim_cur) {
		u.u_error = ENOMEM;
		return;
	}

	/* begin SHMEM */
	if(u.u_procp->p_smbeg){
		if(d > 0){
			if((u.u_tsize+u.u_dsize+d)> u.u_procp->p_smbeg){
				u.u_error = ENOMEM;
				return;
			}
			if(swpexpand(u.u_dsize+d,u.u_ssize,
					&u.u_dmap,&u.u_smap)==0)
				return;
		}

		smexpand(d);
		u.u_procp->p_smsize -= d;
		u.u_smsize -= d;
		return;
	}
	/* end SHMEM */

	if (chksize((u_int)u.u_tsize, (u_int)u.u_dsize+d, (u_int)u.u_ssize))
		return;	

	if (d > 0) {
		if (swpexpand(u.u_dsize+d,u.u_ssize,&u.u_dmap,&u.u_smap) == 0)
			return;
	}
	else
		u.u_dmap.dm_size = u.u_dsize+d;

	expand(d, 0);
}

int	both;

ovadvise()
{
	register struct a {
		int	anom;
	} *uap;
	register struct proc *rp = u.u_procp;
	int oanom = rp->p_flag & SUANOM;
	register struct pte *pte;
	register struct cmap *c;
	register int i;

#ifdef lint
	both = 0;
#endif
	uap = (struct a *)u.u_ap;

	/* begin SHMEM */
	/* currently advise and shmem are incompatable */
	if (rp->p_smbeg && uap->anom != VA_NORM) {
		u.u_error = EINVAL;
		return;
	}
	/*  end SHMEM */

	trace(TR_VADVISE, uap->anom, u.u_procp->p_pid);
	rp->p_flag &= ~(SSEQL|SUANOM);
	switch (uap->anom) {

	case VA_ANOM:
		rp->p_flag |= SUANOM;
		break;

	case VA_SEQL:
		rp->p_flag |= SSEQL;
		break;
	}
	if ((oanom && (rp->p_flag & SUANOM) == 0) || uap->anom == VA_FLUSH) {
		for (i = 0; i < rp->p_dsize; i += CLSIZE) {
			pte = dptopte(rp, i);
			if (pte->pg_v) {
				c = &cmap[pgtocm(pte->pg_pfnum)];
				if (c->c_lock)
					continue;
				pte->pg_v = 0;
				if (anycl(pte, pg_m))
					pte->pg_m = 1;
				distcl(pte);
			}
		}
	}
	if (uap->anom == VA_FLUSH) {	/* invalidate all pages */
		for (i = 1; i < rp->p_ssize; i += CLSIZE) {
			pte = sptopte(rp, i);
			if (pte->pg_v) {
				c = &cmap[pgtocm(pte->pg_pfnum)];
				if (c->c_lock)
					continue;
				pte->pg_v = 0;
				if (anycl(pte, pg_m))
					pte->pg_m = 1;
				distcl(pte);
			}
		}
		for (i = 0; i < rp->p_tsize; i += CLSIZE) {
			pte = tptopte(rp, i);
			if (pte->pg_v) {
				c = &cmap[pgtocm(pte->pg_pfnum)];
				if (c->c_lock)
					continue;
				pte->pg_v = 0;
				if (anycl(pte, pg_m))
					pte->pg_m = 1;
				distcl(pte);
				distpte(rp->p_textp, i, pte);
			}
		}
	}
#ifdef vax
	mtpr(TBIA, 0);
	tbsync();
#endif
}
/* END DEFUNCT */

/*
 * grow the stack to include the SP
 * true return if successful.
 */
grow(sp)
	unsigned sp;
{
	register int si;

	if (sp >= USRSTACK-ctob(u.u_ssize))
		return (0);
	si = clrnd(btoc((USRSTACK-sp)) - u.u_ssize + SINCR);
	if (ctob(u.u_ssize+si) > u.u_rlimit[RLIMIT_STACK].rlim_cur)
		return (0);
	if (chksize((u_int)u.u_tsize, (u_int)u.u_dsize, (u_int)u.u_ssize+si))
		return (0);
	if (swpexpand(u.u_dsize, u.u_ssize+si, &u.u_dmap, &u.u_smap)==0)
		return (0);
	
	expand(si, 1);
	return (1);
}

/*
 * PLOCK -- This system call provides the user with the ability to 
 *	    "lock" the process' segments into memory.  Only a superuser
 *	    process may do this for obvious reasons.  
 *
 *	    When a segment is locked, 2 flags are set, one in u.u_lock that
 *	    tracks the process' memory locking status.  The other in either
 *	    p->p_flag (data) or tp->x_flag (text) to indicate the lock on
 *	    that level for paging purposes.  In addition, the text structure
 *	    contains a count of the number of shared processes have currently
 *	    locked the text.  When a process unlocks it text segment, the 
 *	    text remains locked until the count (x_lcount) drops to 0.  A
 *	    side effect of this is that if one process locks the segment,
 *	    it's locked for all attached procs as this flag is the one used to
 *	    determine eligibility for paging/swapping.
 *
 *	Input:		long opt;	* operation (see ../h/lock.h)
 *	Return:		0		* normal
 *			-1		* error; errno is set
 */
plock()
{
	struct a {
		long opt;
	};

	if (!suser())
		return;
	switch(((struct a *)u.u_ap)->opt) {
	case TXTLOCK:
		if ((u.u_lock & TXTLOCK) || textlock() == 0)
			goto bad;
		break;
	case PROCLOCK:
		if ((u.u_lock & (TXTLOCK|DATLOCK)) || proclock() == 0)
			goto bad;
		break;
	case DATLOCK:
		if ((u.u_lock & DATLOCK)  ||  datalock() == 0)
			goto bad;
		break;
	case UNLOCK:
		if ((u.u_lock & (TXTLOCK|DATLOCK)) == 0 || punlock() == 0)
			goto bad;
		break;

	default:
bad:
		u.u_error = EINVAL;
	}
}

textlock()
{
	register struct text *tp;	/* text pointer */

	if ((tp = u.u_procp->p_textp) == NULL)
		return(0);
	tp->x_flag |= XNOSW;
	tp->x_lcount++;
	u.u_lock |= TXTLOCK;
	return(1);
}
		
tunlock()
{
	register struct text *tp;	/* text pointer */

	if ((tp = u.u_procp->p_textp) == NULL)
		return(0);
	if (!(--tp->x_lcount))		
		tp->x_flag &= ~XNOSW;
	u.u_lock &= ~TXTLOCK;
	return(1);
}

datalock()
{
	register struct proc *pp;	/* proc pointer */

	if ((pp = u.u_procp) == NULL)
		return(0);
	pp->p_flag |= SULOCK;
	u.u_lock |= DATLOCK;
	return(1);
}
		
dunlock()
{
	register struct proc *pp;	/* proc pointer */

	if ((pp = u.u_procp) == NULL)
		return(0);
	pp->p_flag &= ~SULOCK;
	u.u_lock &= ~DATLOCK;
	return(1);
}

proclock()
{
	return (textlock() && datalock());
}

punlock()
{
	if (u.u_lock & (TXTLOCK))
		tunlock();
	if (u.u_lock & (DATLOCK))
		dunlock();
	return(1);
}

/* 
 *  end of PLOCK code
 */

/*
 * VMTEST - Virtual memory test interface to outside world.  Use this
 *	    interface to test internal kernel functions by providing 
 *	    user level controls.
 */
#ifdef VMTEST
#include "../h/kmalloc.h"
#define T_KMEMALL01	0	/* kmemall test 1 */
#define T_KMEMFRE01	1	/* Kmemfree test 1 */
#define T_KMALLOC01	2	/* kmalloc test 1 */
#define T_KFREE01	3	/* kfree test 1 */
#define T_KHAMMER01	4	/* continous test (flg=0 stop) */
#define T_STATS		5	/* status printout */
#define T_CLEANUP	6	/* Cleanup hammer test */

int sizeseg;
char *ptr;
vmtest () {
    struct a {
	int tst;
	int flg;
	int *params;
    } *uap = (struct a *) u.u_ap;
    char *kmemall01();
    char *kmalloc01();
    
	switch (uap->tst) {

	case T_KMEMALL01:
		if (copyin(uap->params,&sizeseg,sizeof(sizeseg)))
			u.u_error = EFAULT;
		else {
			ptr = kmemall01(sizeseg,uap->flg);
				
		}
		break;

	case T_KMEMFRE01:
		if (copyin(uap->params,&sizeseg,sizeof(sizeseg)))
			u.u_error = EFAULT;
		else {
			kmemfre01(uap->flg,sizeseg);
		}
		break;

	case T_KMALLOC01:
		if (copyin(uap->params,&sizeseg,sizeof(sizeseg)))
			u.u_error = EFAULT;
		else {
			ptr = kmalloc01(sizeseg);
				
		}
		break;

	case T_KFREE01:
		if (copyin(uap->params,&sizeseg,sizeof(sizeseg)))
			u.u_error = EFAULT;
		else {
			kfree01(uap->flg,sizeseg);
		}
		break;

	case T_KHAMMER01:
		khammer01(uap->flg);
		break;

	case T_STATS:
		kstats(uap->flg);
		break;

	case T_CLEANUP:
		kh_cleanup();
		break;

	default:
		u.u_error = EINVAL;
		break;
	}
}

char *
kmemall01(size,flg)
register size;
register flg;
{
char *p;
char *kmemall();

	if ((p = kmemall(size,flg&1)) == 0) {
		u.u_error = ENOMEM;
		return(0);
	}
	else
		cprintf("rtn ptr = %x\n",p);
	return(p);
}

kmemfre01 (p,s) 
register char *p;
register  s;
{
	if (kmemfree(p,s) < 0)
		u.u_error = ENOMEM;
}
char *
kmalloc01(size)
register size;
{
char *p;
caddr_t km_alloc();

	if ((p = km_alloc(size)) == 0) {
		u.u_error = ENOMEM;
		return(0);
	}
	else
		cprintf("rtn ptr = %x\n",p);
	return(p);
}

kfree01 (p,sizeseg) 
register char *p;
{
	if (km_free(p,sizeseg) < 0)
		u.u_error = ENOMEM;
}

extern struct timeval time;
int khfreq = 0;
int khcleanup = 0;

khammer01 (flg)
int flg;
{
	int kh_main();

	if (flg) {
		if (khfreq) {
			u.u_error = EINVAL;
			return; 
		}
		khfreq = flg;
		cprintf("khammer: *** START TEST @ %d ***\n",time.tv_sec);
		timeout(kh_main,0,1);
	}
	else {
#ifdef AUTOCLEAN
		kh_cleanup();
#endif AUTOCLEAN
		cprintf("khammer: *** STOP  TEST @ %d ***\n",time.tv_sec);
		khfreq = 0;
	}
}

#define MAXPTR	8
#define MAXSIZ	16

struct allocs {
	int size;
	int count;
	caddr_t ptr[MAXPTR];
} allocs[MAXSIZ] = {
	{50}, {1024}, {1225}, {490}, {1000}, {48}, {2048}, {300},
	{900}, {90}, {1022}, {256}, {5}, {1120}, {163}, {102}
};

int tot_allocs = 0;
int tot_deallocs = 0;
struct all_chk {
	char main;
	char alloc;
	char dealloc;
	char lastop;
	caddr_t lastaddr;
	struct timeval all_time;
} all_chk;
char altype[MAXSIZ] = {1,1,0,1,0,0,1,1,0,0,1,0,1,0,1,0};

kh_main (na)
{
register long seedtime;
int seed2;		/* size index */
int seed3;		/* allocate/deallocate index */

	if (!khfreq)
		return;

	if (all_chk.main) {
		printf("kh_main: recursively entered\n");
		return;
	}
	all_chk.main++;
	seedtime = time.tv_usec;
	seed2 = (seedtime >> 8) & 0xf;
	seed3 = (seedtime >> 12) & 0xf;
	
	if (altype[seed3])
		kh_alloc(seed2);
	else
		kh_dealloc(seed2);

	if (khcleanup)
		kh_cleanup();

	if (khfreq)
		timeout(kh_main,0,khfreq);
	all_chk.main--;
}

kh_alloc(seed)
int seed;
{
register i,j;
register struct allocs *ap = &allocs[seed];
register caddr_t *cp;
register caddr_t tmp;
register rollover = MAXSIZ - seed;
caddr_t km_alloc();

	all_chk.alloc++;
	for (i = 0; i < MAXSIZ;) {
		cp = ap->ptr;
		if (ap->count < MAXPTR) {
			for (j = 0; j < MAXPTR; j++, cp++) {
				if (*cp == NULL)
					goto found1;
			}
			cprintf("kh_alloc: count messed up, cancel test\n");
			khfreq = 0;
			break;;
		}
		if (++i > rollover)
			ap = &allocs[0];
		else
			ap++;
	}

	all_chk.alloc--;
	return;

found1:
	if ((*cp = km_alloc(ap->size, KM_CLRSG)) == NULL) {
		cprintf("kh_alloc: No more memory, cancel test: time = %d\n",time.tv_sec);
		khfreq = 0;
		all_chk.alloc--;
		return;
	}
	ap->count++;
	tot_allocs++;
	if ((tot_allocs & ~0x7) == 0) 
		for (i = 0, tmp = *cp; i < ap->size; i += 512, tmp += 512) {
			if (*tmp != 0) {
				cprintf("kh_alloc: block not cleared; cancel test\n");
				khfreq = 0;
				break;
			}
			*tmp = 1;
		}
	all_chk.lastop = 1;
	all_chk.lastaddr = *cp;
	all_chk.all_time.tv_sec = time.tv_sec;	
	all_chk.all_time.tv_usec = time.tv_usec;	
	all_chk.alloc--;
}

kh_dealloc(seed)
int seed;
{
register i,j;
register struct allocs *ap = &allocs[seed];
register caddr_t *cp;

	all_chk.dealloc++;
	for (i = 0; i < MAXSIZ;) {
		cp = ap->ptr;
		if (ap->count > 0) {
			for (j = 0; j < MAXPTR; j++, cp++) {
				if (*cp != NULL)
					goto found1;
			}
			cprintf("kh_dealloc: count messed up, cancel test @ %d\n",time.tv_sec);
			khfreq = 0;
			break;
		}
		if (i++ == seed)
			ap = &allocs[MAXSIZ - 1];
		else
			ap--;
	}

	/* none left, just return */
	all_chk.dealloc--;
	return;

found1:
	km_free(*cp,ap->size);
	all_chk.lastaddr = *cp;
	*cp = NULL;
	ap->count--;
	tot_deallocs++;
	all_chk.lastop = 2;
	all_chk.dealloc--;
}

kh_cleanup() {
register i,j;
register count = 0;
register struct allocs *ap =allocs;
register caddr_t *cp;

	for (i = 0; i < MAXSIZ; i++, ap++) {
		for (j = 0, cp = ap->ptr; j < MAXPTR; j++, cp++)
			if (*cp) {
				km_free(*cp,ap->size);
				count++;
				*cp = NULL;
				tot_deallocs++;
			}
		ap->count = 0;
	}
	cprintf("khammer: Had to clean up %d entries\nallocs: %d\tdeallocs: %d\n",count,tot_allocs, tot_deallocs);
	tot_allocs = tot_deallocs = 0;
	khcleanup = 0;
	all_chk.main = all_chk.alloc = all_chk.dealloc = all_chk.lastop = 0;
	all_chk.lastaddr = NULL_PTR;
}


kstats(flg) 
int flg;
{
register i,j;
register struct allocs *ap;
register caddr_t *cp;
register union overhead *op;
register totals = 0;
extern union overhead *nextf[];	

	switch (flg) {

	case 0:
		cprintf("\nsize\tcount\t\t\tpointers\n");
		totals = 0;
		for (i=0, ap = allocs; i<MAXSIZ; i++,ap++) {
			if (ap->count == NULL)
				continue;
			cprintf("%d\t%d",ap->size,ap->count);
			for(j = 0, cp = ap->ptr;  j < MAXPTR;  j++, cp++) 
				if (*cp) {
					cprintf(" %x",*cp);
					totals++;
				}
			cprintf("\n");
		}
		if (totals)
			cprintf("\nTOTAL: %d\tallocs: %d\tdeallocs %d\n",totals,tot_allocs, tot_deallocs);
		else {
			cprintf("*** Nothing Allocated ***\n");
			cprintf("\n\t\tallocs: %d\tdeallocs %d\n",tot_allocs, tot_deallocs);
		}
		break;

	case 1:
		cprintf("\nbin #\t\tpointers\n");
		totals = 0;
		for (i = 0, op = nextf[0]; i < NBUCKETS; op = nextf[++i]) {
			cprintf("%d\t", i);
			while (op) {
				cprintf("%x ",op);
				totals++;
				op = op->ov_next;
			}
			cprintf("\n");
		}
		if (totals)
			cprintf("%d buffers in nextf\n\n",totals);
		else
			cprintf("\n");
		break;

	default:
		u.u_error = EINVAL;
		break;
	}
	
	return;
}
#endif VMTEST
