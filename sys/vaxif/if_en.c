#ifndef lint
static char *sccsid = "@(#)if_en.c	1.9	ULTRIX	10/3/86";
#endif lint

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
 *
 * 13-Jun-86   -- jaw 	fix to uba reset and drivers.
 *									*
 * 18-mar-86  -- jaw     br/cvec changed to NOT use registers.
 *
 *	Larry Cohen  -	09/16/85					*
 * 		Add 43bsd alpha tape changes for subnet routing		*
 *									*
 ************************************************************************/

#include "en.h"

#if NEN > 0 || defined(BINARY)

/*
 * Xerox prototype (3 Mb) Ethernet interface driver.
 */

#include "../data/if_en_data.c"

#define	ENMTU	(1024+512)
#define	ENMRU	(1024+512+16)		/* 16 is enough to receive trailer */

int	enprobe(), enattach(), enrint(), enxint(), encollide();
u_short enstd[] = { 0 };
struct	uba_driver endriver =
	{ enprobe, 0, enattach, 0, enstd, "en", eninfo };
#define	ENUNIT(x)	minor(x)

int	eninit(),enoutput(),enreset(),enioctl();

#ifdef notdef
/*
 * If you need to byte swap IP's in the system, define
 * this and do a SIOCSIFFLAGS at boot time.
 */
#define	ENF_SWABIPS	0x100
#endif

/*
 * Do output DMA to determine interface presence and
 * interrupt vector.  DMA is too short to disturb other hosts.
 */
enprobe(reg)
	caddr_t reg;
{
	register struct endevice *addr = (struct endevice *)reg;

#ifdef lint
	enrint(0); enxint(0); encollide(0);
#endif
	addr->en_istat = 0;
	addr->en_owc = -1;
	addr->en_oba = 0;
	addr->en_ostat = EN_IEN|EN_GO;
	DELAY(100000);
	addr->en_ostat = 0;
	return (1);
}

/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.
 */
enattach(ui)
	struct uba_device *ui;
{
	register struct en_softc *es = &en_softc[ui->ui_unit];

	es->es_if.if_unit = ui->ui_unit;
	es->es_if.if_name = "en";
	es->es_if.if_mtu = ENMTU;
	es->es_if.if_init = eninit;
	es->es_if.if_output = enoutput;
	es->es_if.if_ioctl = enioctl;
	es->es_if.if_reset = enreset;
	es->es_ifuba.ifu_flags = UBA_NEEDBDP | UBA_NEED16 | UBA_CANTWAIT;
	/* don't chew up 750 bdp's */
	if ((ui->ui_hd->uba_type & (UBABUA|UBA750)) && ui->ui_unit > 0)
		es->es_ifuba.ifu_flags &= ~UBA_NEEDBDP;
	if_attach(&es->es_if);
}

/*
 * Reset of interface after UNIBUS reset.
 * If interface is on specified uba, reset its state.
 */
enreset(unit, uban)
	int unit, uban;
{
	register struct uba_device *ui;

	if (unit >= nNEN || (ui = eninfo[unit]) == 0 || ui->ui_alive == 0 ||
	    ui->ui_ubanum != uban)
		return;
	printf(" en%d", unit);
	eninit(unit);
}

/*
 * Initialization of interface; clear recorded pending
 * operations, and reinitialize UNIBUS usage.
 */
eninit(unit)
	int unit;
{
	register struct en_softc *es = &en_softc[unit];
	register struct uba_device *ui = eninfo[unit];
	register struct endevice *addr;
	int s;

	if (es->es_if.if_addrlist == (struct ifaddr *)0)
		return;
	if (if_ubainit(&es->es_ifuba, ui->ui_ubanum,
	    sizeof (struct en_header), (int)btoc(ENMRU)) == 0) { 
		printf("en%d: can't initialize\n", unit);
		es->es_if.if_flags &= ~IFF_UP;
		return;
	}
	addr = (struct endevice *)ui->ui_addr;
	addr->en_istat = addr->en_ostat = 0;

	/*
	 * Hang a receive and start any
	 * pending writes by faking a transmit complete.
	 */
	s = splimp();
	addr->en_iba = es->es_ifuba.ifu_r.ifrw_info;
	addr->en_iwc = -(sizeof (struct en_header) + ENMRU) >> 1;
	addr->en_istat = EN_IEN|EN_GO;
	es->es_oactive = 1;
	es->es_if.if_flags |= IFF_UP|IFF_RUNNING;
	enxint(unit);
	splx(s);
}

int	enalldelay = 0;
int	enlastdel = 50;
int	enlastmask = (~0) << 5;

/*
 * Start or restart output on interface.
 * If interface is already active, then this is a retransmit
 * after a collision, and just restuff registers and delay.
 * If interface is not already active, get another datagram
 * to send off of the interface queue, and map it to the interface
 * before starting the output.
 */
enstart(dev)
	dev_t dev;
{
        int unit = ENUNIT(dev);
	struct uba_device *ui = eninfo[unit];
	register struct en_softc *es = &en_softc[unit];
	register struct endevice *addr;
	struct mbuf *m;
	int dest;

	if (es->es_oactive)
		goto restart;

	/*
	 * Not already active: dequeue another request
	 * and map it to the UNIBUS.  If no more requests,
	 * just return.
	 */
	IF_DEQUEUE(&es->es_if.if_snd, m);
	if (m == 0) {
		es->es_oactive = 0;
		return;
	}
	dest = mtod(m, struct en_header *)->en_dhost;
	es->es_olen = if_wubaput(&es->es_ifuba, m);
#ifdef ENF_SWABIPS
	/*
	 * The Xerox interface does word at a time DMA, so
	 * someone must do byte swapping of user data if high
	 * and low ender machines are to communicate.  It doesn't
	 * belong here, but certain people depend on it, so...
	 *
	 * Should swab everybody, but this is a kludge anyway.
	 */
	if (es->es_if.if_flags & ENF_SWABIPS) {
		register struct en_header *en;

		en = (struct en_header *)es->es_ifuba.ifu_w.ifrw_addr;
		if (en->en_type == ENTYPE_IP)
			enswab((caddr_t)(en + 1), (caddr_t)(en + 1),
			    es->es_olen - sizeof (struct en_header) + 1);
	}
#endif

	/*
	 * Ethernet cannot take back-to-back packets (no
	 * buffering in interface.  To help avoid overrunning
	 * receivers, enforce a small delay (about 1ms) in interface:
	 *	* between all packets when enalldelay
	 *	* whenever last packet was broadcast
	 *	* whenever this packet is to same host as last packet
	 */
	if (enalldelay || es->es_lastx == 0 || es->es_lastx == dest) {
		es->es_delay = enlastdel;
		es->es_mask = enlastmask;
	}
	es->es_lastx = dest;

restart:
	/*
	 * Have request mapped to UNIBUS for transmission.
	 * Purge any stale data from this BDP, and start the otput.
	 */
	if (es->es_ifuba.ifu_flags & UBA_NEEDBDP)
		UBAPURGE(es->es_ifuba.ifu_uba, es->es_ifuba.ifu_w.ifrw_bdp,
			es->es_ifuba.ifu_uban);
	addr = (struct endevice *)ui->ui_addr;
	addr->en_oba = (int)es->es_ifuba.ifu_w.ifrw_info;
	addr->en_odelay = es->es_delay;
	addr->en_owc = -((es->es_olen + 1) >> 1);
	addr->en_ostat = EN_IEN|EN_GO;
	es->es_oactive = 1;
}

/*
 * Ethernet interface transmitter interrupt.
 * Start another output if more data to send.
 */
enxint(unit)
	int unit;
{
	register struct uba_device *ui = eninfo[unit];
	register struct en_softc *es = &en_softc[unit];
	register struct endevice *addr = (struct endevice *)ui->ui_addr;

	if (es->es_oactive == 0)
		return;
	if (es->es_mask && (addr->en_ostat&EN_OERROR)) {
		es->es_if.if_oerrors++;
		endocoll(unit);
		return;
	}
	es->es_if.if_opackets++;
	es->es_oactive = 0;
	es->es_delay = 0;
	es->es_mask = ~0;
	if (es->es_ifuba.ifu_xtofree) {
		m_freem(es->es_ifuba.ifu_xtofree);
		es->es_ifuba.ifu_xtofree = 0;
	}
	if (es->es_if.if_snd.ifq_head == 0) {
		es->es_lastx = 256;		/* putatively illegal */
		return;
	}
	enstart(unit);
}

/*
 * Collision on ethernet interface.  Do exponential
 * backoff, and retransmit.  If have backed off all
 * the way print warning diagnostic, and drop packet.
 */
encollide(unit)
	int unit;
{
	struct en_softc *es = &en_softc[unit];

	es->es_if.if_collisions++;
	if (es->es_oactive == 0)
		return;
	endocoll(unit);
}

endocoll(unit)
	int unit;
{
	register struct en_softc *es = &en_softc[unit];

	/*
	 * Es_mask is a 16 bit number with n low zero bits, with
	 * n the number of backoffs.  When es_mask is 0 we have
	 * backed off 16 times, and give up.
	 */
	if (es->es_mask == 0) {
		printf("en%d: send error\n", unit);
		enxint(unit);
		return;
	}
	/*
	 * Another backoff.  Restart with delay based on n low bits
	 * of the interval timer.
	 */
	es->es_mask <<= 1;
	es->es_delay = mfpr(ICR) &~ es->es_mask;
	enstart(unit);
}

#ifdef notdef
struct	sockproto enproto = { AF_ETHERLINK };
struct	sockaddr_en endst = { AF_ETHERLINK };
struct	sockaddr_en ensrc = { AF_ETHERLINK };
#endif
/*
 * Ethernet interface receiver interrupt.
 * If input error just drop packet.
 * Otherwise purge input buffered data path and examine 
 * packet to determine type.  If can't determine length
 * from type, then have to drop packet.  Othewise decapsulate
 * packet based on type and pass to type specific higher-level
 * input routine.
 */
enrint(unit)
	int unit;
{
	register struct en_softc *es = &en_softc[unit];
	struct endevice *addr = (struct endevice *)eninfo[unit]->ui_addr;
	register struct en_header *en;
    	struct mbuf *m;
	int len; short resid;
	register struct ifqueue *inq;
	int off;

	es->es_if.if_ipackets++;

	/*
	 * Purge BDP; drop if input error indicated.
	 */
	if (es->es_ifuba.ifu_flags & UBA_NEEDBDP)
		UBAPURGE(es->es_ifuba.ifu_uba, es->es_ifuba.ifu_r.ifrw_bdp,
			es->es_ifuba.ifu_uban);
	if (addr->en_istat&EN_IERROR) {
		es->es_if.if_ierrors++;
		goto setup;
	}

	/*
	 * Calculate input data length.
	 * Get pointer to ethernet header (in input buffer).
	 * Deal with trailer protocol: if type is trailer
	 * get true type from first 16-bit word past data.
	 * Remember that type was trailer by setting off.
	 */
	resid = addr->en_iwc;
	if (resid)
		resid |= 0176000;
	len = (((sizeof (struct en_header) + ENMRU) >> 1) + resid) << 1;
	len -= sizeof (struct en_header);
	if (len > ENMRU)
		goto setup;			/* sanity */
	en = (struct en_header *)(es->es_ifuba.ifu_r.ifrw_addr);
	en->en_type = ntohs(en->en_type);
#define	endataaddr(en, off, type)	((type)(((caddr_t)((en)+1)+(off))))
	if (en->en_type >= ENTYPE_TRAIL &&
	    en->en_type < ENTYPE_TRAIL+ENTYPE_NTRAILER) {
		off = (en->en_type - ENTYPE_TRAIL) * 512;
		if (off > ENMTU)
			goto setup;		/* sanity */
		en->en_type = ntohs(*endataaddr(en, off, u_short *));
		resid = ntohs(*(endataaddr(en, off+2, u_short *)));
		if (off + resid > len)
			goto setup;		/* sanity */
		len = off + resid;
	} else
		off = 0;
	if (len == 0)
		goto setup;
#ifdef ENF_SWABIPS
	if (es->es_if.if_flags & ENF_SWABIPS && en->en_type == ENTYPE_IP)
		enswab((caddr_t)(en + 1), (caddr_t)(en + 1), len);
#endif
	/*
	 * Pull packet off interface.  Off is nonzero if packet
	 * has trailing header; if_rubaget will then force this header
	 * information to be at the front, but we still have to drop
	 * the type and length which are at the front of any trailer data.
	 */
	m = if_rubaget(&es->es_ifuba, len, off);
	if (m == 0)
		goto setup;
	if (off) {
		m->m_off += 2 * sizeof (u_short);
		m->m_len -= 2 * sizeof (u_short);
	}
	switch (en->en_type) {

#ifdef INET
	case ENTYPE_IP:
		schednetisr(NETISR_IP);
		inq = &ipintrq;
		break;
#endif
#ifdef PUP
	case ENTYPE_PUP:
		rpup_input(m);
		goto setup;
#endif
	default:
#ifdef notdef
		enproto.sp_protocol = en->en_type;
		endst.sen_host = en->en_dhost;
		endst.sen_net = ensrc.sen_net = es->es_if.if_net;
		ensrc.sen_host = en->en_shost;
		raw_input(m, &enproto,
		    (struct sockaddr *)&ensrc, (struct sockaddr *)&endst);
#else
		m_freem(m);
#endif
		goto setup;
	}

	if (IF_QFULL(inq)) {
		IF_DROP(inq);
		m_freem(m);
	} else
		IF_ENQUEUE(inq, m);

setup:
	/*
	 * Reset for next packet.
	 */
	addr->en_iba = es->es_ifuba.ifu_r.ifrw_info;
	addr->en_iwc = -(sizeof (struct en_header) + ENMRU) >> 1;
	addr->en_istat = EN_IEN|EN_GO;
}

/*
 * Ethernet output routine.
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 */
enoutput(ifp, m0, dst)
	struct ifnet *ifp;
	struct mbuf *m0;
	struct sockaddr *dst;
{
	int type, dest, s, error;
	register struct mbuf *m = m0;
	register struct en_header *en;
	register int off;

	switch (dst->sa_family) {

#ifdef INET
	case AF_INET:
		{
		struct in_addr in;
		in = ((struct sockaddr_in *)dst)->sin_addr;
		if (in_broadcast(in))
			dest = EN_BROADCAST;
		else
			dest = in_lnaof(in);
		}
		if (dest >= 0x100) {
			error = EPERM;		/* ??? */
			goto bad;
		}
		off = ntohs((u_short)mtod(m, struct ip *)->ip_len) - m->m_len;
		/* need per host negotiation */
		if ((ifp->if_flags & IFF_NOTRAILERS) == 0)
		if (off > 0 && (off & 0x1ff) == 0 &&
		    m->m_off >= MMINOFF + 2 * sizeof (u_short)) {
			type = ENTYPE_TRAIL + (off>>9);
			m->m_off -= 2 * sizeof (u_short);
			m->m_len += 2 * sizeof (u_short);
			*mtod(m, u_short *) = htons((u_short)ENTYPE_IP);
			*(mtod(m, u_short *) + 1) = ntohs((u_short)m->m_len);
			goto gottrailertype;
		}
		type = ENTYPE_IP;
		off = 0;
		goto gottype;
#endif
#ifdef PUP
	case AF_PUP:
		dest = ((struct sockaddr_pup *)dst)->spup_host;
		type = ENTYPE_PUP;
		off = 0;
		goto gottype;
#endif

#ifdef notdef
	case AF_ETHERLINK:
		goto gotheader;
#endif

	default:
		printf("en%d: can't handle af%d\n", ifp->if_unit,
			dst->sa_family);
		error = EAFNOSUPPORT;
		goto bad;
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
	    MMINOFF + sizeof (struct en_header) > m->m_off) {
		m = m_get(M_DONTWAIT, MT_HEADER);
		if (m == 0) {
			error = ENOBUFS;
			goto bad;
		}
		m->m_next = m0;
		m->m_off = MMINOFF;
		m->m_len = sizeof (struct en_header);
	} else {
		m->m_off -= sizeof (struct en_header);
		m->m_len += sizeof (struct en_header);
	}


	en = mtod(m, struct en_header *);
	/* add en_shost later */
	en->en_dhost = dest;
	en->en_type = htons((u_short)type);


gotheader:
	/*
	 * Queue message on interface, and start output if interface
	 * not yet active.
	 */
	s = splimp();
	if (IF_QFULL(&ifp->if_snd)) {
		IF_DROP(&ifp->if_snd);
		error = ENOBUFS;
		goto qfull;
	}
	IF_ENQUEUE(&ifp->if_snd, m);
	if (en_softc[ifp->if_unit].es_oactive == 0)
		enstart(ifp->if_unit);
	splx(s);
	return (0);
qfull:
	m0 = m;
	splx(s);
bad:
	m_freem(m0);
	return (error);
}

/*
 * Process an ioctl request.
 */
enioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	struct sockaddr_in *sin = (struct sockaddr_in *)data;
	struct ifaddr *ifa = (struct ifaddr *) data;
	struct endevice *enaddr;
	register struct en_softc *es = ((struct en_softc *)ifp);
	int s = splimp(), error = 0;

	switch (cmd) {

	case SIOCSIFADDR:
		enaddr = (struct endevice *)eninfo[ifp->if_unit]->ui_addr;
		es->es_host = (~enaddr->en_addr) & 0xff;
		/*
		 * Attempt to check agreement of protocol address
		 * and board address.
		 */
		switch (ifa->ifa_addr.sa_family) {
		case AF_INET:
			if (in_lnaof(IA_SIN(ifa)->sin_addr) != es->es_host)
				return (EADDRNOTAVAIL);
			break;
		}
		ifp->if_flags |= IFF_UP;
		if ((ifp->if_flags & IFF_RUNNING) == 0)
			eninit(ifp->if_unit);
		break;

	default:
		error = EINVAL;
	}
	splx(s);
	return (error);
}



#ifdef ENF_SWABIPS
/*
 * Swab bytes
 * Jeffrey Mogul, Stanford
 */
enswab(from, to, n)
	register caddr_t *from, *to;
	register int n;
{
	register unsigned long temp;
	
	n >>= 1; n++;
#define	STEP	temp = *from++,*to++ = *from++,*to++ = temp
	/* round to multiple of 8 */
	while ((--n) & 07)
		STEP;
	n >>= 3;
	while (--n >= 0) {
		STEP; STEP; STEP; STEP;
		STEP; STEP; STEP; STEP;
	}
}
#endif
#endif
