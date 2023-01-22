#ifndef lint
static char *sccsid = "@(#)uipc_smem.c	1.15	ULTRIX	1/29/87";
#endif lint

/***********************************************************************
 *									*
 *			Copyright (c) 1985,86 by			*
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
 **********************************************************************/
/*
 *
 *   Modification history:
 *
 * 15 Dec 86 -- depp
 *	Fixed process SM page table swap error
 *
 * 18 Aug 86 -- depp
 *	Fixed a couple of range checking bugs in "smat()"
 *
 * 29 Apr 86 -- depp
 *	converted to locking macros from calls routines
 *
 * 12-Feb-86 -- jrs
 *	Added calls to tbsync() for mp translation buffer control
 *
 * 03 Jan 86 -- depp
 *	Modified "smat" to insure that multiple attaches on the same 
 *	shmid can not be done.  If an attach is attempted on an attached SMS,
 *	smat silently returns the address of the SMS.
 *
 *	Also, in "smdt", smfree was moved before the clearing of the detached
 *	PTEs, so that the modification bit (PG_M) could be properly
 *	propagated to the global PTEs.
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 * 30 Sep 85 -- depp
 *	Added shared memory segment locking to a number of routines
 *
 * 26 Jul 85 -- depp
 *	Insured that an error will be returned in smat if the process
 *	has advised that pageouts should be sequential (SSEQL)
 *
 * 19 Jul 85 -- depp
 *	Cleaned up some shared memory code.  Renamed smexec to smclean.
 *	Removed some commented out code.
 *
 * 29 Apr 85 -- depp
 *	Removed the test for maximum shared memory to be allocated for
 *	system, as virtual memory eliminates the need.
 *
 * 24 Apr 85 -- depp
 *	Fixed bug in smat, the beginning of SM segments was incorrectly 
 *	calculated.
 *
 * 11 Mar 85 -- depp
 *	1) - this file ported from System V and modified to	 
 *		handle the very different Ultrix shared memory	 
 *		approach.					 
 *		LeRoy Fundingsland    1 10 85    DEC		 
 *
 */
#include "../vax/mtpr.h"
#include "../machine/pte.h"

#include "../h/param.h"		/* includes types.h & vax/param.h */
#include "../h/dir.h"
#include "../h/user.h"		/* includes errno.h		*/
#include "../h/proc.h"
#include "../h/seg.h"
#include "../h/cmap.h"
#include "../h/vm.h"

#include "../h/ipc.h"
#include "../h/shm.h"


int	smtot;		/* total shared memory currently used (clicks)*/

extern struct timeval	time;		/* system idea of date */
extern struct smem smem[];
extern struct sminfo sminfo;
extern int maxdsiz;

struct smem	*ipcget(),
		*smconv();


/* SMAT - shared memory attach system call. Attach the		*/
/*	specified shared memory segment to the current process.	*/
smat(){
	register struct a {
		int	smid;
		char	*addr;
		int	flag;
	} *uap = (struct a *)u.u_ap;
	register struct smem *sp;	/* shared memory header ptr */
	register struct proc *p;
	register int *seg, *sm;
	register int segbeg, segend;
	int i, pflag, smindex;
	struct pte *ptaddr;
	int smsize;		/* SM size in CLSIZE chunks	*/
	size_t osms;		/* SM size before this attach */
	int p0lr, p1lr;

	if((sp = smconv(uap->smid, SM_DEST)) == NULL)
		return;
	if(ipcaccess(&sp->sm_perm, SM_R))
		return;
	if((uap->flag & SM_RDONLY) == 0)
		if(ipcaccess(&sp->sm_perm, SM_W))
			return;
	p = u.u_procp;

	/* Shared memory is currently not supported with SSEQL advisory */
	if (p->p_flag & SSEQL) {
		u.u_error = EINVAL;
		return;
	}

	i = -1;
	for(smindex=0; smindex < sminfo.smseg; smindex++) {
		if (i == -1 && p->p_sm[smindex].sm_p == 0) {
			i = smindex;
			continue;
		}
		
		/* if already attached, then silently return (for now) */
		if (p->p_sm[smindex].sm_p == sp) {
			u.u_r.r_val1 = ctob(p->p_sm[smindex].sm_spte);
			return;
		}
	}

	/* No open slots?? */
	if ((smindex = i) == -1) {
		u.u_error = EMFILE;
		return;
	}
	
	if(uap->flag & SM_RND)
		uap->addr = (char *)((u_int)uap->addr & ~(SMLBA - 1));

	/* Check for page alignment and containment within P0 */
	if((int)uap->addr & (ctob(CLSIZE) - 1) ||
			(int)uap->addr & 0xc0000000 ||
			((int)uap->addr + sp->sm_size) & 0xc0000000){
		u.u_error = EINVAL;
		return;
	}

	/* Is the requested address too high? */
	if (((int)uap->addr + sp->sm_size) >= ctob(maxdsiz)) {
		u.u_error = EINVAL;
		return;
	}

	/* An address of 0 places the shared memory	*/
	/* into a first fit location.			*/
	smsize = clrnd(btoc(sp->sm_size));

	if(uap->addr == NULL){
		seg = ((int *)mfpr(P0BR)) + u.u_tsize +
					u.u_dsize + sminfo.smbrk;
		seg = (int *)clrnd((int)seg);
		segend = (int)(((int *)mfpr(P0BR)) + mfpr(P0LR));
		segbeg = NULL;

		for(; seg < (int *)segend; seg+=CLSIZE){
			if (*seg & (PG_ALLOC | PG_FOD)) {
				if (segbeg &&
				(seg - (int *)segbeg) >= smsize)
					break;
				else
					segbeg = NULL;
			} else
                                if (segbeg == NULL)
                                        segbeg = (int)seg;
		}
		if (segbeg)
			segbeg = (int *)segbeg - (int *)mfpr(P0BR);
		else {
			segbeg = mfpr(P0LR);
			if (p->p_smbeg == 0)
				segbeg += clrnd(sminfo.smbrk);
		}
	} else {

			/* Check to make sure segment does not	*/
			/* overlay any valid segments.		*/

			/* "uap->addr" was tested or rounded to	*/
			/* cluster boundary in code above	*/
		segbeg = btop(uap->addr);
		if (isatsv(p,segbeg)) {
			u.u_error = EINVAL;
			return;
		}
		if (vtodp(p,segbeg) < p->p_dsize) {
			u.u_error = EINVAL;
			return;
		}
		if (segbeg < (segend = mfpr(P0LR))) {
			if((segbeg + smsize) < segend)
				segend = segbeg + smsize;
			ptaddr = (struct pte *)mfpr(P0BR) + segbeg;
			for(i=0; i < segend-segbeg; i+=CLSIZE){
				if (*(int *)(ptaddr+i) & (PG_ALLOC | PG_FOD)) {
					u.u_error = ENOMEM;
					return;
				}
			}
		}
	}

	/* Need to increase the size of the page table,	*/
	/* allocate and clear.				*/
	p0lr = u.u_pcb.pcb_p0lr & ~AST_CLR;
	p1lr = u.u_pcb.pcb_p1lr & ~PME_CLR;
	segend = segbeg + smsize;
	osms = p->p_smsize;
	if (p->p_smbeg == 0 || p->p_smbeg > segbeg)
		p->p_smbeg = segbeg;
	if (p->p_smend < segend){
		p->p_smend = segend;
		p->p_smsize = u.u_smsize = segend-u.u_tsize-u.u_dsize;
	}

	if (p0lr < segend) {
		/* Compute the end of the text+data regions and	*/
		/* the beginning of the stack region in the	*/
		/* page tables, and expand the page tables if	*/
		/* necessary.					*/
		register struct pte *p0, *p1;
		register int change;

		p0 = u.u_pcb.pcb_p0br + p0lr;
		p1 = u.u_pcb.pcb_p1br + p1lr;
		if ((change = segend - p0lr) > p1 - p0)
			ptexpand(clrnd(ctopt(change - (p1 - p0))),
						u.u_dsize, u.u_ssize, osms);
		seg = (int *)p->p_p0br + p0lr;
		for (i=0; i < change; i++)
			*seg++ = 0;

		setp0lr(segend);
	}

			/* Copy the shared memory segment in	*/
	sm = (int *)sp->sm_ptaddr;
	seg = (int *)p->p_p0br + segbeg;
	pflag = ((uap->flag & SM_RDONLY) ? PG_URKW : PG_UW);
	for (i=0; i < smsize; i++)
		if (((struct fpte *)sm)->pg_fod)
			*seg++ = *sm++ | pflag;
		else
			*seg++ = *sm++ | PG_ALLOC | pflag;


	p->p_sm[smindex].sm_p = sp;
	p->p_sm[smindex].sm_spte = segbeg;
	p->p_sm[smindex].sm_pflag = pflag;
	p->p_sm[smindex].sm_link = sp->sm_caddr;
	sp->sm_caddr = p;
	sp->sm_count++;
	sp->sm_ccount++;
	u.u_r.r_val1 = ctob(segbeg);
	sp->sm_atime = (time_t) time.tv_sec;
	sp->sm_lpid = p->p_pid;
}


/* SMCONV - Convert user supplied smid into a ptr to the	*/
/*	associated shared memory header.			*/
struct smem *
smconv(s, flg)
    register int s;		/* smid */
    int flg;{		/* error if matching bits are set in mode */
	register struct smem *sp;	/* ptr to associated header */

	sp = &smem[s % sminfo.smmni];
	if((sp->sm_perm.mode & IPC_ALLOC) == 0  ||
			sp->sm_perm.mode & flg  ||
			s / sminfo.smmni != sp->sm_perm.seq) {
		u.u_error = EINVAL;
		return(NULL);
	}
	return(sp);
}


/* SMCTL - shared memory control operations system call.	*/
/*	Provides three functions to the user: "stat" of the	*/
/*	"smem" struct, set the user-group-mode for the segment,	*/
/*	and allow the user to remove the "smem" and destroy the	*/
/*	shared memory segment.					*/
smctl(){
	register struct a {
		int	smid,
			cmd;
		struct smem *arg;
	} *uap = (struct a *)u.u_ap;
	register struct smem *sp;	/* shared memory header ptr */
	register struct p_sm *psmp;	/* see proc.h		*/
	register struct proc *p;	/* proc pointer */
	register int i;
	struct smem ds;			/* hold area for IPC_SET */

	if((sp = smconv(uap->smid, (uap->cmd == IPC_STAT)?0:SM_DEST)) ==
								NULL)
		return;
	u.u_r.r_val1 = 0;
	switch(uap->cmd){

		/* Remove shared memory identifier. */
	case IPC_RMID:
		if(u.u_uid != sp->sm_perm.uid  &&  
				u.u_uid != sp->sm_perm.cuid  &&
				!suser())
			return;
		sp->sm_ctime = (time_t) time.tv_sec;
		sp->sm_perm.mode |= SM_DEST;

		/* Change key to "private" so old key can be	*/
		/* reused without waiting for last detach. Only	*/
		/* allowed accesses to this segment now are	*/
		/* smdt() and smctl(IPC_STAT). All others will	*/
		/* give "bad smem".				*/
		sp->sm_perm.key = IPC_PRIVATE;

			/* If there are no processes attached	*/
			/* to this SMS then delete it.		*/
			/* If there are attached processes then	*/
			/* the SM_DEST flag will cause the SMS	*/
			/* to be deleted on the last detach.	*/
		if(sp->sm_count == 0){
			sp->sm_count++;
			smfree(sp);
		}
		return;

		/* Set ownership and permissions. */
	case IPC_SET:
		if(u.u_uid != sp->sm_perm.uid  &&  
				u.u_uid != sp->sm_perm.cuid  &&
				!suser())
			return;
		if(copyin(uap->arg, &ds, sizeof(ds))){
			u.u_error = EFAULT;
			return;
		}
		sp->sm_perm.uid = ds.sm_perm.uid;
		sp->sm_perm.gid = ds.sm_perm.gid;
		sp->sm_perm.mode = (ds.sm_perm.mode & 0777) |
					(sp->sm_perm.mode & ~0777);
		sp->sm_ctime = (time_t) time.tv_sec;
		return;

		/* Get shared memory data structure. */
	case IPC_STAT:
		if(ipcaccess(&sp->sm_perm, SM_R) != 0)
			return;
		if(copyout(sp, uap->arg, sizeof(*sp)))
			u.u_error = EFAULT;
		return;

	case SHM_LOCK:
		if (!(suser()))
			return;

		/* find segment	in proc table	*/
		p = u.u_procp;
		psmp = &p->p_sm[0];
		for (i=0; i < sminfo.smseg; i++, psmp++)
			if (psmp->sm_p == sp)
				break;
		if (i >= sminfo.smseg || psmp->sm_lock) {
			u.u_error = EINVAL;
			return;
		}
		SM_LOCK(sp);
		psmp->sm_lock++;
		sp->sm_flag |= SMNOSW;
		sp->sm_lcount++;
		SM_UNLOCK(sp);
		return;

	case SHM_UNLOCK:
		if (!(suser()))
			return;

		/* find segment in proc table */
		p = u.u_procp;
		psmp = &p->p_sm[0];
		for (i=0; i < sminfo.smseg; i++, psmp++)
			if (psmp->sm_p == sp)
				break;
		if (i >= sminfo.smseg) {
			u.u_error = EINVAL;
			return;
		}

		SM_LOCK(sp);
		/* If not locked or this proc didn't lock it; error */
		if (!(psmp->sm_lock) || !(sp->sm_flag & SMNOSW)) {
			u.u_error = EINVAL;
			SM_UNLOCK(sp);
			return;
		}

		/* unlock SMS (at least from this proc's point of view */
		psmp->sm_lock = 0;
		if (--(sp->sm_lcount) == 0)
			sp->sm_flag &= ~SMNOSW;
		SM_UNLOCK(sp);
		return;

	default:
		u.u_error = EINVAL;
		return;
	}
}


/* SMDT - shared memory detach system call. Detach the		*/
/*	specified shared memory segment from the current	*/
/*	process.						*/
smdt(){
	struct a {
		char	*addr;
	} *uap = (struct a *)u.u_ap;
	register struct p_sm *psmp;	/* see proc.h		*/
	register struct smem *sp;
	register struct proc *p;
	register int *seg;
	register i, j;
	int segbeg;

	/* Check for page alignment		*/
	if ((int)uap->addr & (ctob(1) - 1)  ||
			(segbeg = btoc(uap->addr)) == 0) {
		u.u_error = EINVAL;
		return;
	}

	/* find segment				*/
	p = u.u_procp;
	psmp = &p->p_sm[0];
	for (i=0; i < sminfo.smseg; i++, psmp++)
		if (psmp->sm_p != NULL  &&  psmp->sm_spte == segbeg)
			break;
	if (i >= sminfo.smseg) {
		u.u_error = EINVAL;
		return;
	}
	sp = psmp->sm_p;

	/* if this process has SMS locked, then unlock */
	if (psmp->sm_lock) {
		psmp->sm_lock = 0;
		if (--(sp->sm_lcount) == 0)
			sp->sm_flag &= ~SMNOSW;
	}
	smfree(sp);

	/* clear the PTEs for this SMS		*/
	i = btoc(sp->sm_size);
	seg = (int *)mfpr(P0BR) + segbeg;
	while (i--)
		*seg++ = 0;
	mtpr(TBIA, 0);
	tbsync();

	psmp->sm_p = NULL;
	psmp->sm_spte = 0;
	sp->sm_dtime = (time_t) time.tv_sec;
	sp->sm_lpid = p->p_pid;
	p->p_smbeg = 0;
	p->p_smend = 0;

#define I_PLUS_SIZE	(i + btoc((psmp->sm_p)->sm_size))
	psmp = &p->p_sm[0];
	for (j=0; j < sminfo.smseg; j++, psmp++) {
		if (i = psmp->sm_spte) {
			if (p->p_smbeg) {
				if (p->p_smbeg > i)
					p->p_smbeg = i;
				if(p->p_smend < I_PLUS_SIZE)
					p->p_smend = I_PLUS_SIZE;
			} else {
				p->p_smbeg = i;
				p->p_smend = I_PLUS_SIZE;
			}
		}
	}
	if (p->p_smbeg == 0) {
		setp0lr(u.u_tsize + u.u_dsize);
		u.u_smsize = p->p_smsize = 0;
	}
	u.u_r.r_val1 = 0;
}


/* SMCLEAN - Called by vrelvm to handle shared			*/
/*	memory cleanup processing.				*/
smclean () {
	register struct p_sm *psmp;	/* see proc.h		*/
	register struct proc *p;
	register int *seg, *segend;	/* ptr's to pte		*/
	register int i;

			/* Detach all attached segments		*/
	p = u.u_procp;
	psmp = &p->p_sm[0];
	p->p_smbeg = 0;
	p->p_smend = 0;
	for(i=0; i < sminfo.smseg; i++, psmp++){
		if(psmp->sm_p == NULL)
			continue;
		/* if this process has SMS locked, then unlock */
		if (psmp->sm_lock) {
			psmp->sm_lock = 0;
			if (--((psmp->sm_p)->sm_lcount) == 0)
				(psmp->sm_p)->sm_flag &= ~SMNOSW;
		}
		smfree(psmp->sm_p);
		seg = (int *)mfpr(P0BR) + psmp->sm_spte;
		segend = seg + btoc((psmp->sm_p)->sm_size);
		while(seg < segend)
			*seg++ = 0;
		psmp->sm_p = NULL;
		psmp->sm_spte = 0;
	}
	p->p_smsize = 0;
	u.u_smsize = 0;

	setp0lr(u.u_tsize + u.u_dsize);
}



/* SMFORK - Called by newproc(sys/kern_fork.c) to handle shared	*/
/*	memory FORK processing.					*/
smfork(pp, cp)
    struct proc *pp,	/* ptr to parent proc table entry */
		*cp;{	/* ptr to child proc table entry */
	register struct p_sm	*cpsmp,	/* ptr to child p_sm	*/
				*ppsmp;	/* ptr to parent p_sm	*/
	register int i;

	/* Copy ptrs and update counts on any attached segments. */
	cpsmp = &cp->p_sm[0];
	ppsmp = &pp->p_sm[0];
	cp->p_smbeg = pp->p_smbeg;
	cp->p_smend = pp->p_smend;
	for(i=0; i < sminfo.smseg; i++, cpsmp++, ppsmp++){
		if(cpsmp->sm_p = ppsmp->sm_p){
			cpsmp->sm_p->sm_count++;
			cpsmp->sm_spte = ppsmp->sm_spte;
			cpsmp->sm_pflag = ppsmp->sm_pflag;
			cpsmp->sm_lock = 0;
			smlink(cp, cpsmp->sm_p);
		}
	}
}


int smdebug;
/* SMGET - get shared memory segment system call.		*/
smget()
{
	register struct a {
		int	key;
		int	size,
			smflg;
	} *uap = (struct a *)u.u_ap;
	register struct smem *sp;	/* shared memory header ptr */
	register int size, i;
	register struct pte *pte;
	int s;				/* ipcget status	*/
	struct fpte proto;
	int vmemall();

again:
	if((sp = ipcget(uap->key, uap->smflg, &smem[0], sminfo.smmni,
					sizeof(*sp), &s)) == NULL)
		return;
	if(sp->sm_flag & SMLOCK){
		SM_WAIT(sp);
		if (s)
			sp->sm_perm.mode = 0;
		goto again;
	}

	SM_LOCK(sp);
	u.u_procp->p_flag |= SKEEP;
	if(s){
			/* This is a new shared memory segment.	*/
			/* allocate memory and finish		*/
			/* initialization.			*/
		if(uap->size < sminfo.smmin  ||
		   uap->size > sminfo.smmax  ||
		   uap->size < 1) {
			u.u_error = EINVAL;
			sp->sm_perm.mode = 0;
			SM_UNLOCK(sp);
			return;
		}
		size = clrnd((int)btoc(uap->size));

			/* allocate the page table and memory	*/
			/* for this SMS.			*/
		sp->sm_size = uap->size;
		if(vgetsmpt(sp) == 0){
			u.u_error = ENOMEM;
			sp->sm_perm.mode = 0;
			SM_UNLOCK(sp);
			return;
		}
		pte = sp->sm_ptaddr;
		*(u_int*) &proto = (u_int) 0;
		proto.pg_fod = 1;
		proto.pg_fileno = PG_FZERO;
		for ( i = 0; i < size; i++, pte++) {
			*(u_int *)pte = *(u_int*) &proto;
		}
		mtpr(TBIA,0);
		tbsync();

		smtot += size;

			/* allocate swap space for this SMS	*/
		if (vssmalloc(sp) == NULL) {
			swkill(u.u_procp, "smalloc: no swap space");
			SM_UNLOCK(sp);
			return;
		}

		sp->sm_count = sp->sm_ccount = 0;
		sp->sm_atime = sp->sm_dtime = 0;
		sp->sm_ctime = (time_t) time.tv_sec;
		sp->sm_lpid = 0;
		sp->sm_cpid = u.u_procp->p_pid;
	} else
		if(uap->size  &&  uap->size > sp->sm_size) {
			u.u_error = EINVAL;
			SM_UNLOCK(sp);
			return;
		}

	u.u_procp->p_flag &= ~SKEEP;
	SM_UNLOCK(sp);
	u.u_r.r_val1 = sp->sm_perm.seq * sminfo.smmni + (sp - smem);
}


/* SMSYS - System entry point for SMAT, SMCTL, SMDT, and SMGET	*/
/*	system calls.						*/
smsys(){
	register struct a {
		u_int	id;
	} *uap = (struct a *)u.u_ap;
	int	smat(),
		smctl(),
		smdt(),
		smget();
	static int (*calls[])() = {smat, smctl, smdt, smget};

	if(uap->id > 3){
		u.u_error = EINVAL;
		return;
	}
	u.u_ap = &u.u_arg[1];
	(*calls[uap->id])();
}


/*
 *	DUMPPTE -- dumps the processes page table.  DEBUG only
 */
#ifdef DEBUG
dumppte(p,p0br,p0lr,p1lr,p1br)
    register struct proc *p;
    int *p0br, p0lr, p1lr, *p1br;{
	register i;
int j;

printf("dumppte:\n");
	printf("    p_tsize %d, p_dsize %d", p->p_tsize, p->p_dsize);
	printf(", p_smsize %d, p_ssize %d", p->p_smsize, p->p_ssize);
	printf(", p_szpt %d\n", p->p_szpt);
	p0lr &= ~AST_CLR;
	p1lr &= ~PME_CLR;
	printf("    p0br %x    p0lr %d    p1br %x    p1lr %d\n\n",
						p0br, p0lr, p1br, p1lr);
	for (i=0; i < p0lr; i++) {
		if ((i%8) == 0)
{
			printf("\n    ");
for(j=0; j < 100000; j++);
}
		printf("%x  ", *p0br++);
	}
	printf("\n\n\n");
}


#ifdef lint
/* ARGSUSED */
/* VARARGS */
printf( s ) char *s; {;}
#endif lint
#endif DEBUG
