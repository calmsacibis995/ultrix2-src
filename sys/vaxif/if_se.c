#ifndef lint
static	char	*sccsid = "@(#)if_se.c	1.12	(ULTRIX)	3/18/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984, 85, 86 by			*
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
 *   University	of   California,   Berkeley,   and   from   Bell	*
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

/* ---------------------------------------------------------------------
 * Modification History:
 *
 *  26-Feb-87 jsd (John Dustin)
 *	Check return of iftype_to_proto so that we don't jump to 0.
 *	Also, added Ed Ferris DECnet fix (from 04-Jan-87) for counters
 *
 *  28-JAN-87  jsd (John Dustin)
 *      Lint cleanup; removed unused variables, etc.
 *
 *  30-Dec-86  jsd (John Dustin)
 *	Fixed DECnet physical/current ethernet address storage
 *	Added mser and mear register printfs on memory errors
 *
 *  20-Oct-86  jsd (John Dustin)
 *	Fixed bug in CRC computation (use len = 6, not 48)
 *	Changed serint to handle if system runs out of cluster mbufs
 *
 *  26-Sep-86  jsd (John Dustin)
 *	Changed panic detection to use cpudata[0].c_state == CPU_PANIC
 *	    instead of relying on panicstr != NULL
 *	Added serestart on MISS errors to prevent corrupted transmits
 *	    and on transmit errors if UFLO, BUFF or RTRY
 *	Fixed an mbuf initialization bug, and removed unused variables
 *
 *  12-Sep-86  jsd (John Dustin)
 *	Shut down the network if the system panics.
 *
 *   5-Sep-86  jsd (John Dustin)
 *	removed most debug statements, cleaned up comments, etc.
 *
 *  14-Aug-86  jsd (John Dustin)
 *	Many driver improvements.
 *
 *   5-Aug-86  jsd (John Dustin)
 *	Major rewrite "real ethernet driver".
 *
 *   2-Jul-86  fred (Fred Canter)
 *	Fixed more collisions with symbols in if_qe.c.
 *
 *  18-Jun-86  jsd (John Dustin)
 *	Created this VAXstar network controller driver.
 *	For Ethernet Lance chip implementation.
 *	Derived from if_qe.c.
 * 
 * ---------------------------------------------------------------------
 */

#include "se.h"
#if     NSE > 0 || defined(BINARY)
/*
 * Digital VAXSTAR Network Interface
 */
#include "../data/if_se_data.c"
extern struct protosw *iftype_to_proto(), *iffamily_to_proto();
extern struct timeval time;
extern struct nexus nexus[];
extern use_type1;	/* NFS: use type 1 mbuf clusters */
extern timeout();

int	seprobe(), seattach(), seintr();
int	seinit(),seoutput(),seioctl();
struct	mbuf *seget();
struct	mbuf *m_pullup();
struct	mbuf *se_tpullup();
char *km_alloc();

u_short sestd[] = { 0 };
struct	uba_driver sedriver =
	{ seprobe, 0, seattach, 0, sestd, "se", seinfo };

u_char sunused_multi[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* #define SEDEBUG */
#define SINT_NP	040	/* network primary, bit 5 */
unsigned long se_crc_table[16];	/* crc initialization table */
int sedebug = 0;	/* debug flag, range 0->4 */
int seshowmulti = 0;	/* debug: show multicast addresses add/delete */
#ifdef SEDEBUG
int seshowrecv = 0;	/* show input packet size/type/srcaddr [ 0..1] */
int seshowxmit = 0;	/* show output packet sizes [0..2] */
#endif
int se_bablcnt=0;	/* transmitter timeout counter */
int se_misscnt=0;	/* missed packet counter */
int se_merrcnt=0;	/* memory error counter */
int se_restarts=0;	/* number of times chip was restarted */
int seclusters = 2;	/* allocate additional mbuf clusters (+1 = 32 more) */
int se_crc;		/* crc, must be declared global */
unsigned int poly = 0xedb88320;	/* polynomial initialization */
int cpindex;		/* cpu index, always 0 */

/*
 * sc->rringaddr and sc->tringaddr are the start of km_alloc'd
 * memory for the receive and transmit descriptor rings.
 */
#define	RRING(elem) ((struct se_ring *) ((int)(sc->rringaddr) + ((sizeof(struct se_ring))*(elem))))
#define	TRING(elem) ((struct se_ring *) ((int)(sc->tringaddr) + ((sizeof(struct se_ring))*(elem))))

/*
 * Probe the Lance to see if it's there
 */
seprobe(reg)
	caddr_t reg;	/* not used */
{
 	register struct nb_regs *nb_addr = (struct nb_regs *)nexus;
 	register struct nb1_regs *nb1_addr = (struct nb1_regs *)qmem;
	register struct se_softc *sc = &se_softc;

	register struct se_initb *initb = (struct se_initb *) &se_initb;
	register int prp; 	/* physical addr ring pointer */
	int pi;			/* physical addr init block */
	int i, j, oldcvec;
	char *kp;		/* ptr to kmalloc'd area for rings */

#ifdef lint
	reg = reg;
#endif
	/*
	 * ONLY on a VAXstar/TEAMmate
	 */
	if ((cpu != MVAX_II) || (cpu_subtype != ST_VAXSTAR))
		return(0);

	/*
	 * Only if network option present.
	 */
	if ((vs_cfgtst&VS_NETOPT) == 0)
		return(0);

	nb1_addr->se_csrselect = SE_CSR0;
	nb1_addr->se_csr = SE_STOP;

	/*
	 * Allocate contiguous, quadword aligned (required)
	 * space for both descriptor rings; the km_alloc() routine
	 * actually octaword aligns the arena.
	 */
	kp = km_alloc(((sizeof(struct se_ring))*nSENTOT), KM_CLRSG);
	if (kp == NULL) {
		printf("se: cannot km_alloc memory for descriptor rings\n");
		return(0);
	}
	sc->rringaddr = (struct se_ring *) kp;
	sc->tringaddr = (struct se_ring *) ((int)kp + ((sizeof(struct se_ring))*nSENRCV));

	/* 
	 * Initialize multicast address array
	 */
	for (i=0; i<NMULTI; i++) {
		sc->muse[i] = 0;
		bcopy(sunused_multi,&sc->multi[i],MULTISIZE);
	}

	/*
	 * Initialize Lance chip with init block, se_initb
	 *
	  se_mode;			mode word
	  se_sta_addr;			station address
	  se_multi_mask;		multicast address mask
	  se_rcvlist_lo, se_rcvlist_hi;	rcv descriptor addr
	  se_rcvlen;			rcv length
	  se_xmtlist_lo, se_xmtlist_hi;	xmt descriptor addr
	  se_xmtlen;			xmt length
	 */

	initb->se_mode=0;	/* normal operation (mode==0) */

	for ( i=0, j=0; j < 3; j++ ) {
		initb->se_sta_addr[j] = (short)(nb_addr->nb_narom[4*i++]&0xff);
		initb->se_sta_addr[j] |= (short)((nb_addr->nb_narom[4*i++]&0xff)<<8);
	}

	/*
	 * Initialize multicast address mask
	 */
	for ( i=0 ; i<4 ; i++ ) {
		initb->se_multi_mask[i] = 0x0000;
	}
	/*
	 * initialize the multicast address CRC table,
	 * using initb->se_sta_addr as dummy variable.
	 */
	se_docrc(initb->se_sta_addr, SE_CRC_INIT);
	
	prp = svtophy(RRING(0));
	initb->se_rcvlist_lo = (short)prp & 0xffff;
	initb->se_rcvlist_hi = (short)(((int)prp >> 16) & 0xff);
	initb->se_rcvresv = 0;	/* reserved, should be 0 */
	initb->se_rcvlen = RLEN;

	if (initb->se_rcvlist_lo & 0x07) {	/* shouldn't happen */
		printf("se: rcvlist not on QUADword boundary");
		printf(": %x\n", initb->se_rcvlist_lo & 0x07);
		return(0);	/* so the Lance won't attempt work */
	}

	prp = svtophy(TRING(0));
	initb->se_xmtlist_lo = (short)prp & 0xffff;
	initb->se_xmtlist_hi = (short)(((int)prp >> 16) & 0xff);
	initb->se_xmtresv = 0;	/* reserved, should be 0 */
	initb->se_xmtlen = TLEN;

	if (initb->se_xmtlist_lo & 0x07) {	/* shouldn't happen */
		printf("se: xmtlist not on QUADword boundary");
		printf(": %x\n", initb->se_xmtlist_lo & 0x07);
		return(0);	/* so the Lance won't attempt work */
	}

	/*
	 * get physical address of init block for Lance
	 */
	/* set-up CSR 1 */
	nb1_addr->se_csrselect = SE_CSR1;
	pi = svtophy(initb);
	nb1_addr->se_csr = (short)(pi & 0xffff);

	/* set-up CSR 2 */
	nb1_addr->se_csrselect = SE_CSR2;
	nb1_addr->se_csr = (short)(((int)pi>>16) & 0xff);	/* hi 8 bits */

	nb1_addr->se_csrselect = SE_CSR0;

	nb_addr->nb_int_msk |= SINT_NP;		/* system network interrupt */

	oldcvec = cvec;

	/*
	 * clear IDON by writing 1, and start INIT sequence
	 */
	nb1_addr->se_csr = (SE_IDON | SE_INIT | SE_INT_ENABLE);

	/* wait for init done */
	j=0;
	while (j++ < 10000) {
		if ((nb1_addr->se_csr & SE_IDON) != 0) {
			/*printf("seprobe: INIT DONE: %d\n", j);*/
			break;
		}
		DELAY(10);
	}
	/* make sure got out okay */
	if ((nb1_addr->se_csr & SE_IDON) == 0) {
		if (nb1_addr->se_csr & SE_ERR) {
			printf("se: initialization error, csr = %04x\n",(nb1_addr->se_csr & 0xffff));
		} else {
			printf("se: cannot initialize Lance\n");
		}
		return(0);		/* didn't interrupt */
	}
	if (cvec == oldcvec)
		return(0);		/* didn't interrupt */

	/* set STOP to clear INIT and IDON (and everything else) */
	nb1_addr->se_csr = SE_STOP;

	return( sizeof(struct se_initb) );
}
 
/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.
 */

seattach(ui)
	struct uba_device *ui;
{
	register struct se_softc *sc = &se_softc;
	register struct nb_regs *nb_addr = (struct nb_regs *)nexus;
	register struct ifnet *ifp = &sc->is_if;
	register int i;
	register struct sockaddr_in *sin;

	use_type1 = 1;	/* keep NFS write size <= 1Kb */
	ifp->if_unit = ui->ui_unit;
	ifp->if_name = "se";
	ifp->if_mtu = ETHERMTU;
	ifp->if_flags |= IFF_BROADCAST | IFF_DYNPROTO;

	/*
	 * Read the address from the prom and save it.
	 */
	for ( i=0 ; i<6 ; i++ ) {
		sc->is_addr[i] = nb_addr->nb_narom[4*i] & 0xff;
	}

	sin = (struct sockaddr_in *)&ifp->if_addr;
	sin->sin_family = AF_INET;
	ifp->if_init = seinit;
	ifp->if_output = seoutput;
	ifp->if_ioctl = seioctl;
	ifp->if_reset = 0;
	if_attach(ifp);
}

/*
 * Initialization of interface and allocation of mbufs for receive ring
 */
seinit(unit)
	int unit;
{
	register struct se_softc *sc = &se_softc;
 	register struct nb1_regs *nb1_addr = (struct nb1_regs *)qmem;
	register struct se_initb *initb = (struct se_initb *) &se_initb;
	register struct ifnet *ifp = &sc->is_if;
	register i, s;

	/* address not known */
	if (ifp->if_addrlist == (struct ifaddr *)0)
		return;
	if (ifp->if_flags & IFF_RUNNING)
		return;

	/*
	 * If seclusters = 1, then we allocate 32 additional mbuf
	 * clusters for the system.  The system default is 32
	 * clusters, however, it is possible to run out since
	 * the se driver alone uses 16 of these, permanently mapped
	 * to the network receive buffers.  So the intent here is
	 * to make available to the system a new default of 64 clusters.
	 *
	 * If seclusters = 2, then we allocate an additional 32 clusters,
	 * for a total of 96 clusters in the system.  The expected
	 * value of seclusters is 1; the value of 2 is reserved.
	 */
	for (i=seclusters; i>0; i--) {
	    if (m_clalloc(4096*8/CLBYTES, MPG_CLUSTERS) == 0) {
		printf("se: mbuf cluster allocation failed (allocated %d/%d)\n",
			(seclusters - i)*32, seclusters*32 );
		break;
	    }
	}
	seclusters = 0;	/* reset */

	/*
	 * Init the buffer descriptors and
	 * indexes for each of the lists.
	 */
	for ( i = 0 ; i < nSENRCV ; i++ ) {
		seinitdesc( RRING(i), &sc->rmbuf[i], SEALLOC );
		(RRING(i)->se_flag) = SE_OWN;	/* give away rring */
	}
	for ( i = 0 ; i < nSENXMT ; i++ ) {
		seinitdesc( TRING(i), &sc->tmbuf[i], SENOALLOC );
	}
	sc->nxmit = sc->otindex = sc->tindex = sc->rindex = 0;

	/*
	 * Take the interface out of reset, program the vector, 
	 * enable interrupts, and tell the world we are up.
	 */
	s=splimp();

	nb1_addr->se_csrselect = SE_CSR0;

	if ( (ifp->if_flags & IFF_LOOPBACK)
	   && ((initb->se_mode & SE_LOOP) == 0) ) {
		/* if not in loopback mode, do loopback */
		initb->se_mode &= ~SE_INTL;
		initb->se_mode |= SE_LOOP;
		serestart(ifp);
		seinit(ifp->if_unit);
		splx(s);
		return;
	}

	/* start the Lance; enable interrupts, etc */
	nb1_addr->se_csr = (SE_START | SE_INT_ENABLE);

	ifp->if_flags |= (IFF_UP | IFF_RUNNING);
	sestart( unit );
	sc->ztime = time.tv_sec;
	/*
	 * Get index number of this cpu (always 0 for VAXstar)
	 */
	cpindex = cpuindex();
	if (cpindex != 0)
		printf("se: cpuindex (%d) not 0!\n",cpindex);
	splx(s);
}
 
/*
 * Ethernet interface interrupt processor
 */
seintr(unit)
	int unit;
{
	register struct se_softc *sc = &se_softc;
	register struct ifnet *ifp = &sc->is_if;
 	register struct nb1_regs *nb1_addr = (struct nb1_regs *)qmem;
	register s, oldcsr;
	struct nb_regs *nb_addr = (struct nb_regs *)nexus;

	s = splimp();

	/*
	 * If a set-1-to-reset bit is 1, then writing a 1
	 * back into the csr register will clear that bit.
	 * This is OK; the intent is to clear the csr of all
	 * errors/interrupts and then process the saved bits
	 * in the old csr.
	 */
	oldcsr = nb1_addr->se_csr;	/* save the old csr */
	oldcsr &= ~SE_INT_ENABLE;	/* clear interrupt enable */
	nb1_addr->se_csr = oldcsr;
	nb1_addr->se_csr = SE_INT_ENABLE; /* set interrupt enable */

	/*
	 * See if a panic has occurred, if so, halt the Lance
	 * so that the system can panic, dump core, and restart.
	 * If this is not done, we might lock out the cpu such
	 * that a valid panic/dump sequence can never occur.
	 */
	if ((cpudata[cpindex].c_state & CPU_PANIC) != 0) {
		nb1_addr->se_csrselect = SE_CSR0;
		nb1_addr->se_csr = SE_STOP;
		nb_addr->nb_int_msk &= ~SINT_NP;
		splx(s);
		return;
	}

	/*
	 * check error bits
	 */
	if ( oldcsr & SE_ERR ) {
		if (oldcsr & SE_MISS) {
			/*
			 * Stop the chip to prevent a corrupt packet from
			 * being transmitted.  There is a known problem with
			 * missed packet errors causing corrupted data to
			 * be transmitted to the same host as was just
			 * transmitted, with a valid crc appended to the
			 * packet.  The only solution is to stop the chip,
			 * which will clear the Lance silo, thus preventing
			 * the corrupt data from being sent.
			 */
			nb1_addr->se_csrselect = SE_CSR0;
			nb1_addr->se_csr = SE_STOP;
			se_misscnt++;
			if (sedebug)
			    mprintf("se: missed packet (MISS)\n");
			serestart(ifp);
			seinit(ifp->if_unit);
			splx(s);
			return;
		}
		if (oldcsr & SE_CERR) {
			sc->is_if.if_collisions++;
			if (sc->ctrblk.est_collis != 0xffff)
				sc->ctrblk.est_collis++;
		}
		if (oldcsr & SE_BABL) {
			se_bablcnt++;
			if (sedebug)
			    mprintf("se: transmitter timeout (BABL)\n");
		}
		if (oldcsr & SE_MERR) {
			se_merrcnt++;
			/* warn the user (printf on the terminal) */
			printf("se: memory error (MERR) \nmser = %x \nmear = %x\n",
				((struct nb_regs *)nexus)->nb_mser,
				((struct nb_regs *)nexus)->nb_mear);

			if (((oldcsr & SE_RXON) == 0)
			    || ((oldcsr & SE_TXON) == 0)) {
				serestart(ifp);
				seinit(ifp->if_unit);
				splx(s);
				return;
			}
		}
	}

	if ( oldcsr & SE_RCV_INT ) 
		serint( unit );

	if ( oldcsr & SE_XMIT_INT )
		setint( unit );

	if (oldcsr == (SE_INTR|SE_RXON|SE_TXON))
		mprintf("se: stray interrupt\n");

	splx(s);
}
 
/*
 * Ethernet interface transmit interrupt.
 */
setint(unit)
	int unit;
{
	register struct se_softc *sc = &se_softc;
	register first, index;
	register struct se_ring *rp;
	register i, flag;

	struct mbuf *mp, *mp0;
	struct ifnet *ifp = &sc->is_if;
	struct ether_header *eh;
	int ring_cnt, flag2;
	short len;

	/*
	 * While we haven't caught up to current transmit index,
	 * AND the buffer is ours, AND there are still transmits
	 * which haven't been pulled off the ring yet, proceed
	 * around the ring in search of the last packet.
	 */

	while ( (sc->otindex != sc->tindex)
	 && !((TRING(sc->otindex)->se_flag) & SE_OWN) && sc->nxmit > 0 ) {

		first = index = sc->otindex;

		/* shouldn't happen */
		if ( !((TRING(sc->otindex)->se_flag) & SE_STP)) {
			if (sedebug)
				mprintf("se: no STP in first transmit packet\n");
			break;
		}

		/*
		 * Find the index of the last descriptor in the
		 * packet (SE_ENP will be set).
		 * While we own it, and it's not the last packet,
		 * keep searching.
		 */
		/* counting the one above, use 1 not 0 */
		ring_cnt = 1;	/* don't beat Lance around the ring */
		while ((((TRING(index)->se_flag) & (SE_OWN|SE_ENP)) == 0)
		    && (ring_cnt++ <= nSENXMT)) {
			index = ++index % nSENXMT;
		}
		/*
		 * Went all the way around, wait
		 */
		if (ring_cnt > nSENXMT) {
			if (sedebug)
				mprintf("se: xmit ring_cnt exceeded\n");
			break;
		}
		/*
		 * Lance owns it, wait
		 */
		if ( (TRING(index)->se_flag) & SE_OWN ) {
			break;
		}
		/*
		 * Not end of packet, wait
		 */
		if ( !((TRING(index)->se_flag) & SE_ENP) ) {
			break;
		}
		/*
		 * Found last buffer in the packet
		 * (hence a valid string of descriptors)
		 * so free things up.
		 */
		rp = TRING(sc->otindex);
		mp = sc->tmbuf[sc->otindex];
		flag = rp->se_flag;
		flag2 = rp->se_flag2;

		/*
		 * Init the buffer descriptor
		 */
		seinitdesc(TRING(first), &sc->tmbuf[first], SENOALLOC);

		while (first != index) {
			first = ++first % nSENXMT;
			seinitdesc(TRING(first), &sc->tmbuf[first], SENOALLOC);
		}
		/* increment old index pointer, if it catches up
		 * with current index pointer, we will break out
		 * of the loop.  Otherwise, go around again
		 * looking for next end-of-packet cycle.
		 */
		sc->otindex = ++index % nSENXMT;
		--sc->nxmit;
		sc->is_if.if_opackets++;

		/*
		 * DECnet statistics
		 */
		/* exactly one collision? */
		if ((flag & SE_ONE) && !(flag2 & SE_LCOL)) {
			if (sc->ctrblk.est_single != (unsigned)0xffffffff)
				sc->ctrblk.est_single++;
		/* more than one collision? */
		} else if (flag & SE_MORE) {
			if (sc->ctrblk.est_multiple != (unsigned)0xffffffff)
				sc->ctrblk.est_multiple++;
		}

		/*
		 * Check for transmission errors.
		 * This assumes that transmission errors
		 * are always reported in the last packet.
		 */
		if (flag & SE_RT_ERR) {
			sc->is_if.if_oerrors++;
			if (sc->ctrblk.est_sendfail != 0xffff) {
				sc->ctrblk.est_sendfail++;
				if (flag2 & SE_RTRY) {
					/* excessive collisions */
					if (sedebug) mprintf("se: excessive collisions (RTRY)\n");
					sc->ctrblk.est_sendfail_bm |= 1;
				}
				if (flag2 & SE_LCOL) {
					if (sedebug) mprintf("se: late transmit collision (LCOL)\n");
					; /* not implemented */
				}
				if (flag2 & SE_LCAR) {
					if (sedebug) mprintf("se: lost carrier during transmit (LCAR)\n");
					sc->ctrblk.est_sendfail_bm |= 2;
				}
				if (flag2 & SE_UFLO) {
					if (sedebug) mprintf("se: packet truncated (UFLO)\n");
				}
				if (flag2 & SE_TBUFF) {
					if (sedebug) mprintf("se: transmit buffer error (BUFF)\n");
				}
			}
			/*
			 * Restart chip if transmitter got turned off
			 * due to transmit errors: UFLO, TBUFF or RTRY.
			 */
			if (flag2 & (SE_UFLO | SE_TBUFF | SE_RTRY)) {
				serestart(ifp);		/* will free mp */
				seinit(ifp->if_unit);
				return;
			}
			m_freem(mp);
		} else {
			/*
			 * If this was a broadcast packet loop it
			 * back, otherwise just free the packet.
			 */
			if (mp) {
				eh = mtod( mp, struct ether_header *);
				for (i=0; (i < 6) && (eh->ether_dhost[i] == 0xff); i++)
					; /*nop*/
				if ( i == 6 ) {
					for ( mp0 = mp, len=0 ; mp0 ; mp0 = mp0->m_next ) {
						len += mp0->m_len;
					}
					seread( sc, mp, len );
				} else {
					m_freem( mp );
				}
			}
		}
	}
	/*
	 * Dequeue next transmit request, if any.
	 */
	sestart( unit );
}
 
/*
 * Ethernet interface receiver interrupt.
 * If can't determine length from type, then have to drop packet.  
 * Othewise decapsulate packet based on type and pass to type specific 
 * higher-level input routine.
 */
serint(unit)
	int unit;
{
	register struct se_softc *sc = &se_softc;
	register struct se_ring *rp;
	register struct mbuf *head; /* head of new chain */
	register index, first, len;

	struct mbuf *n, *m, *tmp;
	struct mbuf *prev;	/* ptr to current mbuf within chain */
	u_short flag;	/* saved status flag */
	int ring_cnt;
	int mbuf_error;	/* mbuf allocation error flag */
	int second;	/* index of 2nd descriptor in 2 mbuf chain */
	int savlen;	/* saved length of the incoming packet */
#ifdef lint
	unit = unit;
#endif

	/*
	 * Traverse the receive ring looking for packets to pass back.
	 * The search is complete when we find a descriptor that is in
	 * use (owned by Lance).
	 */

	for ( ; !((RRING(sc->rindex)->se_flag) & SE_OWN); sc->rindex = ++index % nSENRCV ) {
		first = index = sc->rindex;

		/*
		 * If not the start of a packet, error
		 */
		if ( !((RRING(index)->se_flag) & SE_STP)) {
			if (sedebug) {
				mprintf("se: recv: start of packet expected #%d, flag=%02x\n", index,(RRING(index)->se_flag&0xff));
			}
			break;
		}

		/*
		 * Find the index of the last descriptor in this
		 * packet (SE_OWN clear and SE_ENP set). If cannot
		 * find it then Lance is still working.
		 */
		ring_cnt=1;
		while ((((RRING(index)->se_flag) & (SE_RT_ERR|SE_OWN|SE_ENP)) == 0)
		   && (ring_cnt++ <= nSENRCV)) {
			index = ++index % nSENRCV;
		}

		/*
		 * Went all the way around, wait
		 */
		if (ring_cnt > nSENRCV) {
			if (sedebug)
				mprintf("se: recv ring_cnt exceeded\n");
			break;
		}

		/*
		 * Is it a valid chain of data, ie.
		 * we own the descriptor (SE_OWN==0),
		 * and encountered end of packet (ENP) set?
		 */
		if ( ((RRING(index)->se_flag) & ~SE_STP) == SE_ENP) {
			sc->is_if.if_ipackets++;
			rp = RRING(index);
			/* len here includes the 4 CRC bytes */
			len = (rp->se_flag2 & SE_MCNT);

			/*
			 * Link the mbufs together, reinitialize the
			 * descriptors and pass the mbuf chain off to
			 * seread() if status okay.
			 */

			/*
			 * Keep track of the mapped mbufs, using
			 * m and n to point to the first and optional
			 * second cluster.
			 */
			n = NULL;	/* initialize second mbuf pointer */
			second = -1;	/* initialize second entry */
			head = NULL;	/* head is new chain head */
			prev = NULL;	/* prev is current mbuf in new chain */
			savlen = len;	/* saved length of packet */
			mbuf_error = 0;

			m = sc->rmbuf[first];/* initialize first mbuf pointer */
			if (first != index) {
				second = first;
				second = ++second % nSENRCV;
				if (second != index) {
					if (sedebug) mprintf("se: 3 chained clusters (%d/%d/%d,input len=%d)\n",first,second,index,len);
					/* ignore the third packet */
				}
				n = sc->rmbuf[second];
			}

			/*
			 * process a cluster mbuf each time through the loop
			 */
			do {  /* while (len > 0) */
				if (len <= MLEN) {
					if (len <= 4) {
						RRING(second)->se_flag = SE_OWN;
						head->m_len -= (4-len);
						break;
					}
					MGET(tmp, M_DONTWAIT, MT_DATA);
					if (tmp == NULL) {
						if (sedebug)
							printf("se: no mbuf2\n");
						mbuf_error++;
						break;	/* done */
					}
					tmp->m_off = MMINOFF;
					/* subtract 4 byte CRC */
					tmp->m_len = len-4;
					if (head == NULL) {
						bcopy( mtod(m, char *), mtod(tmp, char *), (unsigned) len);
						head = prev = tmp;
						RRING(first)->se_flag = SE_OWN;
					} else {
						bcopy( mtod(n, char *), mtod(tmp, char *), (unsigned) len);
						prev->m_next = tmp;
						RRING(second)->se_flag = SE_OWN;
					}
					break; /* done */
				} else {  /* found enough for a cluster mbuf */
					int i;
					if (head == NULL)
						i = first;
					else
						i = second;
					if ( seinitdesc(RRING(i),
						&sc->rmbuf[i], SEALLOC) < 0 ) {
						/*
						 * If we run out of clusters,
						 * drop the packet and continue.
						 */
						mbuf_error++;
						break;
					}
					/*
					 * ...else cluster replacement success
					 */
					(RRING(i)->se_flag) = SE_OWN; /* give away rring */
					/*
					 * If head is null, then this is the
					 * 1st time through loop, so we are
					 * doing cluster 'm'.  If head is not
					 * null, then we must have put something
					 * here last time (m) so use 'n' here.
					 * We could just as easily have checked
					 * to see if len>CLBYTES-2, if so, then
					 * this is 1st time, else it's the 2nd.
					 */
					if (head == NULL) {
						head = prev = m;
					} else {
						prev->m_next = n;
						prev = prev->m_next;
					}
					/* set correct length if last mbuf */
					if ( len < (CLBYTES - 2) )
					/* subtract 4 byte CRC */
						prev->m_len = len-4;
					len -= (CLBYTES - 2);
				}
			} while (len > 0);

			if (mbuf_error) {
				/*
				 * We encountered an error somewhere in
				 * allocating mbufs, so just drop the
				 * packet and return.
				 */
				/* re-map previously mapped buffers */
				if (head == NULL)
					RRING(first)->se_flag = SE_OWN;
				if (second != -1) {
				    RRING(second)->se_flag = SE_OWN;
				}
				m_freem(head);
			} else {
				/* subtract 4 byte CRC */
				seread(sc, head, savlen-4);
			}
		/*
		 * else Not a good packet sequence, check for receiver errors
		 */
		} else if ((RRING(index)->se_flag) & SE_RT_ERR) {
			sc->is_if.if_ierrors++;

			flag = RRING(index)->se_flag;
			if (flag & (SE_OFLO | SE_CRC | SE_FRAM | SE_RBUFF)) {
				if (sedebug)
					mprintf("se: recv err %02x\n", flag&0xff);
				if (sc->ctrblk.est_recvfail != 0xffff) {
					sc->ctrblk.est_recvfail++;
					if (flag & SE_OFLO) {
						sc->ctrblk.est_recvfail_bm |= 4;
					}
					if (flag & SE_CRC) {
				    		sc->ctrblk.est_recvfail_bm |= 1;
					}
					if (flag & SE_FRAM) {
					    	sc->ctrblk.est_recvfail_bm |= 2;
					}
					if (flag & SE_RBUFF) {
				    		; /* causes SE_OFLO to be set */
					}
				}
			} else {
				if (sedebug)
					mprintf("se: stray recv err %02x\n",flag&0xff);
			}
			/*
			 * "free" up the descriptors, but really just use
			 * them over again.  Give them back to the Lance
			 * using the same pre-allocated mbufs.
			 */

			RRING(first)->se_flag = SE_OWN;
			while ( first != index ) {
				first = ++first % nSENRCV;
				RRING(first)->se_flag = SE_OWN; /* give away */
			}
		} else {
			/*  else:
			 *   - not a good packet,
			 *   - not an error situation,
			 *
			 * This can happen if we beat the Lance to the
			 * end of a valid list of receive buffers -- the
			 * Lance hasn't relinquished the last buffer (one
			 * with ENP set) or we just found a buffer still
			 * owned by Lance, without finding the ENP bit.
			 * In either case, just return.  We can pick up
			 * the unfinished chain on the next interrupt.
			 */
			return;
		}
	}
}

/*
 * Ethernet output routine.
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 */
seoutput(ifp, m0, dst)
	register struct ifnet *ifp;
	struct mbuf *m0;
	struct sockaddr *dst;
{
	register struct se_softc *is = &se_softc;
	register struct mbuf *m = m0;
	register struct ether_header *eh;
	register int off;
	struct in_addr idst;

	int type, s, error;
	u_char edst[6];
	struct protosw *pr;

	switch (dst->sa_family) {

#ifdef INET
	case AF_INET:
		if (nINET == 0) {
			printf("se: can't handle af%d\n", dst->sa_family);
			error = EAFNOSUPPORT;
			m_freem(m0);
			return(error);
		}
		idst = ((struct sockaddr_in *)dst)->sin_addr;
		if (!arpresolve(&is->is_ac, m, &idst, edst)) {
			return (0);	/* if not yet resolved */
		}
		off = ntohs((u_short)mtod(m, struct ip *)->ip_len) - m->m_len;
		/* need per host negotiation */
		if ((ifp->if_flags & IFF_NOTRAILERS) == 0) {
		    if (off > 0 && (off & 0x1ff) == 0 &&
			m->m_off >= MMINOFF + 2 * sizeof (u_short)) {
			type = ETHERTYPE_TRAIL + (off>>9);
			m->m_off -= 2 * sizeof (u_short);
			m->m_len += 2 * sizeof (u_short);
			*mtod(m, u_short *) = htons((u_short)ETHERTYPE_IP);
			*(mtod(m, u_short *) + 1) = htons((u_short)m->m_len);
			goto gottraqeertype;
		    }
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
		 * Try to find other address families and call protocol
		 * specific output routine.
		 */
		if (pr = iffamily_to_proto(dst->sa_family)) {
			(*pr->pr_ifoutput)(ifp, m0, dst, &type, (char *)edst);
			goto gottype;
		} else {
			printf("se: can't handle af%d\n", dst->sa_family);
			error = EAFNOSUPPORT;
			m_freem(m0);
			return(error);
		}
	}

gottraqeertype:
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

	if (m->m_off > MMAXOFF || MMINOFF + sizeof (struct ether_header) > m->m_off) {
		m = m_get(M_DONTWAIT, MT_HEADER);
		if (m == 0) {
			error = ENOBUFS;
			m_freem(m0);
			return(error);
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
 	bcopy((caddr_t)is->is_addr, (caddr_t)eh->ether_shost, sizeof (is->is_addr));
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
	sestart(ifp->if_unit);
	splx(s);
	return(0);
}
 
/*
 * Start output on interface.
 *
 */
sestart(unit)
int	unit;
{
	register struct se_softc *sc = &se_softc;
	register struct mbuf *m;
	register int dp; 	/* data buffer pointer */
	register struct se_ring *rp;
	register len, j;
	int desc_needed, i, tlen, s;
	int index;
	struct mbuf *mp;

#ifdef lint
	unit = unit;
#endif
	/*
	 * Check for enough transmit descriptors.  If there will never
	 * be enough throw the packet away and complain. If there will
	 * be enough but there aren't right now just return and if there
	 * are enough now, init one or more descriptors to map the packet
	 * onto the interface and start it.
	 */
	s = splimp();
	if ( (m = sc->is_if.if_snd.ifq_head) == 0 ){
		splx(s);
		return;		/* Nothing on the queue	*/
	}

	/* one transmit descriptor per mbuf (large or small mbuf) */
	for ( desc_needed = 0 ; m ; m = m->m_next )
		desc_needed++;

	/* should never happen; previously a panic */
	if ( desc_needed > nSENXMT ) {
		printf("se: impossibly few XMT descriptors, need %d, have %d\n",
			desc_needed, nSENXMT);
		IF_DEQUEUE(&sc->is_if.if_snd, m);
		m_freem(m);
		splx(s);
		return;
	}

	/*
	 * See if the required descriptors are currently available.
	 * (ie. do we own enough of them to send out this packet?)
	 */
	
	/*
	 * Calculate available descriptors without
	 * traversing the whole ring.
	 */
	if (sc->otindex < sc->tindex)
		i = nSENXMT - (sc->tindex - sc->otindex);
	else if (sc->otindex > sc->tindex)
		i = sc->otindex - sc->tindex;
	else {
		/* indexes are same */
		i = nSENXMT;
	}

	if (desc_needed > i) {
		if (sedebug)
			mprintf("se: too few XMT descriptors (need %d, have %d).\n", desc_needed, i);
		splx(s);
		return;
	}

	/*
	 * Dequeue the first transmit request, if any.
	 */
	IF_DEQUEUE(&sc->is_if.if_snd, m);

	/*
	 * Pull data from later mbufs into the leading mbufs.
	 */
#ifdef SEDEBUG
	if (seshowxmit>1) {
		cprintf("B                                 \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	}
#endif
	mp = se_tpullup(m);
	/* se_tpullup can fail (returns NULL) if m_pullup fails;
	 * this results in the mbuf chain being free'd automatically.
	 */
	if (!mp) {
		mprintf("se: se_tpullup failed\n");
		splx(s);
		return;
	}
	m = mp;

	/*
	 * calulate new number of descriptors needed
	 * since the pullup above usually rearranges
	 * the data sizes, hence the number mbufs used.
	 */
	for ( desc_needed = 0 ; m ; m = m->m_next )
		desc_needed++;
	m = mp;

 	/*
	 * Record the chain head and attach each mbuf
	 * data area to a descriptor.  The Lance looks
	 * at the OWN bit to determine if it should
	 * transmit the packet.
	 */
	tlen = 0;
	index = sc->tindex;
	sc->tmbuf[index] = m;
#ifdef SEDEBUG
	if (seshowxmit) {
		cprintf("T                                 \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	}
#endif
	for (  ; m ;  m=m->m_next, index = ++index % nSENXMT ) {
		rp = TRING(index);
		dp = svtophy( mtod( m, int) );
		len = m->m_len;
		tlen += len;
#ifdef SEDEBUG
		if (seshowxmit)
			cprintf("%d ", len);
#endif
		rp->se_buf_len = -(len);
		rp->se_addr_lo = (short)dp & 0xffff;
		rp->se_addr_hi = (short)(((int)dp >> 16) & 0xff);

		/*
		 * Is this the last descriptor?
		 */
		if ( m->m_next == NULL ) {
			rp->se_flag |= SE_ENP;	/* end of packet */ 
			len = -(rp->se_buf_len);
			/* don't send runt packet if only 1 descriptor */
			if ((desc_needed == 1) && (len < MINDATA)) {
				len = MINDATA;		/* 64 */
				rp->se_buf_len = -(len);
			}
			/*
			 * Give away transmit descriptors
			 * in reverse order so Lance does not
			 * beat us down the ring.
			 */
			for ( j = index ; j != sc->tindex ; ) {
				/*
				if ( TRING(j)->se_flag & SE_OWN ) {
				    if (sedebug) printf("se: xmit twice, i=%d, j=%d, sc=%d, need=%d\n",i,j,sc->tindex, desc_needed);
				}
				*/
				(TRING(j)->se_flag) |= SE_OWN;
				j = --j >= 0 ? j : nSENXMT;
			}
			/*
			 * This first packet must be > 100, which will
			 * be true, unless there is only 1 packet.
			 */
			len = -(TRING(j)->se_buf_len);
			if (desc_needed > 1) {
				if ( len < 100 ) {
					mprintf("se: initial packet < 100 bytes, len=%d\n", len);
				}
			} else {
				/* 1 and only 1 buffer */
				if ( len < MINDATA ) {	/* 64 */
					mprintf("se: runt packet, len=%d\n",len);
				}

				/* also, should find ENP set */
				if ( !((TRING(j)->se_flag) & SE_ENP)) {
					mprintf("se: 1-1st packet does not have ENP set\n");
				}
				if (index != j) {
					mprintf("se: 1st index (%d) != j (%d)\n",index,j);
				}
			}

			/*
			 * Mark the first packet with STP
			 * and give it to the Lance.
			 */
			(TRING(j)->se_flag) |= (SE_OWN | SE_STP);
					
#ifdef SEDEBUG
			if (seshowxmit)
				cprintf("= %d\n", tlen);
#endif
			sc->nxmit++;
			break;
		}
	}
	sc->tindex = ++index % nSENXMT;

	/*
	 * Accumulate statistics for DECnet
	 */
	if ((sc->ctrblk.est_bytesent + tlen) > sc->ctrblk.est_bytesent)
		sc->ctrblk.est_bytesent += tlen;
	if (sc->ctrblk.est_bloksent != (unsigned)0xffffffff)
		sc->ctrblk.est_bloksent++;
	splx(s);
}

/*
 * Process an ioctl request.
 */
seioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	register struct se_softc *sc = &se_softc;
	register struct se_initb *initb = (struct se_initb *) &se_initb;
	register int i;

	struct nb_regs *nb_addr = (struct nb_regs *)nexus;
	struct ifreq *ifr = (struct ifreq *)data;
	struct ifdevea *ifd = (struct ifdevea *)data;
	struct ctrreq *ctr = (struct ctrreq *)data;
	struct protosw *pr;
	struct ifaddr *ifa = (struct ifaddr *)data;
	int bitpos;		/* top 6 bits of crc = bit in multicast mask */
	u_short newmask[4];	/* new value of multicast address mask */
	int j = -1, s, error=0;

	s = splimp();
	switch (cmd) {

	case SIOCENABLBACK:
		if (sedebug>1) printf("SIOCENABLBACK ");
		printf("se: internal loopback enable requested\n");

		/* set external loopback */
		initb->se_mode &= ~SE_INTL;
		initb->se_mode |= SE_LOOP;
		serestart( ifp );
		ifp->if_flags |= IFF_LOOPBACK;
		seinit( ifp->if_unit );
		break;
 
	case SIOCDISABLBACK:
		if (sedebug>1) printf("SIOCDISABLBACK ");
		printf("se: internal loopback disable requested\n");

		/* disable external loopback */
		initb->se_mode &= ~SE_INTL;
		initb->se_mode &= ~SE_LOOP;
		serestart( ifp );
		ifp->if_flags &= ~IFF_LOOPBACK;
		seinit( ifp->if_unit );
		break;
 
	case SIOCRPHYSADDR:
		/*
		 * read default hardware address.
		 */
		if (sedebug>1) printf("SIOCRPHYSADDR ");
		bcopy(sc->is_addr, ifd->current_pa, 6);
		for ( i=0 ; i<6 ; i++ ) {
			ifd->default_pa[i] = nb_addr->nb_narom[4*i] & 0xff;
		}
		break;
 
	case SIOCSPHYSADDR:
		if (sedebug>1) printf("SIOCSPHYSADDR ");
		bcopy(ifr->ifr_addr.sa_data, sc->is_addr, 6);
		bcopy(sc->is_addr, initb->se_sta_addr, 6);

		if (ifp->if_flags & IFF_RUNNING) {
			serestart(ifp);
			seinit(ifp->if_unit);
		} else {
			serestart(ifp);
		}
		break;

	case SIOCDELMULTI:
	case SIOCADDMULTI:
		if (cmd == SIOCDELMULTI) {
			if (sedebug>1) printf("SIOCDELMULTI ");
			for (i = 0; i < NMULTI; i++)
				if (bcmp(&sc->multi[i],ifr->ifr_addr.sa_data,MULTISIZE) == 0) {
					if (--sc->muse[i] == 0) {
						bcopy(sunused_multi,&sc->multi[i],MULTISIZE);
					}
					if (seshowmulti) printf("%d deleted.\n",i);
				}
		} else {
			if (sedebug>1) printf("SIOCADDMULTI ");
			for (i = 0; i < NMULTI; i++) {
				if (bcmp(&sc->multi[i],ifr->ifr_addr.sa_data,MULTISIZE) == 0) {
					sc->muse[i]++;
					if (seshowmulti) printf("already using index %d\n", i);
					goto done;
				}
				if (bcmp(&sc->multi[i],sunused_multi,MULTISIZE) == 0)
					j = i;
			}
			/*
			 * j is initialized to -1; if j > 0, then
			 * represents the last valid unused location
			 * in the multicast table.
			 */
			if (j == -1) {
				printf("se: SIOCADDMULTI failed, multicast list full: %d\n",NMULTI);
				error = ENOBUFS;
				goto done;
			}
			bcopy(ifr->ifr_addr.sa_data, &sc->multi[j], MULTISIZE);
			sc->muse[j]++;

			if (seshowmulti)
				printf("added index %d.\n", j);
		}
		/*
		 * Recalculate all current multimask crc/bits
		 * and reload multimask info.
		 *
		 * For each currently used multicast address,
		 * calculate CRC, save top 6 bits, load
		 * appropriate mask bit into newmask[i]
		 */
		for (i=0; i<4; i++)
			newmask[i] = 0x0000;

		for (i=0; i<NMULTI; i++) {
			if (sc->muse[i] == 0)
				continue;
			/* returns 32-bit crc in global variable _se_crc */
			se_docrc(&sc->multi[i], 0);
			bitpos = ((unsigned int)se_crc >> 26) & 0x3f;
			if (seshowmulti)
				printf("crc=%08x, bit=%d.\n",se_crc,bitpos);

			/* 0-15 */
			if (bitpos >= 0 && bitpos < 16)
				newmask[0] |= (1 << (bitpos - 0));
			/* 16-31 */
			else if (bitpos < 32)
				newmask[1] |= (1 << (bitpos - 16));
			/* 32-47 */
			else if (bitpos < 48)
				newmask[2] |= (1 << (bitpos - 32));
			/* 48-63 */
			else if (bitpos < 64)
				newmask[3] |= (1 << (bitpos - 48));
			else {
				if (sedebug || seshowmulti)
					printf("se: bad crc, bitpos=%d.\n", bitpos);
			}
		}
		for (i=0; i<4; i++) {
			initb->se_multi_mask[i] = newmask[i] & 0xffff;
		}
		if (seshowmulti) {
		    printf("new 64-bit multimask= %04x %04x %04x %04x\n",
			initb->se_multi_mask[3], initb->se_multi_mask[2],
			initb->se_multi_mask[1], initb->se_multi_mask[0]);
		}

		if (ifp->if_flags & IFF_RUNNING) {
			serestart(ifp);
			seinit(ifp->if_unit);
		} else {
			serestart(ifp);
		}
		break;

	case SIOCRDCTRS:
	case SIOCRDZCTRS:

		if (sedebug>1) printf("SIOCRDCTRS ");
		ctr->ctr_ether = sc->ctrblk;
		ctr->ctr_type = CTR_ETHER;
		ctr->ctr_ether.est_seconds = (time.tv_sec - sc->ztime) > 0xfffe ? 0xffff : (time.tv_sec - sc->ztime);
		if (cmd == SIOCRDZCTRS) {
			if (sedebug>1) printf("SIOCRDZCTRS ");
			sc->ztime = time.tv_sec;
			bzero(&sc->ctrblk, sizeof(struct estat));
		}
		break;

	case SIOCSIFADDR:
		if (sedebug>1) printf("SIOCSIFADDR ");
		ifp->if_flags |= IFF_UP;
		seinit(ifp->if_unit);
		switch(ifa->ifa_addr.sa_family) {
#ifdef INET
		case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr =
				IA_SIN(ifa)->sin_addr;
			arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
			break;
#endif

		default:
			if (pr=iffamily_to_proto(ifa->ifa_addr.sa_family)) {
				error = (*pr->pr_ifioctl)(ifp, cmd, data);
			}
			break;
		}
		break;
	default:
		error = EINVAL;
	}
done:	splx(s);
	return (error);
}

/*
 * Calculate 32-bit CRC (AUTODIN-II) for the given 48-bit
 * multicast address.  The CRC table must first be initialized
 * for the vax crc instruction to work.  The crc is returned in
 * global variable _se_crc.
 */
se_docrc(addr, flag)
	struct se_multi *addr;
	int flag;
{
	/*
	 * NOTE: do not change the order of these
	 * register declarations due to asm() instruction
	 * used below.
	 */
	register unsigned long *tbl_addr;	/* initialization table addr */
	register int inicrc;			/* initial CRC */
	register int len;			/* length of multicast addr */
	register struct se_multi *multi_addr;	/* multicast address, 48 bits */

	unsigned int tmp, x;
	int index, j;

	/* initialize polynomial table, only done once from seprobe() */
	if (flag & SE_CRC_INIT) {
		for (index=0; index<16; index++) {
			tmp = index;
			for (j=0; j<4; j++) {
			    x = (tmp & 0x01);
			    tmp = (tmp >> 1);	/* logical shift right 1 bit */
			    if (x == 1)
				tmp = (tmp ^ poly);	/* XOR */
			}
			se_crc_table[index] = tmp;
		}
		return(0);
	}

	if (seshowmulti) {
		printf("addr=%x.%x.%x.%x.%x.%x, ",
			(struct se_multi *)addr->se_multi_char[0],
			(struct se_multi *)addr->se_multi_char[1],
			(struct se_multi *)addr->se_multi_char[2],
			(struct se_multi *)addr->se_multi_char[3],
			(struct se_multi *)addr->se_multi_char[4],
			(struct se_multi *)addr->se_multi_char[5]);
	}

	/* initialize arguments */
	tbl_addr = se_crc_table;
	inicrc = -1;
	len = 6;
	multi_addr = addr;

#ifdef lint
	tbl_addr = tbl_addr;
	inicrc = inicrc;
	len = len;
	multi_addr = multi_addr;
#endif
	/* calculate CRC */
	asm( "crc	(r11),r10,r9,(r8)" );
	asm( "movl	r0, _se_crc" );

	return(0);
}


/*
 * Restart the Lance chip -- this routine is called:
 *   - after changing the multicast address filter mask
 *   - on any loopback mode state change
 *   - on any error which disables the transmitter
 *   - on all missed packet errors
 *
 * The routine first halts the Lance chip, clears any previously
 * allcated mbufs, and then re-initializes the hardware (same as in
 * seprobe() routine).  The seinit() routine is usually called next
 * to reallocate mbufs for the receive ring, and then actually
 * start the chip running again.
 */
serestart( ifp )
	register struct ifnet *ifp;
{
	register struct se_softc *sc = &se_softc;
	register struct se_initb *initb = (struct se_initb *) &se_initb;
 	register struct nb1_regs *nb1_addr = (struct nb1_regs *)qmem;
	register int i, pi;

	/*
	 * stop the chip
	 */
	nb1_addr->se_csrselect = SE_CSR0;
	nb1_addr->se_csr = SE_STOP;

	/*
	 * stop network activity
	 */
	if (ifp->if_flags & IFF_RUNNING) {
		ifp->if_flags &= ~(IFF_UP | IFF_RUNNING);
	}
	se_restarts++;

	if (sedebug)
		mprintf("serestart: restarted se  %d\n", se_restarts);

	/*
	 * free up any mbufs currently in use
	 */
	for (i=0; i<nSENRCV; i++) {
		if (sc->rmbuf[i])
			m_freem(sc->rmbuf[i]);
	}
	for (i=0; i<nSENXMT; i++) {
		if (sc->tmbuf[i])
			m_freem(sc->tmbuf[i]);
	}

	/*
	 * reload Lance with init block
	 */
	nb1_addr->se_csrselect = SE_CSR1;
	pi = svtophy(initb);
	nb1_addr->se_csr = (short)(pi & 0xffff);
	nb1_addr->se_csrselect = SE_CSR2;
	nb1_addr->se_csr = (short)(((int)pi>>16) & 0xff);

	/*
	 * clear IDON, and INIT the chip
	 */
	nb1_addr->se_csrselect = SE_CSR0;
	nb1_addr->se_csr = (SE_IDON | SE_INIT | SE_INT_ENABLE);

	i = 0;
	while (i++ < 10000) {
		if ((nb1_addr->se_csr & SE_IDON) != 0) {
			break;
		}
		DELAY(10);
	}
	/* make sure got out okay */
	if ((nb1_addr->se_csr & SE_IDON) == 0) {
		if (nb1_addr->se_csr & SE_ERR)
			printf("serestart: initialization ERR csr0=%04x\n",
				(nb1_addr->se_csr & 0xffff));
		else
			printf("serestart: cannot initialize Lance\n");
	}

	/* set STOP to clear INIT and IDON */
	nb1_addr->se_csr = SE_STOP;
	return(0);
}

/*
 * Initialize a ring descriptor with mbuf allocation side effects
 * Returns -1 if cannot get mbuf or mbuf cluster, otherwise 0.
 */
seinitdesc( rp, mp, option )
	register struct se_ring *rp;
	struct mbuf **mp;
	int option;
{
	register struct mbuf *m, *p;
	register int dp;		/* data pointer */

	/*
	 * clear the entire descriptor
	 */
	if ( option != SEALLOC ) {
		bzero( rp, sizeof(struct se_ring));
		*mp = NULL;
	}

	/*
	 * Perform the necessary allocation/deallocation
	 */
	else {	/* option == SEALLOC */
		/* get regular mbuf */
		MGET(m, M_DONTWAIT, MT_DATA);
		if ( m == 0 ) {
			if (sedebug)
			    printf("se: no mbuf1\n");
			return(-1);
		}
		/* get cluster mbuf */
		MCLGET(m, p);
		if ( p == 0 ) {
			if (sedebug>3)
			    printf("se: no mbuf clusters\n");
			m_freem(m);
			return(-1);	/* return with rp-> as it was */
		} else {
			/* 
			 * When using cluster sized mbufs, force them
			 * to start on an odd longword boundary. This is
			 * because NFS expects to access the data portion
			 * starting on an even longword boundary.  So the
			 * 14 byte ethernet header starts at byte 2 and ends
			 * at byte 15, while the data region starts at byte
			 * 16, instead of odd longword byte 14.
			 */
			m->m_len = (CLBYTES - 2);
			m->m_off += 2;
		}
		*mp = m;
		dp = svtophy(mtod(m, int));
		rp->se_buf_len = -(m->m_len);
		rp->se_addr_lo = (short)dp & 0xffff;
		rp->se_addr_hi = (short)(((int)dp >> 16) & 0xff);
	}
	return(0);
}

/*
 * Pull the trailer info up to the front of the chain.
 */
struct mbuf *
seget(m, mtp, off)
register struct mbuf 	*m,	/* Pointer to first mbuf in the chain 	*/
			*mtp;	/* Pointer to first mbuf in trailer	*/
register 		off;	/* Offset to trailer in trailer mbuf	*/
{
	register struct mbuf *mp, *mp0;
	register char *cp;

	/*
	 * The trailer consists of 2 words followed by an IP header.
	 * The rest of the code assumes that the trailer is enclosed in
	 * a single mbuf, so must copy trailer into same.
	 */
	MGET(mp, M_DONTWAIT, MT_DATA);
	if ( mp == NULL ){
		m_freem( m );
		return 0;
	}
	cp = mtod(mp, caddr_t);
	/*
	 * Copy the portion of the trailer in this mbuf to the new
	 * mbuf and adjust the length. This mbuf now becomes the last
	 * in the packet.
	 */
	bcopy(mtod(mtp, caddr_t)+off, cp, mtp->m_len - off);
	mp->m_len = mtp->m_len - off;
	mtp->m_len = off;
	mp0 = mtp;			/* Save so we can break the chain */
	/*
	 * The remainder of the trailer is in the next mbuf.
	 */
	mtp = mtp->m_next;
	if ( mtp ) {
		cp += mp->m_len;
		if ( mtp->m_len + mp->m_len > MLEN || mtp->m_next ) {
			/*
			 * Can't fit trailer in an mbuf. Must be insane.
			 */
			m_freem( m );
			m_freem( mp );
			return(0);
		}
		/*
		 * Pull the data up, break the chain and release the old
		 * trailer.
		 */
		bcopy(mtod(mtp, caddr_t), cp, mtp->m_len);
		mp->m_len += mtp->m_len;
		mp0->m_next = NULL;	/* Break the chain */
		m_freem( mtp );
	}

	/*
	 * Move the new trailer to the head of the packet and remove the 
	 * ethernet header.
	 */
	mp->m_next = m;
	m->m_off += sizeof (struct ether_header);
	m->m_len -= sizeof (struct ether_header);
	return( mp );
}

/*
 * Pass a packet to the higher levels.
 * We deal with the trailer protocol here.
 */
seread(sc, m, len)
	register struct se_softc *sc;
	register struct mbuf *m;
	register int len;
{
	register struct mbuf *mp;
	register off;
	register struct ether_header *eh;
	int resid, index;
	struct protosw *pr;
	struct ifqueue *inq;

	/*
	 * Deal with trailer protocol: if type is INET trailer
	 * get true type from first 16-bit word past data.
	 * Remember that type was trailer by setting off.
	 */

	resid = 0;
	eh = mtod(m, struct ether_header *);
	eh->ether_type = ntohs((u_short)eh->ether_type);

	if (eh->ether_type >= ETHERTYPE_TRAIL &&
	    eh->ether_type < ETHERTYPE_TRAIL+ETHERTYPE_NTRAILER) {
		off = ((eh->ether_type - ETHERTYPE_TRAIL) * 512) + sizeof(struct ether_header);

		if (off >= ETHERMTU) {
			m_freem(m);
			return;
		}
		for (index=0, mp=m ; mp && (index + mp->m_len) <= off ;) {
			index += mp->m_len;
			mp = mp->m_next;
		}
		if ( mp ) {
			eh->ether_type = ntohs(*(u_short *)( mtod(mp, caddr_t) + (off-index)));
			resid = ntohs(*(u_short *)( mtod(mp, caddr_t) + (off-index+2)));
		} else {
			m_freem(m);
			return;
		}
		 
		if (off + resid > len) {
			m_freem(m);
			return;
		}
		
		len = off + resid;
	} else
		off = 0;

	if (len == 0) {
		m_freem(m);
		return;
	}

	/*
	 * Pull packet off interface.  Off is nonzero if packet
	 * has trailing header; seget will then force this header
	 * information to be at the front, but we still have to drop
	 * the type and length words which are at the front of any trailer data.
	 */
	if ( off ) {
		if ((m = seget(m, mp, off-index)) == 0) {
			return;
		}
		m->m_off += 2 * sizeof (u_short);
		m->m_len -= 2 * sizeof (u_short);
	} else {
		/*
		 * skip past the ether_header,
		 * adjusting length.
		 */
		m->m_off += sizeof( struct ether_header );
		m->m_len -= sizeof( struct ether_header );
	}

	/*
	 * Accumulate statistics for DECnet
	 */
	if ((sc->ctrblk.est_bytercvd + len) > sc->ctrblk.est_bytercvd)
		sc->ctrblk.est_bytercvd += (len + sizeof(struct ether_header));
	if (sc->ctrblk.est_blokrcvd != (unsigned)0xffffffff)
		sc->ctrblk.est_blokrcvd++;

#ifdef SEDEBUG
	if (seshowrecv) {
		cprintf("%d\t%04x\t%02x.%02x.%02x.%02x.%02x.%02x",
			len, eh->ether_type,
			eh->ether_shost[0], eh->ether_shost[1],
			eh->ether_shost[2], eh->ether_shost[3],
			eh->ether_shost[4], eh->ether_shost[5]);
	}
#endif

	switch (eh->ether_type) {
#ifdef INET
	case ETHERTYPE_IP:
#ifdef SEDEBUG
		if (seshowrecv)
			cprintf("  \n");
#endif
		if (nINET==0) {
			m_freem(m);
			return;
		}
		schednetisr(NETISR_IP);
		inq = &ipintrq;
		break;

	case ETHERTYPE_ARP:
#ifdef SEDEBUG
		if (seshowrecv)
			cprintf("  \n");
#endif
		if (nETHER==0) {
			m_freem(m);
			return;
		}
		arpinput(&sc->is_ac, m);
		return;
#endif
	default:
		/*
		 * See if other protocol families defined
		 * and call protocol specific routines.
		 * If no other protocols defined then dump message.
		 */
		if ((pr=iftype_to_proto(eh->ether_type)) && pr->pr_ifinput)  {
#ifdef SEDEBUG
			if (seshowrecv)
				cprintf("=");
#endif
			if ((m = (struct mbuf *)(*pr->pr_ifinput)(m, &sc->is_if, &inq, eh)) == 0) {
#ifdef SEDEBUG
				if (seshowrecv)
					cprintf("D\n");
#endif
				return;
			}
#ifdef SEDEBUG
			if (seshowrecv)
				cprintf("P\n");
#endif
		} else {
			if (sedebug>4) {
		printf("?-family: %04x, from %02x.%02x.%02x.%02x.%02x.%02x\n",
			eh->ether_type,
			eh->ether_shost[0], eh->ether_shost[1],
			eh->ether_shost[2], eh->ether_shost[3],
			eh->ether_shost[4], eh->ether_shost[5]);
			}
			if (sc->ctrblk.est_unrecog != 0xffff)
				sc->ctrblk.est_unrecog++;
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
 * Pull data from later mbufs up to earlier mbufs in
 * the transmit ring.
 *
 * This is done because the mbuf chain to be transmitted
 * might actually have data segments smaller than 64 bytes
 * in the middle of a chain, while the Lance expects a
 * minimum buffer size of 64 bytes in order to have time
 * to chain to the next buffer.  Cluster sized mbufs are
 * left as is.  Basically, this routine guarantees a 112
 * byte minimum buffer size, except for the last buffer,
 * which gets padded later.
 *
 * Example:
 * an input chain of mbufs with data sizes of:
 *	50-50-76-512-1-60
 *
 * is transformed into:
 *	112-112-112-448-60
 */
struct mbuf *
se_tpullup(m0)
struct mbuf *m0;
{
	register struct mbuf *m = m0;
	register struct mbuf *mp;
	register struct mbuf *head;
	register struct mbuf *prev;
	register int need = 0;
	register int next = 0;

#ifdef SEDEBUG
	if (seshowxmit>1) {
	    do {
		cprintf("%d ", m->m_len);
	    } while (m=m->m_next);
	    cprintf("\b.\n");
	    m = m0;/* reset */
	}
#endif


	prev=0;		/* pointer to previous mbuf, usually (m-1) */
	head=m;		/* pointer to head */
    	for (;;) {
		/* Is current mbuf full? */
		if (m->m_len < MLEN) {
		    if (m->m_next == 0)
			break;	/* done */
		} else if (m->m_len == MLEN) {
		    /* full mbuf, continue */
		    prev = m;	/* save old mbuf */
		    m = m->m_next;
		    if (m == 0)
			break;	/* done */
		    continue;
		} else {
		    /*
		     * found a big (>MLEN) mbuf, skip it and keep
		     * going, unless it is the last one in the chain.
		     */
		    if (m->m_next == 0)
			break;	/* done */
		    if (prev == 0)
		        head = m;
		    else
		        prev = m;
		    m = m->m_next;
		    continue;
		}
		/* how many bytes do we need? */
		need = (MLEN - m->m_len);
		/* how many bytes are in next packet? */
		next = (m->m_next)->m_len;
		if (next <= 0) {
		    m_freem(m->m_next);
		    m->m_next = NULL;	/* terminate chain */
		    break;		/* and quit */
		}
		/* pull up as many bytes as possible from next mbuf */
		if (next > need) {
			mp = m_pullup(m, MLEN);
			if ( !mp ) {
			    if (sedebug) printf("se: m_pullup1 failed\n");
			    head = NULL;	/* return error */
			    break;
			}
		} else {	/* next <= need, grab everything and go again */
			int i;
			i = next + m->m_len;
			mp = m_pullup(m, i);
			if ( !mp ) {
			    if (sedebug) printf("se: m_pullup2 failed\n");
			    head = NULL;	/* return error */
			    break;
			}
		}
		if (prev == 0)	/* first mbuf in chain */
		    head = mp;
		else
		    prev->m_next = mp;
		m = mp;
		/*
		 * If what we got was less than what
		 * we needed, don't advance m pointer,
		 * since we still need to pull up data.
		 */
		if (next < need)
		    continue;
		prev = m;
		m = m->m_next;
		if (m == 0)
		    break;
	}
	return(head);
}
#endif
