/*	@(#)dli_var.h	1.7	(ULTRIX)	9/11/86	*/

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

 /*
 * 2.00 18-Apr-1986
 *		DECnet-Ultrix	V2.0
 *
 * Added sysid and point-to-point support
 *
 */

/* Data Structures and Literals applicable to DLI */

extern struct domain dlidomain;

/*
 * Socket queue literals
 */
#define DLI_SENDSPACE 1024*2
#define DLI_RECVSPACE 1024*4


/*
 * DLI socket literals
 */
#define DLPROTO_DLI	0		/* protocol */


/*
 * DLI i/o control flag literals
 */
#define DLI_EXCLUSIVE	001		/* exclusive use of protocol */
#define DLI_NORMAL	002		/* prot/mcast filtering */
#define DLI_DEFAULT	004		/* unmatched packets go here */

/*
 * DLI literals corresponding to the following address structures.
 */
#define DLI_ETHERNET	0		/* sockaddr_edl format */
#define DLI_POINTOPOINT	1		/* sockaddr_pdl format */
#define	DLI_802		2           /* sockaddr_802 format */
#define DLI_EADDRSIZE	6		/* Ethernet address size */
#define DLI_DEVSIZE	16		/* device name size */

/*
*  802 support
*/

#define	MAX_BROADCSTDEV	4         /* max # una's, delua's, qna's, etc */

#define	ENOTFND		-1
#define	USER		(u_char)0     /* user supplied service */
#define	TYPE1		(u_char)1     /* class 1 service */
#define	OSI802		0X5DC         /* max value of 802 len field */
#define SNAP_SAP	(u_char)0xAA
#define	NULL_SAP	(u_char)0x00
#define	GLOBAL_SAP	(u_char)0xFF
#define	XID		(u_char)0xAF     /* xid command w/out poll bit set */
#define	TEST		(u_char)0xE3     /* test command w/out poll bit set */
#define	XID_PCMD	(u_char)0xBF     /* xid command w/poll bit set */
#define	XID_NPCMD	(u_char)0xAF     /* xid command w/out poll bit set */
#define	XID_PRSP	(u_char)0xBF     /* xid response w/poll bit set */
#define	XID_NPRSP	(u_char)0xAF     /* xid response w/out poll bit set */
#define	TEST_PCMD	(u_char)0xF3     /* test command w/poll bit set */
#define	TEST_NPCMD	(u_char)0xE3     /* test command w/out poll bit set */
#define	TEST_PRSP	(u_char)0xF3     /* test response w/poll bit set */
#define	TEST_NPRSP	(u_char)0xE3     /* test response w/out poll bit set */
#define	UI_NPCMD	(u_char)0x03     /* unnumbered info cmd w/out poll bit */
#define	MAX802DATANP	1496         /* maximum user data without proto id */
#define	MAX802DATAP	1493         /* maximum user data with proto id field */
#define	NISAPS		64
#define	NGSAPS		64

struct	osi_802hdr	
{
	u_char	dst[DLI_EADDRSIZE];	/* where packet is going */
	u_char	src[DLI_EADDRSIZE];	/* who packet is for (usually this node)*/
	u_short	len;			/* length of user data plus 802.3 header */
	u_char	dsap;			/* what dst sap the packet is going to */
	u_char	ssap;			/* source individual sap */
	union {				/* control field */
		u_char U_fmt;		/* unnumbered information format */
		u_short I_S_fmt;	/* information/supervisory format */
	}ctl;	
	u_char	osi_pi[5];           	/* 802.3 protocol id */
};

/*
 * address structures used by DLI
 */
struct dli_devid
	
   {
	u_char dli_devname[DLI_DEVSIZE+1];	/* device name */
	u_short dli_devnumber;			/* device number */
   };

struct dli_recv			/* received message prefix */
{
	struct ifnet *rcv_ifp;	/* received interface descriptor */
	union
	{
		struct ether_header rcv_ether; /* ethernet header */
		struct osi_802hdr osi_header;  /* 802 */
	}rcv_hdr;               /* type of data link */
};

struct sockaddr_pdl
{
	struct ifstate dev_cstate;	/* current point to point device state */
	struct ifstate dev_pstate;	/* previous point to point device state */
};

struct sockaddr_edl
{
	u_char	dli_ioctlflg;		/* i/o control flags */
	u_short dli_protype;		/* Ethernet protocol type */
	u_char dli_target[DLI_EADDRSIZE]; /* Ethernet address of target node */
	u_char dli_dest[DLI_EADDRSIZE]; /* Ethernet destination address */
};

struct sockaddr_802			/* 802.3 sockaddr struct */
{
	u_char	ioctl;			/* i/o control flags */
	u_char	svc;			/* service class for this portal */
	struct	osi_802hdr	eh_802;	/* OSI 802 header format */
};

struct sockaddr_dl
{
	u_short dli_family;		/* address family (AF_DLI) */
	struct dli_devid dli_device;	/* id of comm device to use */
	u_char dli_substructype;	/* id to interpret following structure */
	union
	{
		struct sockaddr_edl dli_eaddr; /* Ethernet */
		struct sockaddr_pdl dli_paddr; /* Point to Point */
		struct sockaddr_802 dli_802addr; /* OSI 802 support */
	} choose_addr;
};

#define DLI_MAXLINE	16	/* max number of users */
#define DLI_MAXPKT	1500	/* max number of user bytes per pkt */

struct if_isapt                 /* 802 per device structure */
{
	struct ifnet *ifp;          /* owning device */
	struct socket	*so[NISAPS];		/* socket ptr to isap */
	/* svc bit mask, bit = 0 -> user, bit = 1 -> class 1 */
	u_char svc[NISAPS / 8];     
	u_char gsap [NISAPS][NISAPS / 8 ];		/* gsap is index, bit set is isap */
};

/*
 * DLI socket option literals
 */
#define DLI_STATE	0	/* state option requested */
#define DLI_MULTICAST	1	/* load multicast address(es) */
#define DLI_INTERNALOOP	2	/* change device to/from internal loopback */

#define	DLI_ENAGSAP		4	/* enable an 802.3 group sap */
#define	DLI_DISGSAP		6	/* disable an 802.3 group sap */
#define	DLI_GETGSAP		8	/* get 802.3 group saps that are enabled by user */
#define	DLI_SET802CTL	10	/* change control field format, 802 */

/*
 * DLI state literals
 */
#define DLS_OFF		0	/* turn comm device off */
#define DLS_ON		1	/* turn comm device on */
#define DLS_SLEEP	2	/* process asleep in bind */

/*
 * DLI internal loopback literals
 */
#define DLP_IOFF	0	/* turn internal loopback on */
#define DLP_ION		1	/* turn internal loopback off */

/*
 * DLI socket option structure
 */
#define MCAST_MAXNUM	10	/* allowable number of multicast addresses */
#define MCAST_SIZE	6	/* size of mulicast address */
#define MCAST_ASIZE	MCAST_MAXNUM * MCAST_SIZE	/* size of mulicast address array */

struct dli_sopt
{
	u_char dli_state;	/* state of device */
	u_char dli_iloop;	/* loopback state */
	u_char dli_mcast[MCAST_MAXNUM][MCAST_SIZE]; /* multicast addresses */
};

/*
 * DLI line table structure - one for each user;
 */
struct dli_line
{
	struct sockaddr_dl dli_lineid;	/* link user to target */
	struct dli_sopt	dli_sockopt;	/* user's socket options */
	struct ifnet	*dli_if;	/* device interface to use */
	struct socket	*dli_so;	/* back ptr to owning socket */
};

/*
 * MOP literals used by DLI
 */
#define DLI_LBACK	0x9000		/* MOP loopback protocol type */
#define DLI_LBACK_FWD	0x2		/* MOP loopback forward data function */
#define DLI_LBACK_LOOP	24		/* MOP point to point loop code */
#define DLI_LBACK_LOOPED 26		/* MOP point to point looped code */

/*
 * EVL literals used by DLI
 */
#define DLI_LBEVL_WAIT	60		/* passive loopback timeout - ethernet */
#define DLI_LBEVL_POP	60		/* passive loopback timeout - point to point */
#define DLI_LBEVL_CLASS	0		/* passive loopback class */
#define DLI_LBEVL_TYPE	6		/* passive loopback type */
#define DLI_LBEVL_ETYPE	3		/* passive loopback type */
#define DLI_EVLOP_LBINI	0		/* passive loopback initiated */
#define DLI_EVLOP_LBTER	1		/* passive loopback terminated */
#define DLI_EVLOP_CODE	2		/* operation code */
#define DLI_EVLOP_DESC	0x81		/* describes LBINI/LBTER field to follow */

/*
 * Loopback timer structure.
 */
#define DLI_MAX_LBTIMR	16		/* maximum number of active timers */
struct dli_timers {
	u_short tval;			/* timeout period in units of 500 ms */
	struct ifnet *ifp;		/* interface being used */
	u_char actv_addr[DLI_EADDRSIZE]; /* used to identify active side */
	struct ifstate prev_devstate;	/* previous device state */
};

/*
 * ethernet address structure used as a cast to copy ethernet
 * physical addresses.
 */
struct ether_pa {
	u_char	ether_addr_octet[6];
};

typedef unsigned char field8b;		/* 8-bit message field */
typedef unsigned short field16b;		/* 16-bit message field */

#define PUT8B(p,v) (*p++ = (v))
#define PUT16B(p,v) *(field16b *)p = (v); p += sizeof(field16b)

#define	MAXQNAS	MAX_BROADCSTDEV	
#define	QNA	5
#define	SYSID_CODE 7
#define	MAINTV 1
#define	VER	1
#define	ECO	0
#define	USER_ECO	0
#define	FUNCTIONS 2
#define	HADDR	7
#define	COMDEV	100
#define REQSYSID	5
#define SYSIDPROTO	0x6002
#define	SYSIDMCAST {0xab, 0x00, 0x00, 0x02, 0x00, 0x00}

#define	SYSID_MSGL sizeof(u_short)+\
sizeof(u_char)+\
sizeof(u_char)+\
sizeof(u_short)+\
sizeof(u_short)+\
sizeof(u_short)+\
3*sizeof(u_char)+\
sizeof(u_short)+\
sizeof(u_short)+\
sizeof(u_short)+\
sizeof(u_short)+\
sizeof(u_short)+\
sizeof(struct ether_pa)+\
sizeof(u_short)+\
sizeof(u_short)+\
sizeof(u_char) 

struct dli_sysid_to {                /* sysid time out struct */
	u_short	to;                      /* time as count of ms */
	u_short	tr;                      /* save for resets */
	struct	ifnet	*ifp;            /* device to broadcast sysid on */
	struct	ifdevea	dev;             /* name and address of device */
} ;

