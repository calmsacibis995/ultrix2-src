
#ifndef lint
static char *sccsid = "@(#)uba.c	1.26	ULTRIX	10/3/86";
#endif lint

/************************************************************************
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
 ************************************************************************
 * Modification History:
 *
 * 29-Sep-86  -- darrell
 *	Space for vsbuf is now allocated here instead of in 
 *	../data/sdc_data.c.
 *
 * 26-Sep-86  -- darrell
 *	Fixed a bug in vs_bufctl that caused the vaxstar tape/disk
 *	buffer to be allocated incorrectly.
 *
 * 05-Sep-86  -- darrell
 *	Added a panic. vs_bufctl will now panic if called by the
 *	owner of the buffer.
 *
 * 30-Aug-86  -- darrell (Darrell Dunnuck)
 *	Fix bugs in VAXstar data buffer interlock code, which
 *	allows TZK50 and disk to share a common DMA data buffer.
 *
 *  5-Aug-86   -- gmm (George Mathew) and darrell (Darrell Dunnuck)
 *	Added routines to allow sharing the common disk data buffer
 *	between the VAXstar disk and TZK50 drivers.
 *
 * 13-Jun-86   -- jaw 	fix to uba reset and drivers.
 *
 * 14-May-86 -- bjg
 *	Move uba# field in subid when logging uba errors
 *
 * 16-Apr-86 -- afd
 *	Changed UMEMmap to QMEMmap and umem to qmem for QBUS reset.
 *
 * 19-feb-86 -- bjg  add uba error logging
 *
 * 04-feb-86 -- jaw  get rid of biic.h.
 *
 * 15-jul-85 -- jaw
 *	VAX8800 support
 *
 * 11 Nov 85   depp
 *	Removed System V conditional compiles.
 *
 * 08-Aug-85	darrell
 *	Zero vectors are now timed.  If we get too many, a message is 
 *	printed into the message buffer reporting the rate at which they
 *	are accuring.  The routine "ubatimer" was added.
 *
 * 11-jul-85 -- jaw
 *	fix bua/bda map registers.
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 06 Jun 85 -- jaw
 *	add in the BDA support.
 *
 *  7 May 85 -- rjl
 *	Turned on Q-bus map as part of bus init. 
 * 
 * 22 Mar 85 -- depp
 *	Added Sys V Shared memory support
 *
 * 13-MAR-85 -jaw
 *	Changes for support of the VAX8200 were merged in.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 * 12 Nov 84 -- rjl
 *	Added support for MicroVAX-II notion of a q-bus adapter.
 */

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../h/vm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/conf.h"
#include "../h/dk.h"
#include "../h/kernel.h"
#include "../h/clist.h"

#include "../h/ipc.h"
#include "../h/shm.h"
#include "../h/errlog.h"

#include "../vax/cpu.h"
#include "../vax/mtpr.h"
#include "../vax/nexus.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"

#include "../vaxbi/buareg.h"


/*
 *  For zero vector timer -- should really be in a data.c file, but
 *  but there isn't one for uba.c
 */
int	ubatimer();
int	zvintvl = ZVINTVL;	/* zero vector timer interval in seconds */
int	zvthresh = ZVTHRESH;	/* zero vector timer threhsold for reporting */

char	ubasr_bits[] = UBASR_BITS;

/*
 * Do transfer on device argument.  The controller
 * and uba involved are implied by the device.
 * We queue for resource wait in the uba code if necessary.
 * We return 1 if the transfer was started, 0 if it was not.
 * If you call this routine with the head of the queue for a
 * UBA, it will automatically remove the device from the UBA
 * queue before it returns.  If some other device is given
 * as argument, it will be added to the request queue if the
 * request cannot be started immediately.  This means that
 * passing a device which is on the queue but not at the head
 * of the request queue is likely to be a disaster.
 */
ubago(ui)
	register struct uba_device *ui;
{
	register struct uba_ctlr *um = ui->ui_mi;
	register struct uba_hd *uh;
	register int s, unit;

	uh = &uba_hd[um->um_ubanum];
	s = spl6();
	if (um->um_driver->ud_xclu && uh->uh_users > 0 || uh->uh_xclu)
		goto rwait;
	um->um_ubinfo = ubasetup(um->um_ubanum, um->um_tab.b_actf->b_actf,
	    UBA_NEEDBDP|UBA_CANTWAIT);
	if (um->um_ubinfo == 0)
		goto rwait;
	uh->uh_users++;
	if (um->um_driver->ud_xclu)
		uh->uh_xclu = 1;
	splx(s);
	if (ui->ui_dk >= 0) {
		unit = ui->ui_dk;
		dk_busy |= 1<<unit;
		dk_xfer[unit]++;
		dk_wds[unit] += um->um_tab.b_actf->b_actf->b_bcount>>6;
	}
	if (uh->uh_actf == ui)
		uh->uh_actf = ui->ui_forw;
	(*um->um_driver->ud_dgo)(um);
	return (1);
rwait:
	if (uh->uh_actf != ui) {
		ui->ui_forw = NULL;
		if (uh->uh_actf == NULL)
			uh->uh_actf = ui;
		else
			uh->uh_actl->ui_forw = ui;
		uh->uh_actl = ui;
	}
	splx(s);
	return (0);
}

ubadone(um)
	register struct uba_ctlr *um;
{
	register struct uba_hd *uh = &uba_hd[um->um_ubanum];

	if (um->um_driver->ud_xclu)
		uh->uh_xclu = 0;
	uh->uh_users--;
	/* already released map registers if it's a MicroVAX I */
	if( (uh->uba_type & UBAUVI) ==0)
		ubarelse(um->um_ubanum, &um->um_ubinfo);
}

/*
 * Allocate and setup UBA map registers, and bdp's
 * Flags says whether bdp is needed, whether the caller can't
 * wait (e.g. if the caller is at interrupt level).
 *
 * Return value:
 *	Bits 0-8	Byte offset
 *	Bits 9-17	Start map reg. no.
 *	Bits 18-27	No. mapping reg's
 *	Bits 28-31	BDP no.
 */
ubasetup(uban, bp, flags)
	struct buf *bp;
{
	register struct uba_hd *uh = &uba_hd[uban];
	register int temp;
	int npf, reg, bdp;
	unsigned v;
	register struct pte *pte, *io;
	struct proc *rp;
	int a, o, ubinfo;

	if (uh->uba_type & (UBA730|UBAUVI|UBAUVII))
		flags &= ~UBA_NEEDBDP;
	v = btop(bp->b_un.b_addr);
	o = (int)bp->b_un.b_addr & PGOFSET;
	npf = btoc(bp->b_bcount + o) + 1;
	a = spl6();
	while ((reg = rmalloc(uh->uh_map, (long)npf)) == 0) {
		if (flags & UBA_CANTWAIT) {
			splx(a);
			return (0);
		}
		uh->uh_mrwant++;
		sleep((caddr_t)&uh->uh_mrwant, PSWP);
	}
	bdp = 0;
	if (flags & UBA_NEEDBDP) {
		while ((bdp = ffs(uh->uh_bdpfree)) == 0) {
			if (flags & UBA_CANTWAIT) {
				rmfree(uh->uh_map, (long)npf, (long)reg);
				splx(a);
				return (0);
			}
			uh->uh_bdpwant++;
			sleep((caddr_t)&uh->uh_bdpwant, PSWP);
		}
		uh->uh_bdpfree &= ~(1 << (bdp-1));
	} else if (flags & UBA_HAVEBDP)
		bdp = (flags >> 28) & 0xf;
	splx(a);
	reg--;
	ubinfo = (bdp << 28) | (npf << 18) | (reg << 9) | o;
	temp = (bdp << 21) | UBAMR_MRV;
	if (bdp && (o & 01))
		temp |= UBAMR_BO;
	rp = bp->b_flags&B_DIRTY ? &proc[2] : bp->b_proc;
	if ((bp->b_flags & B_PHYS) == 0)
		pte = &Sysmap[btop(((int)bp->b_un.b_addr)&0x7fffffff)];
	else if (bp->b_flags & B_UAREA)
		pte = &rp->p_addr[v];
	else if (bp->b_flags & B_PAGET)
		pte = &Usrptmap[btokmx((struct pte *)bp->b_un.b_addr)];
	else if ((bp->b_flags & B_SMEM)  &&	/* SHMEM */
					((bp->b_flags & B_DIRTY) == 0))
		pte = ((struct smem *)rp)->sm_ptaddr + v;
	else
		pte = vtopte(rp, v);
	if ((uh->uba_type & UBAUVI) ==0 || (flags&UBA_MAPANYWAY)) {
		/* get address of starting UNIBUS map register */
		io = &uh->uh_uba->uba_map[reg];

		while (--npf != 0) {
			if (pte->pg_pfnum == 0)
				panic("uba zero uentry");
			*(int *)io++ = pte++->pg_pfnum | temp;
		}
		*(int *)io++ = 0;
	} else	{
		ubinfo = contigphys( ubinfo, bp->b_bcount, pte);
		++reg;
		a = spl6();
		rmfree( uh->uh_map, (long)npf, (long)reg);
		splx(a);
	}
	return (ubinfo);
}

/*
 * Non buffer setup interface... set up a buffer and call ubasetup.
 */
uballoc(uban, addr, bcnt, flags)
	int uban;
	caddr_t addr;
	int bcnt, flags;
{
	struct buf ubabuf;

	ubabuf.b_un.b_addr = addr;
	ubabuf.b_flags = B_BUSY;
	ubabuf.b_bcount = bcnt;
	/* that's all the fields ubasetup() needs */
	return (ubasetup(uban, &ubabuf, flags));
}
 
/*
 * Release resources on uba uban, and then unblock resource waiters.
 * The map register parameter is by value since we need to block
 * against uba resets on 11/780's.
 */
ubarelse(uban, amr)
	int *amr;
{
	register struct uba_hd *uh = &uba_hd[uban];
	register int bdp, reg, npf, s;
	int mr;
 
	/*
	 * Carefully see if we should release the space, since
	 * it may be released asynchronously at uba reset time.
	 */
	s = spl6();
	mr = *amr;
	if (mr == 0) {
		/*
		 * A ubareset() occurred before we got around
		 * to releasing the space... no need to bother.
		 */
		splx(s);
		return;
	}
	*amr = 0;
	splx(s);		/* let interrupts in, we're safe for a while */
	bdp = (mr >> 28) & 0x0f;

	if (bdp) {
		if (uh->uba_type&UBABUA){
			bdp = bdp & 0x07;
			((struct bua_regs *)uh->uh_uba)->bua_dpr[bdp] |= BUADPR_PURGE;
		}
		else if (uh->uba_type&UBA780)		
			uh->uh_uba->uba_dpr[bdp] |= UBADPR_BNE;

		else if (uh->uba_type&UBA750)
			uh->uh_uba->uba_dpr[bdp] |=
				    UBADPR_PURGE|UBADPR_NXM|UBADPR_UCE;

		uh->uh_bdpfree |= 1 << (bdp-1);		/* atomic */
		if (uh->uh_bdpwant) {
			uh->uh_bdpwant = 0;
			wakeup((caddr_t)&uh->uh_bdpwant);
		}
	}
	/*
	 * Put back the registers in the resource map.
	 * The map code must not be reentered, so we do this
	 * at high ipl.
	 */
	npf = (mr >> 18) & 0x3ff;
	reg = ((mr >> 9) & 0x1ff) + 1;
	s = spl6();
	rmfree(uh->uh_map, (long)npf, (long)reg);
	splx(s);

	/*
	 * Wakeup sleepers for map registers,
	 * and also, if there are processes blocked in dgo(),
	 * give them a chance at the UNIBUS.
	 */
	if (uh->uh_mrwant) {
		uh->uh_mrwant = 0;
		wakeup((caddr_t)&uh->uh_mrwant);
	}
	while (uh->uh_actf && ubago(uh->uh_actf))
		;
}

ubapurge(um)
	register struct uba_ctlr *um;
{
	register struct uba_hd *uh = um->um_hd;
	register int bdp = (um->um_ubinfo >> 28) & 0x0f;


	if (uh->uba_type & UBABUA) {
		bdp = bdp & 0x07;
		((struct bua_regs *)uh->uh_uba)->bua_dpr[bdp] |= BUADPR_PURGE;
	}
	else if (uh->uba_type & UBA780)
		uh->uh_uba->uba_dpr[bdp] |= UBADPR_BNE;
	else if (uh->uba_type & UBA750)
		uh->uh_uba->uba_dpr[bdp]|=UBADPR_PURGE|UBADPR_NXM|UBADPR_UCE;
}

ubainitmaps(uhp)
	register struct uba_hd *uhp;
{

	rminit(uhp->uh_map, (long)NUBMREG, (long)1, "uba", UAMSIZ);

	if (uhp->uba_type&UBABUA) 
		uhp->uh_bdpfree = (1<<NBDP_BUA) - 1;	
	else if(uhp->uba_type & UBA780)	
		uhp->uh_bdpfree = (1<<NBDP8600) - 1;
	else if(uhp->uba_type & UBA750)
		uhp->uh_bdpfree = (1<<NBDP750) - 1;
}

/*
 * Generate a reset on uba number uban.  Then
 * call each device in the character device table,
 * giving it a chance to clean up so as to be able to continue.
 */
ubareset(uban)
	int uban;
{
	register struct cdevsw *cdp;
	register struct uba_hd *uh = &uba_hd[uban];
	int s;

	s = spl6();
	uh->uh_users = 0;
	uh->uh_zvcnt = 0;
	uh->uh_xclu = 0;
	uh->uh_actf = uh->uh_actl = 0;
	uh->uh_bdpwant = 0;
	uh->uh_mrwant = 0;
	ubainitmaps(uh);
	wakeup((caddr_t)&uh->uh_bdpwant);
	wakeup((caddr_t)&uh->uh_mrwant);

	if (uh->uba_type & UBABDA) {
		printf("bda%d: reset",uban);
	}
	else {
		printf("uba%d: reset", uban);
		ubainit(uh->uh_uba,uh->uba_type);
	}

	/* reallocate global unibus space for tty drivers */
	if (tty_ubinfo[uban] != 0)
		tty_ubinfo[uban] = uballoc(uban, (caddr_t)cfree,
	    		nclist*sizeof (struct cblock), 0);

	for (cdp = cdevsw; cdp < cdevsw + nchrdev; cdp++)
		(*cdp->d_reset)(uban);
#ifdef INET
	ifubareset(uban);
#endif
	printf("\n");
	splx(s);
}

/*
 * Init a uba.  This is called with a pointer
 * rather than a virtual address since it is called
 * by code which runs with memory mapping disabled.
 * In these cases we really don't need the interrupts
 * enabled, but since we run with ipl high, we don't care
 * if they are, they will never happen anyways.
 */
ubainit(uba,ubatype)
	register int ubatype;
	register struct uba_regs *uba;

{

	if (ubatype&UBA780) {
		uba->uba_cr = UBACR_ADINIT;
		uba->uba_cr = UBACR_IFS|UBACR_BRIE|UBACR_USEFIE|UBACR_SUEFIE;
		while ((uba->uba_cnfgr & UBACNFGR_UBIC) == 0)
			;
	}
	else if (ubatype&UBABUA) 
		buainit(uba);
	else if (ubatype&(UBA750|UBA730|UBAUVI)) {
		mtpr(IUR, 0);
		/* give devices time to recover from power fail */
		DELAY(500000);
	}

	else if (ubatype&UBAUVII) {
#define LMEA 0x20			/* Local memory access enable	*/
#define QIPCR 0x1f40			/* Q-bus Inter-processor csr	*/
		/*
		 * Reset the bus and wait for the devices to
		 * settle down
		 */
		mtpr(IUR, 0);
		DELAY(500000);
		/*
		 * The bus reset turns off the q-bus map (unfortunately)
		 * The problem is further agravated by the fact that the
		 * enable bit is in the IPC register which is in I/O space
		 * instead of local register space.  Because of this we
		 * have to figure out if we're virtual or physical.
		 */
		if( mfpr(MAPEN) & 0x1 ) {
			/*
			 * Virtual
			 */
			*(u_short *)((char *)qmem+QMEMSIZEUVI+QIPCR) = LMEA;
		} else {
			/*
			 * Physical
			 */
			*(u_short *)((char *)QDEVADDRUVI+QIPCR) = LMEA;
		}
	}
}


int	ubawedgecnt = 10;
int	ubacrazy = 500;
/*
 * This routine is called by the locore code to
 * process a UBA error on an 11/780.  The arguments are passed
 * on the stack, and value-result (through some trickery).
 * In particular, the uvec argument is used for further
 * uba processing so the result aspect of it is very important.
 * It must not be declared register.
 */
/*ARGSUSED*/
ubaerror(uban, uh, xx, uvec, uba, ubapc)
	register int uban;
	register struct uba_hd *uh;
	int uvec;
	register struct uba_regs *uba;
	int *ubapc;
{
	register sr, s;
	struct el_rec *elrp;

	/*
	 *	Start a timer to time the rate of zero vectors.
	 *	The counting is done in locore.s.
	 */

	if (uvec == 0) {
		if (uh->uh_zvflg)
			mprintf("ubaerror: zero vector flag shouldn't be set\n");
		else {
			uh->uh_zvcnt++;
			uh->uh_zvflg++;
			timeout(ubatimer, uban, hz * zvintvl);
		}
		return;
	}
	if (uh->uba_type&UBABUA) {
		sr = ((struct bua_regs *)uh->uh_uba)->bua_ctrl;
		s = spl7();
		printf("bua%d: bua error ctrl=%x",uban,sr); 
		splx(s);
		((struct bua_regs *)uh->uh_uba)->bua_ctrl = sr;
		ubareset(uban);
	}
	else if ((uh->uba_type&UBABDA)==0) {	


		if (uba->uba_cnfgr & NEX_CFGFLT) {
			elrp = ealloc(EL_UBASIZE,EL_PRILOW);
			if (elrp != NULL) {
			    LSUBID(elrp,ELCT_ADPTR,ELADP_UBA,EL_UNDEF,EL_UNDEF,uban,EL_UNDEF);
		 	    elrp->el_body.eluba780.uba_cf = uba->uba_cnfgr;
		 	    elrp->el_body.eluba780.uba_cr = uba->uba_cr;
			    elrp->el_body.eluba780.uba_sr = uba->uba_sr;
		 	    elrp->el_body.eluba780.uba_dcr = uba->uba_dcr;
		 	    elrp->el_body.eluba780.uba_fmer = uba->uba_fmer;
			    elrp->el_body.eluba780.uba_fubar = uba->uba_fubar;
		 	    elrp->el_body.eluba780.uba_pc = *ubapc;
		 	    elrp->el_body.eluba780.uba_psl = *++ubapc;
			    EVALID(elrp);
			}	
			else {
				cprintf("uba%d: sbi fault sr=%b cnfgr=%b\n",
				    uban, uba->uba_sr, ubasr_bits,
				    uba->uba_cnfgr, NEXFLT_BITS);
			}
			ubareset(uban);
			uvec = 0;
			return;
		}
		sr = uba->uba_sr;
		s = spl7();
		elrp = ealloc(EL_UBASIZE,EL_PRILOW);
		if (elrp != NULL) {
		    LSUBID(elrp,ELCT_ADPTR,ELADP_UBA,EL_UNDEF,EL_UNDEF,uban,EL_UNDEF);
	 	    elrp->el_body.eluba780.uba_cf = uba->uba_cnfgr;
	 	    elrp->el_body.eluba780.uba_cr = uba->uba_cr;
		    elrp->el_body.eluba780.uba_sr = uba->uba_sr;
	 	    elrp->el_body.eluba780.uba_dcr = uba->uba_dcr;
	 	    elrp->el_body.eluba780.uba_fmer = uba->uba_fmer;
		    elrp->el_body.eluba780.uba_fubar = uba->uba_fubar;
	 	    elrp->el_body.eluba780.uba_pc = *ubapc;
	 	    elrp->el_body.eluba780.uba_psl = *++ubapc;
		    EVALID(elrp);
		}	
		else {
			cprintf("uba%d: uba error sr=%b fmer=%x fubar=%o\n",
			    uban, uba->uba_sr, ubasr_bits, uba->uba_fmer, 4*uba->uba_fubar);
		}
		splx(s);
		uba->uba_sr = sr;
		uvec &= UBABRRVR_DIV;
	}
	if (++uh->uh_errcnt % ubawedgecnt == 0) {
		if (uh->uh_errcnt > ubacrazy)
			panic("uba crazy");
		printf("ERROR LIMIT ");
		ubareset(uban);
		uvec = 0;
		return;
	}
	return;
}

/*
 * Allocate UNIBUS memory.  Allocates and initializes
 * sufficient mapping registers for access.  On a 780,
 * the configuration register is setup to disable UBA
 * response on DMA transfers to addresses controlled
 * by the disabled mapping registers.
 */
ubamem(uban, addr, npg, doalloc)
	int uban, addr, npg, doalloc;
{
	register struct uba_hd *uh = &uba_hd[uban];
	register int a;

	if (doalloc) {
		int s = spl6();
		a = rmget(uh->uh_map, npg, (addr >> 9) + 1);
		splx(s);
	} else
		a = (addr >> 9) + 1;
	if (a) {
		register int i, *m;

		m = (int *)&uh->uh_uba->uba_map[a - 1];

		for (i = 0; i < npg; i++)
			*m++ = 0;	/* All off, especially 'valid' */
		/*
		 * On a 780 and 8600, set up the map register disable
		 * field in the configuration register.  Beware
		 * of callers that request memory ``out of order''.
		 */
		if (uh->uba_type & UBA780) {
			int cr = uh->uh_uba->uba_cr;

			i = (addr + npg * 512 + 8191) / 8192;
			if (i > (cr >> 26))
				uh->uh_uba->uba_cr |= i << 26;
		}
	}
	return (a);
}

#include "ik.h"
#if NIK > 0
/*
 * Map a virtual address into users address space. Actually all we
 * do is turn on the user mode write protection bits for the particular
 * page of memory involved.
 */
maptouser(vaddress)
	caddr_t vaddress;
{

	Sysmap[(((unsigned)(vaddress))-0x80000000) >> 9].pg_prot = (PG_UW>>27);
}

unmaptouser(vaddress)
	caddr_t vaddress;
{

	Sysmap[(((unsigned)(vaddress))-0x80000000) >> 9].pg_prot = (PG_KW>>27);
}
#endif

/*
 *  Check the number of zero vectors and report if we get too many of them.
 *  Always reset the zero vector count and the zero vector timer flag.
 */

ubatimer(uban)
int	uban;
{
	struct uba_hd *uh;

	uh = &uba_hd[uban];
	if(uh->uh_zvcnt > zvthresh)
		mprintf("ubatimer: uba%d -- %d zero vectors in %d minutes\n",
			uban, uh->uh_zvcnt, zvintvl/60);
	uh->uh_zvcnt = 0;
	uh->uh_zvflg = 0;
}

/*
 * vs_bufctl is the locking mechanism that allows the VAXSTAR disk
 * tape controllers to share a common I/O buffer.
 *
 * This routine MUST be called from spl5.
 */
struct vsbuf vsbuf = { 0, 0, 0 };
extern int sdustart();
extern int st_start();

vs_bufctl(id, action, stillwant)
u_char id;
u_char action;
u_char stillwant;
{
	register struct vsbuf *vs = &vsbuf;
	register int rval;
	int temp;

	if (action == VS_ALLOC) {
	    if (vs->vs_active) {
		if (vs->vs_id != id) {
		    vs->vs_wants = id;
		    return;
		}
		else {
		    panic("vs_bufctl called by owner");
		    return;
		}
	    }
	    else {
		vs->vs_id = id;
		vs->vs_active++;
		if (vs->vs_id) {
		    if (vs->vs_id == VS_SDC) {
			rval = sdustart();
		    }
		    else {
			rval = st_start();
		    }
		}
		else {
		    printf ("vs_bufctl: error - no id set\n");
		    return;
		}
		
		for (;;) {
		    switch (rval) {
			case VS_DONE:
			    if (vs->vs_wants) {
				vs->vs_id = vs->vs_wants;
				vs->vs_wants = VS_IDLE;
			    }
			    else {
				vs->vs_active = vs->vs_wants = vs->vs_id = VS_IDLE;
				return;
			    }
			    break;

			case VS_WANT:
			    /*
			     * A temporary variable (temp) is needed here as
			     * we cannot trust the "id" variable to be the
			     * driver that actually has the buffer at the
			     * present time.
			     *
			     * The else case of this if statement is not needed
			     * as the driver which has the buffer keeps it
			     */
			    if (vs->vs_wants) {
				temp = vs->vs_id;
				vs->vs_id = vs->vs_wants;
				vs->vs_wants = temp;
			    }
			    break;
	
			case VS_IP:
			    return;
			    break;

			default:
			    printf ("vs_bufctl: unknown return value = 0x%x\n",
				rval);
		            return; 

		    }

		    if (vs->vs_id) {
			if (vs->vs_id == VS_SDC) {
			    rval = sdustart();
			}
		        else {
			    rval = st_start();
		        }
		    }
		}
	    }
	}
	else {
	    if (vs->vs_wants) {
		if (vs->vs_wants != id ) {
		    vs->vs_id = vs->vs_wants;
		    if (stillwant) {
			vs->vs_wants = id;
		    }
		    else {
			vs->vs_wants = VS_IDLE;
		    }
		}
		else {
		    panic("vs_bufctl owner wants buffer");
		    return;
		}
	    }
	    else {
		if (stillwant) {
		    vs->vs_id = id;
		}
		else {
		    vs->vs_id = VS_IDLE;
	        }
	    }
	    if (vs->vs_id) {
		if (vs->vs_id == VS_SDC) {
		    rval = sdustart();
		}
		else {
		    rval = st_start();
		}
		
		for (;;) {
		    switch (rval) {
			case VS_DONE:
			    if (vs->vs_wants) {
				vs->vs_id = vs->vs_wants;
				vs->vs_wants = VS_IDLE;
			    }
			    else {
				vs->vs_active = vs->vs_wants = vs->vs_id = VS_IDLE;
				return;
			    }
			    break;

			case VS_WANT:
			    /*
			     * A temporary variable (temp) is needed here as
			     * we cannot trust the "id" variable to be the
			     * driver that actually has the buffer at the
			     * present time.
			     *
			     * The else case of this if statement is not needed
			     * as the driver which has the buffer keeps it
			     */
			    if (vs->vs_wants) {
				temp = vs->vs_id;
				vs->vs_id = vs->vs_wants;
				vs->vs_wants = temp;
			    }
			    break;

			case VS_IP:
			    return;
			    break;

			default:
			    printf ("vs_bufctl: unknown return value = 0x%x\n",
				rval);
		            return; 

		    }
		    if (vs->vs_id) {
			if (vs->vs_id == VS_SDC) {
			    rval = sdustart();
			}
		        else {
			    rval = st_start();
		        }
		    }
		}
	    }
	    else {
		vs->vs_active = vs->vs_wants = vs->vs_id = VS_IDLE;
		return;
	    }
	}
}

