/*
 * if.h
 */

/* static	char	*sccsid = "@(#)if.h	1.9	(ULTRIX)	4/18/86"; */

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
 *									*
 *			Modification History				*
 *									*
 *  001 - Larry Cohen 3/6/85  - add LOOPBACK flag to indicate that 	*
 *		interface is in loopback mode.				*
 *									*
 *	Larry Cohen  -	09/16/85					*
 * 		Add 43bsd alpha tape changes for subnet routing		*
 *									*
 *	Ursula Sinkewic - 03/05/86					*
 *		Added bscintrq for BISYNC 2780/3780			*
 *									*
 *      Ed Ferris - 4/18/86						*
 * 		Add point to point state info for DECnet.		*
 ***********************************************************************/

/*
 * Structures defining a network interface, providing a packet
 * transport mechanism (ala level 0 of the PUP protocols).
 *
 * Each interface accepts output datagrams of a specified maximum
 * length, and provides higher level routines with input datagrams
 * received from its medium.
 *
 * Output occurs when the routine if_output is called, with three parameters:
 *	(*ifp->if_output)(ifp, m, dst)
 * Here m is the mbuf chain to be sent and dst is the destination address.
 * The output routine encapsulates the supplied datagram if necessary,
 * and then transmits it on its medium.
 *
 * On input, each interface unwraps the data received by it, and either
 * places it on the input queue of a internetwork datagram routine
 * and posts the associated software interrupt, or passes the datagram to a raw
 * packet input routine.
 *
 * Routines exist for locating interfaces by their addresses
 * or for locating a interface on a certain network, as well as more general
 * routing and gateway routines maintaining information used to locate
 * interfaces.  These routines live in the files if.c and route.c
 */

/*
 * Structure defining a queue for a network interface.
 *
 * (Would like to call this struct ``if'', but C isn't PL/1.)
 *
 * EVENTUALLY PURGE if_net AND if_host FROM STRUCTURE
 */
struct ifnet {
	char	*if_name;		/* name, e.g. ``en'' or ``lo'' */
	short	if_unit;		/* sub-unit for lower level driver */
	short	if_mtu;			/* maximum transmission unit */
	int	if_net;			/* network number of interface */
	short	if_flags;		/* up/down, broadcast, etc. */
	short	if_timer;		/* time 'til if_watchdog called */
	struct	ifaddr *if_addrlist;	/* linked list of addresses per if */
	struct	sockaddr if_addr;	/* address of interface */
	struct	ifqueue {
		struct	mbuf *ifq_head;
		struct	mbuf *ifq_tail;
		int	ifq_len;
		int	ifq_maxlen;
		int	ifq_drops;
	} if_snd;			/* output queue */
/* procedure handles */
	int	(*if_init)();		/* init routine */
	int	(*if_output)();		/* output routine */
	int	(*if_ioctl)();		/* ioctl routine */
	int	(*if_reset)();		/* bus reset routine */
	int	(*if_watchdog)();	/* timer routine */
/* generic interface statistics */
	int	if_ipackets;		/* packets received on interface */
	int	if_ierrors;		/* input errors on interface */
	int	if_opackets;		/* packets sent on interface */
	int	if_oerrors;		/* output errors on interface */
	int	if_collisions;		/* collisions on csma interfaces */
/* end statistics */
	struct	ifnet *if_next;
};


/*
 * The ifaddr structure contains information about one address
 * of an interface.  They are maintained by the different address families,
 * are allocated and attached when an address is set, and are linked
 * together so all addresses for an interface can be located.
 */
struct ifaddr {
	struct	sockaddr ifa_addr;	/* address of interface */
	union {
		struct	sockaddr ifu_broadaddr;
		struct	sockaddr ifu_dstaddr;
	} ifa_ifu;
#define	ifa_broadaddr	ifa_ifu.ifu_broadaddr	/* broadcast address */
#define	ifa_dstaddr	ifa_ifu.ifu_dstaddr	/* other end of p-to-p link */
	struct	ifnet *ifa_ifp;		/* back-pointer to interface */
	struct	ifaddr *ifa_next;	/* next address for interface */
};


#define	IFF_UP		0x1		/* interface is up */
#define	IFF_BROADCAST	0x2		/* broadcast address valid */
#define	IFF_DEBUG	0x4		/* turn on debugging */
#define	IFF_ROUTE	0x8		/* routing entry installed */
#define	IFF_POINTOPOINT	0x10		/* interface is point-to-point link */
#define	IFF_NOTRAILERS	0x20		/* avoid use of trailers */
#define	IFF_RUNNING	0x40		/* resources allocated */
#define	IFF_NOARP	0x80		/* no address resolution protocol */
#define	IFF_CANTCHANGE	(IFF_BROADCAST | IFF_POINTOPOINT | IFF_RUNNING)
#define	IFF_LOCAL	0x100		/* local network, host part encoded */
#define IFF_DYNPROTO	0x200		/* support dynamic proto dispatching */
#define IFF_LOOPBACK	0x400		/* interface set to loopback - LSC001*/
#define IFF_MOP		0x800		/* device in MOP mode */
/*
 * Output queues (ifp->if_snd) and internetwork datagram level (pup level 1)
 * input routines have queues of messages stored on ifqueue structures
 * (defined above).  Entries are added to and deleted from these structures
 * by these macros, which should be called with ipl raised to splimp().
 */
#define	IF_QFULL(ifq)		((ifq)->ifq_len >= (ifq)->ifq_maxlen)
#define	IF_DROP(ifq)		((ifq)->ifq_drops++)
#define	IF_ENQUEUE(ifq, m) { \
	(m)->m_act = 0; \
	if ((ifq)->ifq_tail == 0) \
		(ifq)->ifq_head = m; \
	else \
		(ifq)->ifq_tail->m_act = m; \
	(ifq)->ifq_tail = m; \
	(ifq)->ifq_len++; \
}
#define	IF_PREPEND(ifq, m) { \
	(m)->m_act = (ifq)->ifq_head; \
	if ((ifq)->ifq_tail == 0) \
		(ifq)->ifq_tail = (m); \
	(ifq)->ifq_head = (m); \
	(ifq)->ifq_len++; \
}
#define	IF_DEQUEUE(ifq, m) { \
	(m) = (ifq)->ifq_head; \
	if (m) { \
		if (((ifq)->ifq_head = (m)->m_act) == 0) \
			(ifq)->ifq_tail = 0; \
		(m)->m_act = 0; \
		(ifq)->ifq_len--; \
	} \
}

#define	IFQ_MAXLEN	50
#define	IFNET_SLOWHZ	1		/* granularity is 1 second */

/*
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with ifr_name.  The
 * remainder may be interface specific.
 */
struct	ifreq {
#define	IFNAMSIZ	16
	char	ifr_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	union {
		struct	sockaddr ifru_addr;
		struct	sockaddr ifru_dstaddr;
		struct	sockaddr ifru_broadaddr;
		short	ifru_flags;
		caddr_t	ifru_data;
	} ifr_ifru;
#define	ifr_addr	ifr_ifru.ifru_addr	/* address */
#define	ifr_dstaddr	ifr_ifru.ifru_dstaddr	/* other end of p-to-p link */
#define	ifr_broadaddr	ifr_ifru.ifru_broadaddr	/* broadcast address */
#define	ifr_flags	ifr_ifru.ifru_flags	/* flags */
#define	ifr_data	ifr_ifru.ifru_data	/* for use by interface */
};


/*
 * structure used to query de and qe for physical addresses
 */
struct ifdevea {
        char    ifr_name[IFNAMSIZ];             /* if name, e.g. "en0" */
        u_char default_pa[6];                   /* default hardware address */
        u_char current_pa[6];                   /* current physical address */
};

/*
 * structure used in SIOCSTATE request to set device state and ownership
 */
struct ifstate {
        char    ifr_name[IFNAMSIZ];             /* if name, e.g. "en0" */
	u_short	if_family;			/* current family ownership */
	u_short	if_next_family;			/* next family ownership */
	u_short	if_mode:3,			/* mode of device */
		if_ustate:1,			/* user requested state */
		if_nomuxhdr:1,			/* if set, omit mux header */
		if_dstate:4,			/* current state of device */
		if_xferctl:1,			/* xfer control to nxt family */
		if_rdstate:1,			/* read current state */
		if_wrstate:1,			/* set current state */
		if_reserved:4;
};
#define	IFS_USROFF	0x0	/* user request to stop device */
#define	IFS_USRON	0x1	/* user request to start device */
#define	IFS_DDCMPFDX	0x0	/* operate in DDCMP full duplex */
#define	IFS_MOP		0x1	/* operate in MOP */
#define	IFS_DDCMPHDXP	0x2	/* operate in DDCMP half duplex, primary */
#define	IFS_DDCMPHDXS	0x3	/* operate in DDCMP half duplex, secondary */
#define IFS_NOMUXHDR	0x1	/* do not multiplex on point to point line */
#define IFS_MUXHDR	0x0	/* multiplex on point to point line */
#define IFS_HALTED	0x0	/* device state = halted */
#define IFS_STARTING	0x1	/* device state = starting */
#define IFS_RUNNING	0x3	/* device state = running */
#define IFS_HALTING	0x4	/* device state = halting */
#define IFS_OWNREQ	0x5	/* device state = ownership requested */
#define IFS_OWNREL	0x6	/* device state = ownership released */
#define IFS_ENTEREDMOP	0x7	/* device state = mop */
#define IFS_XFERCTL	0x1	/* xfer control to next family */
#define IFS_RDSTATE	0x1	/* read device state */
#define IFS_WRSTATE	0x1	/* set device state */

/*
 * Structure used in SIOCGIFCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible).
 */
struct	ifconf {
	int	ifc_len;		/* size of associated buffer */
	union {
		caddr_t	ifcu_buf;
		struct	ifreq *ifcu_req;
	} ifc_ifcu;
#define	ifc_buf	ifc_ifcu.ifcu_buf	/* buffer address */
#define	ifc_req	ifc_ifcu.ifcu_req	/* array of structures returned */
};

/*
 * ARP ioctl request
 */
struct arpreq {
	struct sockaddr	arp_pa;		/* protocol address */
	struct sockaddr	arp_ha;		/* hardware address */
	int	arp_flags;		/* flags */
};
/*  arp_flags and at_flags field values */
#define	ATF_INUSE	1	/* entry in use */
#define ATF_COM		2	/* completed entry (enaddr valid) */
#define	ATF_PERM	4	/* permanent entry */
#define	ATF_PUBL	8	/* publish entry (respond for other host) */

/*
 * interface statistics structures
 */
struct estat {				/* Ethernet interface statistics */
	u_short	est_seconds;		/* seconds since last zeroed */
	u_int	est_bytercvd;		/* bytes received */
	u_int	est_bytesent;		/* bytes sent */
	u_int	est_blokrcvd;		/* data blocks received */
	u_int	est_bloksent;		/* data blocks sent */
	u_int	est_mbytercvd;		/* multicast bytes received */
	u_int	est_mblokrcvd;		/* multicast blocks received */
	u_int	est_deferred;		/* blocks sent, initially deferred */
	u_int	est_single;		/* blocks sent, single collision */
	u_int	est_multiple;		/* blocks sent, multiple collisions */
	u_short	est_sendfail_bm;	/*	0 - Excessive collisions */
					/*	1 - Carrier check failed */
					/*	2 - Short circuit */
					/*	3 - Open circuit */
					/*	4 - Frame too long */
					/*	5 - Remote failure to defer */
	u_short	est_sendfail;		/* send failures: (bit map)*/
	u_short	est_collis;		/* Collision detect check failure */
	u_short	est_recvfail_bm;	/*	0 - Block check error */
					/*	1 - Framing error */
					/*	2 - Frame too long */
	u_short	est_recvfail;		/* receive failure: (bit map) */
	u_short	est_unrecog;		/* unrecognized frame destination */
	u_short	est_overrun;		/* data overrun */
	u_short	est_sysbuf;		/* system buffer unavailable */
	u_short	est_userbuf;		/* user buffer unavailable */
};

struct dstat {				/* DDCMP pt-to-pt interface statistics */
	u_short	dst_seconds;		/* seconds since last zeroed */
	u_int	dst_bytercvd;		/* bytes received */
	u_int	dst_bytesent;		/* bytes sent */
	u_int	dst_blockrcvd;		/* data blocks received */
	u_int	dst_blocksent;		/* data blocks sent */
	u_short	dst_inbound_bm;		/*	0 - NAKs sent, header crc */
					/*	1 - NAKs sent, data crc */
					/*	2 - NAKs sent, REP response */
	u_char	dst_inbound;		/* data errors inbound: (bit map) */
	u_short	dst_outbound_bm;	/*	0 - NAKs rcvd, header crc */
					/*	1 - NAKs rcvd, data crc */
					/*	2 - NAKs rcvd, REP response */
	u_char	dst_outbound;		/* data errors outbound: (bit map) */
	u_char	dst_remotetmo;		/* remote reply timeouts */
	u_char	dst_localtmo;		/* local reply timeouts */
	u_short	dst_remotebuf_bm;	/*	0 - NAKs rcvd, buffer unavailable */
					/*	1 - NAKs rcvd, buffer too small */
	u_char	dst_remotebuf;		/* remote buffer errors: (bit map) */
	u_short	dst_localbuf_bm;	/*	0 - NAKs sent, buffer unavailable */
					/*	1 - NAKs sent, buffer too small */
	u_char	dst_localbuf;		/* local buffer errors: (bit map) */
	u_char	dst_select;		/* selection intervals elapsed */
	u_short	dst_selecttmo_bm;	/*	0 - No reply to select */
					/*	1 - Incomplete reply to select */
	u_char	dst_selecttmo;		/* selection timeouts: (bit map) */
	u_short	dst_remotesta_bm;	/*	0 - NAKs rcvd, receive overrun */
					/*	1 - NAKs sent, header format */
					/*	2 - Select address errors
					/*	3 - Streaming tributaries */
	u_char	dst_remotesta;		/* remote station errors: (bit map) */
	u_short	dst_localsta_bm;	/*	0 - NAKs sent, receive overrun */
					/*	1 - Receive overrun, NAK not sent */
					/*	2 - Transmit underruns */
					/*	3 - NAKs rcvd, header format */
	u_char	dst_localsta;		/* local station errors: (bit map) */
};

/*
 * interface counter ioctl request
 */
struct ctrreq {
	char	ctr_name[IFNAMSIZ];	/* if name */
	char	ctr_type;		/* type of interface */
	union {
		struct estat ctrc_ether;/* ethernet counters */
		struct dstat ctrc_ddcmp;/* DDCMP pt-to-pt counters */
	} ctr_ctrs;
};
#define CTR_ETHER 0			/* Ethernet interface */
#define CTR_DDCMP 1			/* DDCMP pt-to-pt interface */
#define ctr_ether ctr_ctrs.ctrc_ether
#define ctr_ddcmp ctr_ctrs.ctrc_ddcmp
#define CTR_HDRCRC	0		/* header crc bit index */
#define CTR_DATCRC	1		/* data crc bit index */
#define CTR_BUFUNAVAIL	0		/* buffer unavailable bit index */

#ifdef KERNEL
struct	ifnet *ifnet;
struct	ifnet *if_ifwithaddr(), *if_ifwithnet(), *if_ifwithaf();
struct	ifnet *if_ifonnetof();
#ifdef INET
struct	ifqueue	ipintrq;		/* ip packet input queue */
#endif
#ifdef BSC
struct	ifqueue	bscintrq;		/* BISYNC packet input queue */
#endif
struct	ifqueue rawintrq;		/* raw packet input queue */
extern	struct	in_addr if_makeaddr();
extern  struct	ifaddr *ifa_ifwithaddr(), *ifa_ifwithnet(), *ifa_ifwithaf();
#endif
