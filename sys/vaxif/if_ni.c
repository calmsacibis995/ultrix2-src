/*
 * if_ni.c
 */

#ifndef lint
static char *sccsid = "@(#)if_ni.c	1.14	ULTRIX	3/31/87";
#endif lint

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
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

#include "bvpni.h"
#if NBVPNI > 0 || defined(BINARY)

/*
 * 31-Mar-87  -- lp	
 *			Put apa back on a reset for decnet.
 * 06-Feb-87  -- lp
 *			Check return of ifproto_type to gaurantee that
 *			we dont jump to 0.
 * 28-Jan-87  -- lp
 *			Cleanup as LINT showed some extra
 *			variables.
 *
 * 12-Dec-86  -- lp	Post FT-v2.0. Added dpaddr, Bumped up 
 *			freeq0 allocation.
 *
 * 23-Oct-86  -- lp	Type 2 mbuf are forced to return on respq.
 *
 * 2-Sep-86   -- lp	Cleanup. Bugfix for long (improperly chained)
 *			packets.
 *
 * 7-Aug-86   -- lp	Removed some printf's ('I' baseline).
 *
 * 7-Jul-86   -- lp	Fixed a timing problem in niattach.
 *
 * 5-Jun-86   -- lp 	Fixed a little bug in reset code.
 *
 * 5-Jun-86   -- jaw 	changes to config.
 *
 * 22 May 86 -- bjg
 *	Include types.h and errlog.h for error logging.
 *
 * 21 May 86 -- lp
 *	Reworked sptdb routine to allow clear prior to setting. Bugfixes
 *	for decnet. Errlog for SUME errors.
 *
 * 08 May 86 -- lp
 *	Trailer packets work. General cleanup. Vaddr no longer saved
 *	in receive packets.
 *
 * 09 Apr 1986 -- lp
 * 	DEC AIE/NI ethernet driver
 *		By 
 * 	Larry Palmer (decvax!lp).
 *	(rev 40 or higher aie firmware needed)
 *
 */

#include "../data/if_ni_data.c"
#include "../h/types.h"
#include "../h/errlog.h"

extern struct protosw *iftype_to_proto(), *iffamily_to_proto();
struct	mbuf *niget();
int	niattach(), bvpniintr(), niprobe();
int	niinit(),nioutput(),niioctl(),nireset(), ni_ignore;
u_char  ni_multi[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00};
u_char  ni_notset[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
u_short nistd[] = { 0 };
#define svtopte(x) (unsigned long)(&Sysmap[btop((int)x&~VA_SYS)])
#define ptetosv(x) (unsigned long)((unsigned)ptob((struct pte *)(x)-&Sysmap[0])|VA_SYS)
#define vaddr(y) (ptetosv((y)->pt_addr)+(y)->offset)

int nidebug = 0;

/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.  We get the ethernet address here.
 * We initialize big mbuf storage for receives here.
 */
niattach(ni)
	struct ni *ni;
{
	register struct ni_softc *ds = &ni_softc[ni->unit];
	register struct ifnet *ifp = &ds->ds_if;
	int i=0;
	struct sockaddr_in *sin;

	ds->ds_devid = 0;

	ifp->if_unit = ni->unit;
	ifp->if_name = "ni";
	ifp->if_mtu = ETHERMTU;
	ifp->if_flags |= IFF_BROADCAST|IFF_DYNPROTO;

	/* 
	 * Set ni to initialized state
	 */
	i=0;
	while((ni->ni_regs->ps&PS_STATEMASK) != PS_UNDEFINED) {
	    if(++i > 200) {
		printf("ni%d in wrong state\n", ni->unit);
		return;
	    }
	    DELAY(100000);
	}

	DELAY(200000);
	ni->ni_regs->ps &= ~PS_OWN;
	ni->ni_regs->pc &= ~PC_OWN;

	/* Go to initialized state */
	ni->ni_regs->pc = ni->phys_pqb|PC_INIT|PC_OWN;
	DELAY(1000000);
	while((ni->ni_regs->pc&PC_OWN)) 
		;
	i=0;
	while((ni->ni_regs->ps&PS_INITIALIZED) == 0) {
		if(i++ > 15) {
			printf("ni%d Cannot initialize\n", ni->unit);
			return;
		} else 
			DELAY(1000000);
	}
	ni->ni_regs->ps &= ~PS_OWN;

	/*
	 * Fill the multicast address table with unused entries (broadcast
	 * address) so that we can always give the full table to the device
	 * and we don't have to worry about gaps.
	 */
	for(i=0; i<NMULTI; i++)
		bcopy(ni_multi, ds->ds_multi[i], 8);

	/* Since we cant free mbuf storage we only do this once! */
	for(i=NI_FREEQ_1; i<(NI_FREEQ_1+NI_FREEQ_2); i += NI_RBUF) {
		ni->mbuf_clusters[i+1] = (char *) m_clalloc(1, MPG_SPACE);
	}

	sin = (struct sockaddr_in *)&ifp->if_addr;
	sin->sin_family = AF_INET;

	ifp->if_init = niinit;
	ifp->if_output = nioutput;
	ifp->if_ioctl = niioctl;
	ifp->if_reset = nireset;
	if_attach(ifp);
}

/*
 * Reset of interface after reset.
 */
nireset(unit)
	int unit;
{
	register struct ni *ni;
	register struct ni_softc *ds = &ni_softc[unit];
	register char *mem;
	register int i;
	struct ifnet *ifp = &ds->ds_if;

	if (unit >= nNI || (ni = &niinfo[unit]) == 0 || ni->alive == 0)
		return;
	printf("reset ni%d %x %x %x %x\n", unit, ni->ni_regs->pc,
		ni->ni_regs->ps, ni->ni_regs->pe, ni->ni_regs->pd);
	if((ni->ni_regs->ps&PS_STATEMASK) != PS_ENABLED) {

	/* Eat freeq 2 */
		while((mem = (char *)remqhi(&ni->freeq2, NI_MAXITRY)) != (char *)QEMPTY) {
			struct _bd *bde = (struct _bd *)ni->ni_pqb->ni.bdt_base;
			bde += ((struct ni_data *)mem)->cbufs[0].bdt_index;
			for(i=0; i<NI_RBUF; bde++, i++) {
				if(i != 1)
					km_free(vaddr(bde), bde->buf_len);	
			}
			km_free(mem, NI_DQHEAD+NI_DGRLEN);
		}
	/* Eat freeq0 & freeq1 */
		while((mem = (char *)remqhi(&ni->freeq0, NI_MAXITRY)) != (char *)QEMPTY)
			km_free(mem, sizeof (struct ni_msg));
		while((mem = (char *)remqhi(&ni->freeq1, NI_MAXITRY)) != (char *)QEMPTY)
			km_free(mem, NI_DQSIZE);

	/* Eat the respq */
		while((mem = (char *)remqhi(&ni->respq, NI_MAXITRY)) != (char *)QEMPTY) {
			if(((struct ni_data *)mem)->opcode == DGREC) {
			struct _bd *bde = (struct _bd *)ni->ni_pqb->ni.bdt_base;
			bde += ((struct ni_data *)mem)->cbufs[0].bdt_index;
			for(i=0; i<NI_RBUF; bde++, i++) {
				if(i != 1)
					km_free(vaddr(bde), bde->buf_len);	
			}
			km_free(mem, NI_DQHEAD+NI_DGRLEN);
			} else if(((struct ni_data *)mem)->opcode == SNDDG) {
				km_free(mem, NI_DQSIZE);
			} else {
				km_free(mem, sizeof (struct ni_msg));
			}
		}
			

		if((ni->ni_regs->ps&PS_STATEMASK) == PS_STOPPED) {
			ni->ni_regs->ps &= ~PS_OWN;
			ni->ni_regs->pc = PC_RESTART|PC_OWN;
			DELAY(1000000);
			while((ni->ni_regs->pc&PC_OWN)) 
				;
		}
		ni->ni_regs->ps &= ~PS_OWN;
		/* 
	 	* Set ni to initialized state
	 	*/
		i=0;
		while((ni->ni_regs->ps&PS_STATEMASK) != PS_UNDEFINED) {
	    	if(++i > 200) {
			printf("ni%d in wrong state\n", ni->unit);
			return(0);
	    	}
	    	DELAY(100000);
		}
		ni->ni_regs->ps &= ~PS_OWN;
		/* Go to initialized state */
		ni->ni_regs->pc = ni->phys_pqb|PC_INIT|PC_OWN;
		DELAY(100000);
		while((ni->ni_regs->pc&PC_OWN)) 
			;
		if((ni->ni_regs->ps&PS_INITIALIZED) == 0) {
			printf("ni%d Cannot initialize\n", ni->unit);
			return;
		}
		ni->ni_regs->ps &= ~PS_OWN;
		ni->ni_regs->pe = 0;
		ni->ni_regs->pd = 0;
		ifp->if_flags &= ~(IFF_RUNNING);
		untimeout(niinit, unit);
		timeout(niinit, unit, 1);
		return(0);
	}
	return(1);
}

/*
 * Initialization of interface; clear recorded pending
 * operations.
 */
niinit(unit)
	int unit;
{
	register struct ni_softc *ds = &ni_softc[unit];
	register struct ni *ni = &niinfo[unit];
	register struct nidevice *addr;
	struct ifnet *ifp = &ds->ds_if;
	int s,i,empty;

	/* not yet, if address still unknown */
	/* DECnet must set this somewhere to make device happy */

	if (ifp->if_addrlist == (struct ifaddr *)0)
			return;
	if (ifp->if_flags & IFF_RUNNING)
		return;

	/* 
	 * Set ni to enabled state. Still dont have freeq ready.
	 */

	addr = (struct nidevice *)ni->ni_regs;
	while((addr->pc&PC_OWN))
		;
	addr->pc = PC_ENABLE|PC_OWN;
	/* Should interrupt here */
	while((addr->pc&PC_OWN))
		;
	while((addr->ps&PS_OWN))
		;
	i = 0;
	while((addr->ps&PS_STATEMASK) != PS_ENABLED) {
		if(i++ > 500000) {
			printf("ni%d Cannot enable\n", ni->unit);
			return;
		}
	}
	addr->ps &= ~PS_OWN;

	/* 
	 * Setup message freeq Just a few so its not empty 
	 */
	{
		struct ni_msg *ni_msg;
		for(i=0; i<6; i++) {
		ni_msg = (struct ni_msg *)km_alloc(sizeof (struct ni_msg),KM_CLRSG);
		if((empty=insqti(ni_msg, &ni->freeq0, NI_MAXITRY)) > 0)
			printf("insqti failed\n");
		else if(empty == QEMPTY) {
			while((addr->pc&PC_OWN))
				;
			addr->pc = PC_FREEQNE|PC_MFREEQ|PC_OWN;
			while((addr->pc&PC_OWN))
				;
		}
		}
	}
	/* Setup xmit buffers (empty) */
	for(i=0; i<NI_FREEQ_1; i += NI_NUMBUF) {
		struct ni_data *ni_data;
		struct _bd *bde;
		int j;
		ni_data = (struct ni_data *) km_alloc(NI_DQSIZE,KM_CLRSG);
		ni_data->status = 0;
		ni_data->dg_len = NI_DQSIZE-NI_DQHEAD;
		ni_data->dg_ptdb_index = 1;
		ni_data->opcode = SNDDG;
		for(j=0; j<NI_NUMBUF; j++) {	
			ni_data->cbufs[j].offset = 0;
			ni_data->cbufs[j].buffer_key = 1;	
			bde = (struct _bd *) ni->ni_pqb->ni.bdt_base;
			bde += i+j;
			bde->key = 1;
			bde->valid = 0;
			ni_data->cbufs[j].bdt_index = i+j;
		}
		if((empty=insqti(ni_data, &ni->freeq1, NI_MAXITRY)) > 0)
			printf("xmit insqti failed %d\n", i);
		else if(empty == QEMPTY) {
			while((addr->pc&PC_OWN))
				;
			addr->pc =  PC_FREEQNE|PC_DFREEQ|PC_OWN;
			while((addr->pc&PC_OWN))
				;
		}
	}
	/*
	 * Setup recv buffers 
	 */	
	/* Note that these have mbuf data areas associated */
	for(i=NI_FREEQ_1; i<(NI_FREEQ_1+NI_FREEQ_2); i += NI_RBUF) {
		struct ni_data *ni_data;
	  	struct _bd *bde;
	  	char *buffer;
		int j=0;
		ni_data = (struct ni_data *)
				km_alloc(NI_DQHEAD+NI_DGRLEN,KM_CLRSG);
		/* header + space for DGRLEN buffer names */
		ni_data->dg_len = NI_DGRLEN;
		ni_data->opcode = DGREC;
		ni_data->dg_ptdb_index = 2;
		for(j=0; j<NI_RBUF; j++) {
		bde = (struct _bd *) ni->ni_pqb->ni.bdt_base;
		bde += i+j;
/* Need to have 1st buffer be ether_header */
		if(j == 0) {
			buffer = (char *)
				km_alloc(sizeof(struct ether_header), KM_CLRSG);
			bde->buf_len = sizeof(struct ether_header);
			bde->pt_addr = svtopte(buffer);
			bde->offset = (unsigned)buffer&PGOFSET;
		} else if( j == 1) {
			buffer = (char *)ni->mbuf_clusters[i+j];
			bde->pt_addr = svtopte(buffer);
			bde->offset = (unsigned)buffer&PGOFSET;
			bde->buf_len = NI_MAXPACKETSZ;
		} else {
			buffer = (char *)km_alloc(512, KM_CLRSG);
			bde->buf_len = 512;
			bde->pt_addr = svtopte(buffer);
			bde->offset = (unsigned)buffer&PGOFSET;
		}
		bde->key = 1;
		bde->valid = 1;
		ni_data->cbufs[j].offset = 0;
		ni_data->cbufs[j].s_len = bde->buf_len;
		ni_data->cbufs[j].bdt_index = i+j;
		ni_data->cbufs[j].buffer_key = 1;
		}
		if((empty=insqti(ni_data, &ni->freeq2, NI_MAXITRY)) > 0)
			printf("recv insqti failed %d\n", i);
		else if(empty == QEMPTY) {
			while((addr->pc&PC_OWN))
				;
			addr->pc = PC_FREEQNE|PC_RFREEQ|PC_OWN;
			while((addr->pc&PC_OWN))
				;
		}
	}
	while((addr->ps&PS_OWN))
		;
	/* Write Parameters */
	{
		struct ni_msg *ni_msg;
		ni_msg = (struct ni_msg *)km_alloc(sizeof (struct ni_msg),KM_CLRSG);
		ni_msg->opcode = SNDMSG;
		ni_msg->status = 0;
		ni_msg->msg_len = sizeof(struct ni_param) + 6;
		ni_msg->ni_opcode = NIOP_WPARAM;

		/* Someone has set the apa at least once */
		if((bcmp(ds->ds_addr, ni_notset, 6) != 0))
		bcopy(ds->ds_addr,((struct ni_param *)&ni_msg->text[0])->apa, 6);
		((struct ni_param *)&ni_msg->text[0])->flags = NI_PAD;
		if((empty=insqti(ni_msg, &ni->comq0, NI_MAXITRY)) > 0)
			printf("insqti failed\n");
		else if(empty == QEMPTY) {
			while((addr->pc&PC_OWN))
				; 
			addr->pc = PC_CMDQNE|PC_CMDQ0|PC_OWN;
			while((addr->pc&PC_OWN))
				; 
		}
	}
	{
		struct ni_msg *ni_msg;
		ni_msg = (struct ni_msg *)km_alloc(sizeof (struct ni_msg),KM_CLRSG);
		ni_msg->opcode = SNDMSG;
		ni_msg->status = 0;
		ni_msg->msg_len = sizeof(struct ni_param) + 6;
		ni_msg->ni_opcode = NIOP_RCCNTR;
		if((empty=insqti(ni_msg, &ni->comq0, NI_MAXITRY)) > 0)
			printf("insqti failed\n");
		else if(empty == QEMPTY) {
			while((addr->pc&PC_OWN))
				; 
			addr->pc = PC_CMDQNE|PC_CMDQ0|PC_OWN;
			while((addr->pc&PC_OWN))
				; 
		}
	}
	/* Let all these commands complete */
	while((addr->ps & PS_OWN))
		;
	/* Hardware address not set yet */
	while((bcmp(ds->ds_addr, ni_notset, 6) == 0))
		;
	/* Set up PTDB's */
	ni_sptdb(ni,0,1,1,0,1);	/* #1 for send queue returns */
	ni_sptdb(ni,ETHERTYPE_IP,2,2,PTDB_UNK|PTDB_BDC,1); /* #2 all incoming */
	/* The board is up (ooo rah) */
	s = splimp();
	ds->ds_if.if_flags |= IFF_UP|IFF_RUNNING;
	nistart(unit);				/* queue output packets */
	splx(s);
}

/*
 * Setup output on interface.
 */
nistart(unit)
	int unit;
{
	register struct ni *ni = &niinfo[unit];
	register struct mbuf *m, *n;
	register struct _bd *bde;
	register struct ni_data *nid;
	register int len, curindex;
	struct mbuf *m_compress();
	struct ni_softc *ds = &ni_softc[unit];
	struct nidevice *addr = (struct nidevice *)ni->ni_regs;
        int empty;


	if((addr->ps&PS_STATEMASK) != PS_ENABLED) {
		cprintf("ni%d state(nistart) %x %x %x %x\n", unit, addr->ps, 
			addr->pe, addr->pc, addr->pd);
		if((addr->ps&PS_STATEMASK) == PS_UNDEFINED ||
		   (addr->ps&PS_STATEMASK) == PS_STOPPED)
			nireset(unit);
	}
	for(;;) {
		IF_DEQUEUE(&ds->ds_if.if_snd, m);
		if(m == 0)
			break;
		if((nid = (struct ni_data *)remqhi(&ni->freeq1, NI_MAXITRY))
			== (struct ni_data *)QEMPTY) {
			goto drop;
		}
		if(nid->mbuf_tofree) { /* Free last mbuf sent */
			m_freem((struct mbuf *)nid->mbuf_tofree);
			nid->mbuf_tofree = 0;
		}
		/* force a match on broadcasts */
		if((bcmp(mtod(m, struct ether_header *), ni_multi, 6) == 0))
			nid->R = 1;
		else
			nid->R = 0;

		for(n = m,curindex = 0; n; n = n->m_next) {
			curindex++;
			if(n->m_cltype == 2)
				nid->R = 1;
		}
		if(curindex >= NI_NUMBUF) {
			n = m;
			m = m_compress(n);
		}

		bde = (struct _bd *) ni->ni_pqb->ni.bdt_base;
		bde += nid->cbufs[0].bdt_index;

		nid->mbuf_tofree = (unsigned long)m;

		for(len=0, curindex=0;m; m = m->m_next,bde++) {
			bde->offset = mtod(m, unsigned)&PGOFSET;
			bde->pt_addr = svtopte(mtod(m, char *));
			bde->buf_len = m->m_len;
			bde->valid = 1;
			nid->cbufs[curindex].offset = 0;
			nid->cbufs[curindex].s_len = bde->buf_len;
			nid->cbufs[curindex].chain = 1;
			len += m->m_len;
			curindex++;
		}
		if(len < 64) { /* Last buffer may get something tacked on */
			(--bde)->buf_len += 64 - len;
			nid->cbufs[curindex-1].s_len += 64 - len;
		}
		nid->opcode = SNDDG;
		nid->status = 0;
		nid->dg_ptdb_index = 1;
		nid->dg_len = 10 + curindex*8;
		nid->cbufs[--curindex].chain = 0;
		ds->ds_if.if_opackets++;
		if((empty=insqti(nid, &ni->comq0, NI_MAXITRY)) > 0)
			printf("insqti failed\n");
		else if(empty == QEMPTY) {
			while((addr->pc&PC_OWN))
				;
			addr->pc =  PC_CMDQNE|PC_CMDQ0|PC_OWN;
		} 
	}
	return;
drop: 
	IF_ENQUEUE(&ds->ds_if.if_snd, m);
}

/*
 * Command done interrupt.
 */
bvpniintr(unit)
	register int unit;
{
	register struct ni *ni = &niinfo[unit];
	register struct nidevice *addr = (struct nidevice *)ni->ni_regs;

retry:
	if((addr->ps&PS_STATEMASK) != PS_ENABLED) {
		if((addr->ps&PS_STATEMASK) == PS_UNDEFINED ||
		   (addr->ps&PS_STATEMASK) == PS_STOPPED)
			nireset(unit);
		addr->ps &= ~PS_SUME;
		goto done;
	}

	/*
	 * Check for incoming packets.
	 */
		if(addr->ps&PS_RSQ) {
			nirecv(unit);
		}
	/*
	 * Check for outgoing packets.
	 */
		{
		int s = splimp();
			nistart(unit);
		splx(s);
		}
done:
	if(addr->ps & PS_SUME) {
		register struct el_rec *elrp;

		if((elrp = ealloc(sizeof(struct el_bvp), EL_PRILOW))) {
		register struct el_bvp *elbod;
		struct biic_regs *nxv;
		elbod = &elrp->el_body.elbvp;
		nxv = (struct biic_regs *)
			((char *)(ni->ni_regs) - NI_NI_ADDR);
		elbod->bvp_biic_typ = nxv->biic_typ;
		elbod->bvp_biic_csr = nxv->biic_ctrl;
		elbod->bvp_pcntl = addr->pc;
		elbod->bvp_pstatus = addr->ps;
		elbod->bvp_perr = addr->pe;
		elbod->bvp_pdata = addr->pd;
		LSUBID(elrp,ELCT_DCNTL,ELBI_BVP,ELBVP_AIE,
			ni->ni_pqb->ni.piv.bi_node,unit,addr->pe);
		EVALID(elrp);
		}
		if((addr->ps&PS_STATEMASK) == PS_UNDEFINED ||
		   (addr->ps&PS_STATEMASK) == PS_STOPPED)
		goto retry;
	}
		
	addr->ps &= ~(PS_OWN|PS_SUME|PS_RSQ);

}

/*
 * Ethernet interface receiver interface.
 */

nirecv(unit)
	int unit;
{
	register struct ni *ni = &niinfo[unit];
	register struct ni_data *nid;
	register struct nidevice *addr = (struct nidevice *)ni->ni_regs;
	register struct ni_softc *ds = &ni_softc[unit];
	int len, empty;
	struct _bd *bde;

	/* First guess is that its a data gram recieve */
	
	for(;;) {
	if((nid = (struct ni_data *)remqhi(&ni->respq, NI_MAXITRY)) 
		>= (struct ni_data *)QEMPTY) 
		break;
		if(nid->status&PCK_FAIL) {
			register struct el_rec *elrp;

			if((elrp = ealloc(sizeof(struct el_bvp), EL_PRILOW))) {
			register struct el_bvp *elbod;
			struct biic_regs *nxv;
			elbod = &elrp->el_body.elbvp;
			nxv = (struct biic_regs *)
				((char *)(ni->ni_regs) - NI_NI_ADDR);
			elbod->bvp_biic_typ = nxv->biic_typ;
			elbod->bvp_biic_csr = nxv->biic_ctrl;
			elbod->bvp_pcntl = addr->pc;
			elbod->bvp_pstatus = addr->ps;
			elbod->bvp_perr = addr->pe;
			elbod->bvp_pdata = addr->pd;
			LSUBID(elrp,ELCT_DCNTL,ELBI_BVP,ELBVP_AIE,
				ni->ni_pqb->ni.piv.bi_node,unit,nid->status);
			EVALID(elrp);
			}
		}
		switch(nid->opcode) {
		case DGIREC:
		case DGISNT:
			break;
		case DGREC:
			ds->ds_if.if_ipackets++;
			bde = (struct _bd *) ni->ni_pqb->ni.bdt_base;
			bde += nid->cbufs[0].bdt_index;
			if(nid->status&PCK_FAIL)
				ds->ds_if.if_ierrors++;
			len = 0;
			/* Walk buffers & add length */
			{ register int curindex = 0;
			  register struct _bd *pbde = bde;
			while(curindex < NI_RBUF ) {
				len += nid->cbufs[curindex].s_len;
				nid->cbufs[curindex].s_len = pbde++->buf_len;
				if(nid->cbufs[curindex].chain == 0)
					break;
				curindex++;
			}
			}
			niread(ni, ds, bde, len, 0);
			nid->opcode=DGREC;
			nid->dg_len = NI_DGRLEN;
			nid->status=0;
	/* DGREC must end up on freeq2 */
			if((empty=insqti(nid, &ni->freeq2, NI_MAXITRY)) > 0)
				printf("insqti failed\n");
			else if(empty == QEMPTY) {
				while((addr->pc&PC_OWN))
					;
				addr->pc =  PC_FREEQNE|PC_RFREEQ|PC_OWN;
			}
			break;

		case DGSNT:
	/* DGSNT must end up on freeq1 */
			if(nid->status&PCK_FAIL)
				ds->ds_if.if_oerrors++;
			else {
			struct mbuf *n, *m = (struct mbuf *)nid->mbuf_tofree;

			for(n = m; n; n = n->m_next)
				if(n->m_cltype == 2) { /* Stupid nfs */
					m_freem(m);
					goto cnt;
				} 
			/* else A loopback */
				niread(ni, ds, 0, 64, m);
			}
cnt:
			nid->mbuf_tofree = 0;
			if((empty=insqti(nid, &ni->freeq1, NI_MAXITRY)) > 0)
				printf("insqti failed\n");
			else if(empty == QEMPTY) {
				while((addr->pc&PC_OWN))
					;
				addr->pc =  PC_FREEQNE|PC_DFREEQ|PC_OWN;
			}
			break;
		case MSGSNT:
		case MSGREC:
			{ 
			struct ni_msg *ni_msg;
			ni_msg = (struct ni_msg *)nid;
			switch(ni_msg->ni_opcode) {
				case NIOP_WPARAM:
				case NIOP_RPARAM:
				bcopy(((struct ni_param *)&ni_msg->text[0])->apa, ds->ds_addr, 6);
				bcopy(((struct ni_param *)&ni_msg->text[0])->dpa, ds->ds_dpaddr, 6);
					break;
				case NIOP_RCCNTR:
				case NIOP_RDCNTR:
				/* User may be waiting for info to come back */
					wakeup((caddr_t)ni_msg);
					break;
				case NIOP_STPTDB:
				case NIOP_CLPTDB:
				default:
					break;
			}
	/* MSGSNT must end up on freeq0 */
			if((empty=insqti(ni_msg, &ni->freeq0, NI_MAXITRY))>0)
				printf("insqti failed\n");
			else if(empty == QEMPTY) {
				while((addr->pc&PC_OWN))
					;
				addr->pc = PC_FREEQNE|PC_MFREEQ|PC_OWN;
			}
			}
			break;
		default:
			cprintf("ni%d unknown respq opcode\n", unit);
			if((empty=insqti(nid, &ni->freeq0, NI_MAXITRY))>0)
				printf("insqti failed\n");
			else if(empty == QEMPTY) {
				while((addr->pc&PC_OWN))
					;
				addr->pc = PC_FREEQNE|PC_MFREEQ|PC_OWN;
			}
			break;
		}
	}	
}
/*
 * Pass a packet to the higher levels.
 * We deal with the trailer protocol here.
 */
niread(ni, ds, bde, len, swloop)
	struct ni *ni;
	register struct ni_softc *ds;
	register struct _bd *bde;
	int len;
	struct mbuf *swloop; 
{
    	register struct mbuf *m, *swloop_tmp1;
	struct ether_header *eh, swloop_eh;
	struct protosw *pr;
	int off, resid;
	struct ifqueue *inq;
	struct _bd *pbde = 0;
	int toff = 0;

	/*
	 * Deal with trailer protocol: if type is trailer
	 * get true type from first 16-bit word past data.
	 * Remember that type was trailer by setting off.
	 */
	if (swloop) {
		eh = mtod(swloop, struct ether_header *);
		swloop_eh = *eh;
		eh = &swloop_eh;
		if ( swloop->m_len > sizeof(struct ether_header))
			m_adj(swloop, sizeof(struct ether_header));
		else {
			MFREE(swloop, swloop_tmp1);
			if ( ! swloop_tmp1 )
				return;
			else
				swloop = swloop_tmp1;
		}
	} else  
	    eh = (struct ether_header *)(vaddr(bde));
	eh->ether_type = ntohs((u_short)eh->ether_type);
#define	dataaddr(eh, off, type)	((type)(((caddr_t)(eh)+(off))))
	if (eh->ether_type >= ETHERTYPE_TRAIL &&
	    eh->ether_type < ETHERTYPE_TRAIL+ETHERTYPE_NTRAILER) {
		off = (eh->ether_type - ETHERTYPE_TRAIL) * 512;
		if (off >= ETHERMTU)
			return;		/* sanity */
		if (swloop) {
			struct mbuf *mprev, *m0 = swloop;
/* need to check this against off */
			mprev = m0;
			while (swloop->m_next){/*real header at end of chain*/
				mprev = swloop;
				swloop = swloop->m_next;
			}
			/* move to beginning of chain */
			mprev->m_next = 0;
			swloop->m_next = m0;
			eh->ether_type = ntohs( *mtod(swloop, u_short *));

		} else {
			struct ether_header *peh;
			toff = (off < NI_MAXPACKETSZ ? off 
				: off - NI_MAXPACKETSZ);
			pbde = (off < NI_MAXPACKETSZ ? (bde + 1) : (bde + 2));
			peh = (struct ether_header *)vaddr(pbde);
			eh->ether_type = ntohs(*dataaddr(peh, toff, u_short *));
			resid = ntohs(*(dataaddr(peh, toff+2, u_short *)));
			if (off + resid > len)
			return;		/* sanity */
			len = off + resid + sizeof(struct ether_header);
		}
	} else
		off = 0;
	if (len == 0)
		return;

	/*
	 * Pull packet off interface.  Off is nonzero if packet
	 * has trailing header; niget will then force this header
	 * information to be at the front, but we still have to drop
	 * the type and length which are at the front of any trailer data.
	 */
	if (swloop) {
		m = m_copy(swloop, 0, M_COPYALL);
		m_freem(swloop);
	} else {
	/* Header was in the 1st buffer */
		bde++;
		len -= sizeof(struct ether_header);
		m = niget(ni, bde, len, off, pbde, toff);
	}
	if (m == 0)
		return;
	if (off) {
		m->m_off += 2 * sizeof (u_short);
		m->m_len -= 2 * sizeof (u_short);
	}
	switch (eh->ether_type) {

#ifdef INET
	case ETHERTYPE_IP:
		if (nINET==0) {
			m_freem(m);
			return;
		}
		schednetisr(NETISR_IP);
		inq = &ipintrq;
		break;

	case ETHERTYPE_ARP:
		if (nETHER==0) {
			m_freem(m);
			return;
		}
		arpinput(&ds->ds_ac, m);
		return;
#endif
	default:

		/*
		 * see if other protocol families defined
		 * and call protocol specific routines.
		 * If no other protocols defined then dump message.
		 */
		if ((pr=iftype_to_proto(eh->ether_type)) && pr->pr_ifinput)  {
			if ((m = (struct mbuf *)(*pr->pr_ifinput)(m, &ds->ds_if, &inq, eh)) == 0)
				return;
		} else {
			ds->ds_if.if_ierrors++;
			m_freem(m);
			return;
		}
	}

	if (IF_QFULL(inq)) {
		IF_DROP(inq);
		m_freem(m);
		return;
	}
	IF_ENQUEUE(inq, m);
}

/*
 * Ethernet output routine.
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 */
nioutput(ifp, m0, dst)
	struct ifnet *ifp;
	struct mbuf *m0;
	struct sockaddr *dst;
{
	int type, s, error;
	u_char edst[6]; 
	struct in_addr idst;
	struct protosw *pr;
	register struct ni_softc *ds = &ni_softc[ifp->if_unit];
	register struct mbuf *m = m0;
	register struct ether_header *eh;
	register int off;

	switch (dst->sa_family) {

#ifdef INET
	case AF_INET:
		if (nINET == 0) {
			printf("ni%d: can't handle af%d\n", ifp->if_unit,
				dst->sa_family);
			error = EAFNOSUPPORT;
			goto bad;
		}
		idst = ((struct sockaddr_in *)dst)->sin_addr;
		if (!arpresolve(&ds->ds_ac, m, &idst, edst))
			return (0);	/* if not yet resolved */
		off = ntohs((u_short)mtod(m, struct ip *)->ip_len) - m->m_len;
		/* need per host negotiation */
		if ((ifp->if_flags & IFF_NOTRAILERS) == 0)
		if (off > 0 && (off & 0x1ff) == 0 &&
		    m->m_off >= MMINOFF + 2 * sizeof (u_short)) {
			type = ETHERTYPE_TRAIL + (off>>9);
			m->m_off -= 2 * sizeof (u_short);
			m->m_len += 2 * sizeof (u_short);
			*mtod(m, u_short *) = htons((u_short)ETHERTYPE_IP);
			*(mtod(m, u_short *) + 1) = htons((u_short)m->m_len);
			goto gottrailertype;
		}
		type = ETHERTYPE_IP;
		off = 0;
		goto gottype;
#endif

	case AF_UNSPEC:
		eh = (struct ether_header *)dst->sa_data;
 		bcopy((caddr_t)eh->ether_dhost, (caddr_t)edst, sizeof (edst));
		type = eh->ether_type;
		goto gottype;

	default:
		/*
		 * try to find other address families and call protocol
		 * specific output routine.
		 */
		
		if (pr=iffamily_to_proto(dst->sa_family)) {
			(*pr->pr_ifoutput)(ifp, m0, dst, &type, (char *)edst);
			goto gottype;
		}
		else {
			printf("ni%d: can't handle af%d\n", ifp->if_unit,
				dst->sa_family);
			error = EAFNOSUPPORT;
			goto bad;
		}
	}

gottrailertype:
	/*
	 * Packet to be sent as trailer: move first packet
	 * (control information) to end of chain.
	 */
	while (m->m_next)
		m = m->m_next;
	m->m_next = m0;
	m = m0->m_next;
	m0->m_next = 0;
	m0 = m;

gottype:
	/*
	 * Add local net header.  If no space in first mbuf,
	 * allocate another.
	 */
	if (m->m_off > MMAXOFF ||
	    MMINOFF + sizeof (struct ether_header) > m->m_off) {
		m = m_get(M_DONTWAIT, MT_HEADER);
		if (m == 0) {
			error = ENOBUFS;
			goto bad;
		}
		m->m_next = m0;
		m->m_off = MMINOFF;
		m->m_len = sizeof (struct ether_header);
	} else {
		m->m_off -= sizeof (struct ether_header);
		m->m_len += sizeof (struct ether_header);
	}
	eh = mtod(m, struct ether_header *);
	eh->ether_type = htons((u_short)type);
 	bcopy((caddr_t)edst, (caddr_t)eh->ether_dhost, sizeof (edst));

	/*
	 * Queue message on interface, and start output if interface
	 * not yet active.
	 */
	s = splimp();
	if (IF_QFULL(&ifp->if_snd)) {
		IF_DROP(&ifp->if_snd);
		splx(s);
		m_freem(m);
		return (ENOBUFS);
	}
	IF_ENQUEUE(&ifp->if_snd, m);
	nistart(ifp->if_unit);
	splx(s);
	return (0);

bad:
	m_freem(m0);
	return (error);
}

/*
 * Process an ioctl request.
 */
niioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	register struct ni_softc *ds = &ni_softc[ifp->if_unit];
	register struct ni *ni = &niinfo[ifp->if_unit];
	register struct nidevice *addr = (struct nidevice *)ni->ni_regs;
	struct protosw *pr;
	struct ifreq *ifr = (struct ifreq *)data;
	struct ifdevea *ifd = (struct ifdevea *)data;
	register struct ifaddr *ifa = (struct ifaddr *)data;
	int s = splnet(), error = 0, empty;

	switch (cmd) {

        case SIOCENABLBACK:
                ifp->if_flags |= IFF_LOOPBACK;
                break;
 
        case SIOCDISABLBACK:
                ifp->if_flags &= ~IFF_LOOPBACK;
                niinit(ifp->if_unit);
                break;
 
        case SIOCRPHYSADDR: 
                /*
                 * read default hardware address.
                 */
		bcopy(ds->ds_dpaddr, ifd->default_pa, 6);
		bcopy(ds->ds_addr, ifd->current_pa, 6);
                break;
 

	case SIOCSPHYSADDR: 
		/* 
		 * Set physaddr.
		 */
                niinit(ifp->if_unit);
	{
		struct ni_msg *ni_msg;
		if((ni_msg = (struct ni_msg *)remqhi(&ni->freeq0, NI_MAXITRY))
!= (struct ni_msg *)QEMPTY) {
		ni_msg->opcode = SNDMSG;
		ni_msg->status = 0;
		ni_msg->ni_opcode = NIOP_WPARAM;
		ni_msg->msg_len = sizeof (struct ni_param) + 6;
		((struct ni_param *)&ni_msg->text[0])->flags = NI_PAD;
		bcopy(ifr->ifr_addr.sa_data, 
			((struct ni_param *)&ni_msg->text[0])->apa, 6);
		bcopy(ifr->ifr_addr.sa_data, ds->ds_addr, 6);
		if((empty=insqti(ni_msg, &ni->comq0, NI_MAXITRY))>0)
			printf("insqti failed\n");
		else if(empty == QEMPTY) {
			while((addr->pc&PC_OWN))
				;
			addr->pc = PC_CMDQNE|PC_CMDQ0|PC_OWN;
		}
		}
	}

		break;

	case SIOCDELMULTI: 
	case SIOCADDMULTI: 
	{
		int i,j = -1;

		if (cmd==SIOCDELMULTI) {
		   for (i = 0; i < NMULTI; i++)
		       if (bcmp(ds->ds_multi[i], ifr->ifr_addr.sa_data,MULTISIZE) == 0) {
			    	if (--ds->ds_muse[i] == 0)
					bcopy(ni_multi,ds->ds_multi[i],MULTISIZE);
		       }
		} else {
		    for (i = 0; i < NMULTI; i++) {
			if (bcmp(ds->ds_multi[i],ifr->ifr_addr.sa_data,MULTISIZE) == 0) {
			    ds->ds_muse[i]++;
			    goto done;
			}
			if (bcmp(ds->ds_multi[i],ni_multi,MULTISIZE) == 0)
			    j = i;
		    }
		    if (j == -1) {
			printf("ni%d: multi failed, multicast list full: %d\n",
				ni->unit, NMULTI);
			error = ENOBUFS;
			goto done;
		    }
		    bcopy(ifr->ifr_addr.sa_data, ds->ds_multi[j], MULTISIZE);
		    ds->ds_muse[j]++;
		}
	/* Update up Protocol Type Definition Block */
		if(ifp->if_flags&IFF_UP)
			ni_sptdb(ni,ETHERTYPE_IP,2,2,PTDB_UNK|PTDB_BDC,1);
		break;
	}

	case SIOCRDCTRS:
	case SIOCRDZCTRS:
		{
		register struct ctrreq *ctr = (struct ctrreq *)data;

	/* Read/Read-Clear counters */
		struct ni_msg *ni_msg;
		struct ni_counters *ni_counters;
		int k;
		ni_msg = (struct ni_msg *)remqhi(&ni->freeq0, NI_MAXITRY);
		if(ni_msg == (struct ni_msg *)QEMPTY) {
			error = ENOBUFS;
			goto done;
		}
		ni_msg->opcode = SNDMSG;
		if(cmd == SIOCRDCTRS)
			ni_msg->ni_opcode = NIOP_RDCNTR;
		else
			ni_msg->ni_opcode = NIOP_RCCNTR;
		ni_msg->msg_len = sizeof(struct ni_counters) + 6;
		while((addr->pc&PC_OWN))
			;
		k=splimp();
		if((empty=insqti(ni_msg, &ni->comq0, NI_MAXITRY))>0)
			printf("insqti failed\n");
		else if(empty == QEMPTY)
			addr->pc = PC_CMDQNE|PC_CMDQ0|PC_OWN;
		/* Wait for read counters to finish */
		sleep((caddr_t)ni_msg, PUSER);
		splx(k);

		ni_counters = (struct ni_counters *)&ni_msg->text[0];
		bzero(&ctr->ctr_ctrs, sizeof(struct estat));
		ctr->ctr_type = CTR_ETHER;
		ctr->ctr_ether.est_seconds = ni_counters->last_zero;
		ctr->ctr_ether.est_bytercvd = ni_counters->bytes_rec;
		ctr->ctr_ether.est_bytesent = ni_counters->bytes_snt;
		ctr->ctr_ether.est_mbytercvd = ni_counters->mbytes_rec;
		ctr->ctr_ether.est_blokrcvd = ni_counters->frame_rec;
		ctr->ctr_ether.est_bloksent = ni_counters->frame_snt;
		ctr->ctr_ether.est_mblokrcvd = ni_counters->mframe_rec;
		ctr->ctr_ether.est_deferred = ni_counters->fs_def;
		ctr->ctr_ether.est_single = ni_counters->fs_sc;
		ctr->ctr_ether.est_multiple = ni_counters->fs_mc;
		ctr->ctr_ether.est_sendfail = ni_counters->sfail;
		ctr->ctr_ether.est_sendfail_bm = ni_counters->sfbm;
		ctr->ctr_ether.est_collis = ni_counters->datao;
		ctr->ctr_ether.est_recvfail = ni_counters->rfail;
		ctr->ctr_ether.est_recvfail_bm = ni_counters->rfbm;
		ctr->ctr_ether.est_unrecog = ni_counters->unrec;
		ctr->ctr_ether.est_sysbuf = ni_counters->sbu;
		ctr->ctr_ether.est_userbuf = ni_counters->ubu;
		}
		break;

	case SIOCSIFADDR:
		{
		ifp->if_flags |= IFF_UP;
		niinit(ifp->if_unit);
		switch(ifa->ifa_addr.sa_family) {
#ifdef INET
		case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr =
				IA_SIN(ifa)->sin_addr;
			/* 1st packet out */
			arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
			break;
#endif

		default:
			if (pr=iffamily_to_proto(ifa->ifa_addr.sa_family)) {
				error = (*pr->pr_ifioctl)(ifp, cmd, data);
			}
			break;
		}
		}
		break;
	default:
		error = EINVAL;
	}
done:	splx(s);
	return (error);
}

/*
 * Pull read data off a interface.
 * Len is length of data, with local net header stripped.
 * Off is non-zero if a trailer protocol was used, and
 * gives the offset of the trailer information.
 * We copy the trailer information and then all the normal
 * data into mbufs.
 */
struct mbuf *
niget(ni, bde, totlen, off0, pbde, trailoff)
	struct ni *ni;
	struct _bd *bde, *pbde;
	int totlen, off0, trailoff;
{
	struct mbuf *top, **mp, *m;
	int off = off0, len = 0;
	/* setup for 1st buffer */
	int cp = (int) vaddr(bde); 
	int bdelen = bde->buf_len;

	top = 0;
	mp = &top;
	while (totlen > 0) {
		MGET(m, M_DONTWAIT, MT_DATA);
		if (m == 0)
			goto bad;
		if (off) {
			cp = (int) vaddr(pbde) + trailoff;
			bdelen = pbde->buf_len;
			len = totlen - off;
		} else
			len = totlen;

		if (bdelen >= CLBYTES && len >= CLBYTES) {
			struct mbuf *p;
			struct pte *cpte, *ppte;
			int i;

			MCLGET(m, p);
			if (p == 0)
				goto nopage;
			len = m->m_len = CLBYTES;
			m->m_off = (int)p - (int)m;
			if (!claligned(cp))
				goto copy;

		/* Swap cluster 1 for 1 */
			cpte = &Mbmap[mtocl(cp)*CLSIZE];
			ppte = &Mbmap[mtocl(p)*CLSIZE];
			for(i=0; i<CLSIZE; i++) {
				struct pte t;
				t = *ppte; *ppte++ = *cpte; *cpte++ = t;
				mtpr(TBIS, (caddr_t)cp);
				cp += NBPG;
				mtpr(TBIS, (caddr_t)p);
				p += NBPG / sizeof (*p);
			} 
			goto nocopy;
		}
nopage:
		m->m_len = MIN(MLEN, len);
		m->m_len = MIN(m->m_len, bdelen);
		m->m_off = MMINOFF;
copy:
		bcopy((caddr_t)cp, mtod(m, caddr_t), (unsigned)m->m_len);
		cp += m->m_len;

nocopy:
		bdelen -= m->m_len;

		if(bdelen <= 0 && off == 0) { /* next buffer */
				bde++;
				bdelen=bde->buf_len;
				cp = (int)vaddr(bde);
		}

		*mp = m;
		mp = &m->m_next;
		if (off) {
			/* sort of an ALGOL-W style for statement... */
			off += m->m_len;
			if (off == totlen) {
				cp = (int) vaddr(bde);
				bdelen = bde->buf_len;
				off = 0;
				totlen = off0;
			}
		} else
			totlen -= m->m_len;
	}
	return (top);
bad:
	m_freem(top);
	return (0);
}
#endif

ni_sptdb(ni, type, index, q, flags, doclear)
	struct ni *ni;
	/* Set Protocol Type Definition Block */
{
		register struct ni_softc *ds = &ni_softc[ni->unit];
		struct ni_msg *ni_msg;
		struct ptdb *ptdb;
		int empty,i;
		struct nidevice *addr = (struct nidevice *)ni->ni_regs;
		int clear = doclear;


loop:
		ni_msg = (struct ni_msg *)remqhi(&ni->freeq0, NI_MAXITRY);
		if(ni_msg == (struct ni_msg *)QEMPTY)
			return;
		bzero(&ni_msg->text[0], sizeof(struct ptdb));
		ni_msg->status = 0;
		if(clear)
			ni_msg->ni_opcode = NIOP_CLPTDB;
		else
			ni_msg->ni_opcode = NIOP_STPTDB;
		ni_msg->opcode = SNDMSG;
		ptdb = (struct ptdb *)&ni_msg->text[0];
		ptdb->flags = flags;
		ptdb->fq_index = q;
		ptdb->ptt = type;
		ptdb->ptdb_index = index;
		ptdb->adr_len = 0;
		ni_msg->msg_len = 18;
		if(!clear && flags&(PTDB_AMC|PTDB_BDC)) {
		   int nptdb=0;

		/* 1st reserved for -1 broadcast addr */			
		   ptdb->adr_len++;
		   ni_msg->msg_len += 8;
		   bcopy(ni_multi, (&ptdb->multi[nptdb++])->addr,6);

		   for (i = 0; i < NMULTI - 1; i++)
		        if (ds->ds_muse[i] > 0) {
			ptdb->adr_len++;
			ni_msg->msg_len += 8;
			bcopy(ds->ds_multi[i],(&ptdb->multi[nptdb++])->addr,6);
		       }
		}

		if((empty=insqti(ni_msg, &ni->comq0, NI_MAXITRY)) > 0)
			printf("insqti failed\n");
		else if(empty == QEMPTY) {
			while((addr->pc&PC_OWN))
				;
			addr->pc = PC_CMDQNE|PC_CMDQ0|PC_OWN;
			while((addr->pc&PC_OWN))
				;
		}
	while((addr->ps&PS_OWN))
		;
		if(clear) {
			clear = 0;
			goto loop;
		}
}	
struct mbuf *
m_compress(n) /* compress silly LONG mbuf chains */
	register struct mbuf *n;
{
	register struct mbuf *m, *q, *p;
	int len, count;

	MGET(m, M_DONTWAIT, n->m_type);
	p = m;
	m->m_len = 0;
	for(;;) {
		len = MLEN;
	do {
		count = MIN(n->m_len, len);
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t)+m->m_len,
		  (unsigned)count);
		len -= count;
		m->m_len += count;
		n->m_len -= count;
		if (n->m_len)
			n->m_off += count;
		else
			n = m_free(n);
		if(!n)
			return(p);
	} while (len && n);
	MGET(q, M_DONTWAIT, n->m_type);
	q->m_len = 0;
	m->m_next = q;
	m = q;
	}
}
