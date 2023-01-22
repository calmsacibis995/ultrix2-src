#ifndef lint
static char *sccsid = "@(#)vm_smem.c	1.7	ULTRIX	10/3/86";
#endif lint

/***********************************************************************
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
 **********************************************************************/
/*
 *
 *   Modification history:
 *
 * 29 Apr 86 -- depp
 *	converted to locking macros from calls routines
 *
 *	smlock/smunlock/smwait have are now macros SM_LOCK/SM_UNLOCK/SM_WAIT
 *	and are defined in /sys/h/vmmac.h
 *
 * 03 Jan 86 -- depp
 *	In "smfree" moved the call to smunlink, so that the memory is
 *	properly deallocated before the SMS is unlinked.
 *
 * 11 Nov 85 -- depp
 *	Removed all conditional compiles for System V IPC.
 *
 * 30 Sep 85 -- depp
 *	Added shared memory segment locking
 *
 * 19 Jul 85 -- depp
 *	Minor cleanup
 *
 * 11 Mar 85 -- depp		SHMEM
 *	1) - this file provides the virtual memory support for	
 *		the shared memory IPC (uipc_smem.c). It is a	
 *		direct analog of vm_text.c since the Ultrix	
 *		design for shared memory segments is to handle	
 *		them the same as shared text segments.		
 *		LeRoy Fundingsland    1/16/85    DEC		
 *								
 */

#include "../vax/mtpr.h"
#include "../machine/pte.h"

#include "../h/param.h"		/* includes types.h & vax/param.h */
#include "../h/dir.h"
#include "../h/user.h"		/* includes errno.h & smem.smpt.h */
#include "../h/proc.h"
#include "../h/seg.h"
#include "../h/cmap.h"
#include "../h/vm.h"

#include "../h/ipc.h"
#include "../h/shm.h"


extern struct smem smem[];
extern struct sminfo sminfo;
extern smtot;


/* SMFREE - relinquish use of the shared memory segment of a	*/
/*	process.						*/
smfree(sp)
    register struct smem *sp;	/* shared memory header ptr	*/
{
	register struct proc *p;
	register int smsize;		/* in clicks		*/

	if(sp == NULL)
		return;
	p = u.u_procp;
	SM_LOCK(sp);
	if(--sp->sm_count == 0  &&  (sp->sm_perm.mode & SM_DEST)){
		smsize = clrnd((int)btoc(sp->sm_size));

			/* free the memory for this SMS		*/
		sp->sm_rssize -= vmemfree(sp->sm_ptaddr, smsize);
		if(sp->sm_rssize != 0)
			panic("smfree rssize");

		while (sp->sm_poip)
			sleep((caddr_t)&sp->sm_poip, PSWP+1);
		smunlink(p, sp);

			/* free the page table for this SMS	*/
		wmemfree((caddr_t)sp->sm_ptaddr,
					(sizeof (struct pte)) * smsize);

			/* free the swap space for this SMS	*/
		vssmfree(sp, (long)smsize);
		smtot -= smsize;

		sp->sm_perm.mode = 0;
		sp->sm_perm.seq++;
		if(((int)(sp->sm_perm.seq * sminfo.smmni + (sp - smem)))
								< 0)
			sp->sm_perm.seq = 0;

		sp->sm_flag &= ~SMLOCK;
	} else {
		sp->sm_flag &= ~SMLOCK;
		smccdec(sp, p);
	}
}


/* SMCCDEC - shared memory core-count decrement. Decrement	*/
/*	the in-core usage count of a shared data segment. When	*/
/*	it drops to zero, free the core space.			*/
smccdec(sp, p)
    register struct smem *sp;
    register struct proc *p;{
	register int i, smsize;
	register struct pte *pte;

	if (sp == NULL  ||  sp->sm_ccount == 0)
		return;
	SM_LOCK(sp);

	/*
	 * this process is detaching from this SMS (due to exit, swapout, 
	 * or detach) so if the modify bit is on for any of the SMS PTEs 
	 * we must be sure to propogate it back to the primary PTEs before 
	 * the copy-PTEs are zeroed.  Note: Since the primary PTEs are only
	 * table entries and are not used by the hardware, an TBIS is not
	 * required.
	 */
	for(i=0; i < sminfo.smseg; i++)
		if(p->p_sm[i].sm_p == sp)
			break;
	if (i == sminfo.smseg)
		panic("smccdec: smseg");
	smsize = clrnd(btoc(sp->sm_size));
	pte = p->p_p0br + p->p_sm[i].sm_spte;
	for(i=0; i < smsize; i+=CLSIZE,pte+=CLSIZE){
		register int j;
		if(dirtycl(pte)){
			*(int *)(sp->sm_ptaddr + i) |= PG_M;
			distcl(sp->sm_ptaddr + i);
		}
		for(j=0; j < CLSIZE; j++)
			*(int *)(pte+j) = 0;
	}

	if (--sp->sm_ccount == 0 && !(sp->sm_flag & SMNOSW)) {

		vsswap(sp, sp->sm_ptaddr, CSMEM, 0, smsize,
						(struct dmap *)0);
		if(sp->sm_rssize != 0)
			panic("smccdec: rssize");
	}
	smunlink(p, sp);
	SM_UNLOCK(sp);
}


/* SMLINK - Add a process to those sharing a shared memory	*/
/*	segment by getting the page tables and then linking to	*/
/*	sm_caddr.						*/
smlink(p, sp)
    register struct proc *p;
    register struct smem *sp;{
	register int smindex;

	for(smindex=0; smindex < sminfo.smseg; smindex++)
		if(p->p_sm[smindex].sm_p == sp)
			break;
	if(smindex >= sminfo.smseg)
		panic("smlink");	/* cannot happen	*/

			/* vinitsmpt() expects that the		*/
			/* sm_spte and sm_pflag fields have	*/
			/* already been set within the proper	*/
			/* "p_sm" struct for this proc.		*/
	vinitsmpt(p, sp);
	p->p_sm[smindex].sm_link = sp->sm_caddr;
	sp->sm_caddr = p;
	sp->sm_ccount++;
}


/* SMUNLINK - unlink the given process from the linked list of	*/
/*	processes sharing the given shared memory segment.	*/
smunlink(p, sp)
    register struct proc *p;
    register struct smem *sp;{
	register struct proc *q;
	register int p_smindex, q_smindex;

	if (sp == NULL  ||  sp->sm_caddr == NULL)
		return;

			/* find index of this shared memory	*/
			/* seg for this process.		*/
	for(p_smindex=0; p_smindex < sminfo.smseg; p_smindex++)
		if(p->p_sm[p_smindex].sm_p == sp)
			break;
			/* return gracefully if not found	*/
			/* because sp may not be attached when	*/
			/* IPC_RMID is asserted.		*/
	if(p_smindex >= sminfo.smseg)
		return;

			/* handle the special case where "p" is	*/
			/* at the beginning of the linked list.	*/
	if (sp->sm_caddr == p) {
		sp->sm_caddr = p->p_sm[p_smindex].sm_link;
		p->p_sm[p_smindex].sm_link = 0;
		return;
	}

			/* now handle the general case		*/

			/* find index of shared memory seg	*/
			/* for first process in list.		*/
	q = sp->sm_caddr;
	for(q_smindex=0; q_smindex < sminfo.smseg; q_smindex++)
		if(q->p_sm[q_smindex].sm_p == sp)
			break;
	if(q_smindex >= sminfo.smseg)
		panic("smunlink #1");

	while(q->p_sm[q_smindex].sm_link){
		if (q->p_sm[q_smindex].sm_link == p) {
			q->p_sm[q_smindex].sm_link =
					p->p_sm[p_smindex].sm_link;
			p->p_sm[p_smindex].sm_link = 0;
			return;
		}
			/* check next entry in linked list	*/
		q = q->p_sm[q_smindex].sm_link;

			/* find index of shared memory seg	*/
			/* for this process.			*/
		for(q_smindex=0; q_smindex < sminfo.smseg; q_smindex++)
			if(q->p_sm[q_smindex].sm_p == sp)
				break;
		if(q_smindex >= sminfo.smseg)
			panic("smunlink #2");
	}
	panic("lost shared memory");
}


/* SMREPL - Replace p by q in a shared memory incore linked	*/
/*	list. Used by vfork() (vpassvm()) internally.		*/
smrepl(p, q, smindex)
    register struct proc *p, *q;{
	register struct smem *sp;

	if((sp = q->p_sm[smindex].sm_p) == NULL)
		return;

	smunlink(p, sp);
	q->p_sm[smindex].sm_link = sp->sm_caddr;
	sp->sm_caddr = q;
}
