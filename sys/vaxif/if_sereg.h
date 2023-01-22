/*
 *	@(#)if_sereg.h	1.3	(ULTRIX)	9/8/86
 */

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

/* ---------------------------------------------------------------------
 *  Modification History:
 *
 *   5-Sep-86  jsd (John Dustin)
 *	Removed obsolete code, cleaned up comments, etc.
 *
 *   5-Aug-86  jsd (John Dustin)
 *	Major changes for real VAXstar ethernet driver.
 *
 *  18-Jun-86  jsd (John Dustin)
 *	Created this header file for the VAXstar Ethernet driver.
 *	Derived from if_qereg.h.
 *
 * ---------------------------------------------------------------------
 */

/*
 * Digital VAXstar NI Adapter
 */

/*
 * Network INIT Block
 */
struct se_initb {
	u_short se_mode;		/* NIB_MODE mode word 		*/
	u_short	se_sta_addr[3]; 	/* NIB_PADR station address 	*/
	u_short	se_multi_mask[4];	/* NIB_LADRF Multicast addr mask*/
	u_short	se_rcvlist_lo,		/* NIB_RDRP Rcv Dsc Ring Ptr	*/
		se_rcvlist_hi:8,	/* Rcv list hi addr 		*/
		se_rcvresv:5,		/* reserved			*/
		se_rcvlen:3;		/* RLEN Rcv ring length		*/
	u_short	se_xmtlist_lo,		/* NIB_TDRP Xmt Dsc Ring Ptr	*/
		se_xmtlist_hi:8,	/* Xmt list hi addr 		*/
		se_xmtresv:5,		/* reserved			*/
		se_xmtlen:3;		/* TLEN Xmt ring length		*/
};

/*
 * VAXstar initialization block mode word
 */
#define SE_DRX			0x0001	/* Disable receiver		*/
#define SE_DTX			0x0002	/* Disable transmitter		*/
#define SE_LOOP			0x0004	/* Loopback control		*/
#define SE_DTCR			0x0008	/* Disable transmit CRC		*/
#define SE_COLL			0x0010	/* Force collision		*/
#define SE_DIRTY		0x0020	/* Disable retry		*/
#define SE_INTL			0x0040	/* Internal loopback		*/
/* #define SE_XXXX		0x0080	/* reserved			*/
/* #define SE_XXXX		0x0100	/* reserved			*/
/* #define SE_XXXX		0x0200	/* reserved			*/
/* #define SE_XXXX		0x0400	/* reserved			*/
/* #define SE_XXXX		0x0800	/* reserved			*/
/* #define SE_XXXX		0x1000	/* reserved			*/
/* #define SE_XXXX		0x2000	/* reserved			*/
/* #define SE_XXXX		0x4000	/* reserved			*/
#define SE_PROM			0x8000	/* Promiscuous mode		*/

/*
 * VAXstar CSR3 status word
 * (these should always be 0 for the VAXstar hardware)
 */
#define SE_BCON			0x0001	/* Byte control 		*/
#define SE_ACON			0x0002	/* ALE control			*/
#define SE_BSWP			0x0004	/* Byte swap (for DMA)		*/

/*
 * VAXstar ring descriptors
 */
struct se_ring	{
	u_short se_addr_lo;		/* Low order bits of buf addr	*/
	char se_addr_hi;		/* Hi order bits of buf addr	*/
	char se_flag;			/* rcv/xmt status flag		*/
	short se_buf_len;		/* buffer length (2's comp)	*/
	u_short se_flag2;		/* transmit: status/TDR		*/
					/* receive: len of packet	*/
};

/*
 * VAXstar receive status (se_flag)
 */
#define SE_ENP			0x0001	/* End of packet		*/
#define SE_STP			0x0002	/* Start of packet		*/
#define SE_RBUFF		0x0004	/* Receive Buffer error		*/
#define SE_CRC			0x0008	/* Checksum error		*/
#define SE_OFLO			0x0010	/* Overflow error		*/
#define SE_FRAM			0x0020	/* Framing error		*/
#define SE_RT_ERR		0x0040	/* Lance Error summary 		*/
#define SE_OWN			0x0080	/* Owned flag (1=Lance, 0=host)	*/

/*
 * VAXstar transmit status (se_flag)
 */
/* #define SE_ENP		0x0001	/* End of packet		*/
/* #define SE_STP		0x0002	/* Start of packet		*/
#define	SE_DEF			0x0004	/* Deferred - network busy	*/
#define	SE_ONE			0x0008	/* One retry was required	*/
#define	SE_MORE			0x0010	/* More retries were required	*/
/* #define SE_XXXX		0x0020	/* reserved			*/
/* #define SE_RT_ERR		0x0040	/* Lance Error summary		*/
/* #define SE_OWN		0x0080	/* Owned flag (1=Lance, 0=host)	*/

/* receive length */
#define SE_MCNT			0x0fff	/* low 12 bits of se_flag2	*/

/* transmit status (se_flag2,15:10) */
#define	SE_TBUFF		0x8000	/* Transmit buffer error	*/
#define	SE_UFLO			0x4000	/* Underflow			*/
/* #define SE_XXXX		0x2000	/* reserved			*/
#define	SE_LCOL			0x1000	/* Late collision		*/
#define	SE_LCAR			0x0800	/* Loss of carrier		*/
#define	SE_RTRY			0x0400	/* Retries exhausted		*/

/* transmit TDR (se_flag2,9:0) */
#define SE_TDR			0x03ff	/* Time Domain Reflectometer	*/

/*
 * VAXstar command and status bits (CSR0)
 */
#define SE_INIT			0x0001	/* (Re)-Initialize		*/
#define SE_START		0x0002	/* Start operation		*/
#define SE_STOP			0x0004	/* Reset firmware (stop)	*/
#define SE_RESET	SE_STOP		/*   "				*/
#define SE_TDMD			0x0008	/* Transmit on demand		*/
#define SE_XMIT_DEMAND	SE_TDMD		/*   "				*/
#define SE_TXON			0x0010	/* Transmitter is enabled	*/
#define SE_RXON			0x0020	/* Receiver is enabled		*/
#define SE_INEA			0x0040	/* Interrupt enable		*/
#define SE_INT_ENABLE	SE_INEA		/*   "				*/
#define SE_INTR			0x0080	/* Interrupt request		*/
#define SE_IDON 		0x0100	/* Initialization done		*/
#define SE_TINT			0x0200	/* Transmitter interrupt	*/
#define SE_XMIT_INT	SE_TINT		/*   "				*/
#define SE_RINT			0x0400	/* Receive interrupt		*/
#define SE_RCV_INT	SE_RINT		/*   "				*/
#define SE_MERR			0x0800	/* Memory error			*/
#define SE_MISS			0x1000	/* Missed packet		*/
#define SE_CERR			0x2000	/* Collision error		*/
#define SE_BABL			0x4000	/* Transmit timeout err		*/
#define SE_ERR			0x8000	/* Error summary		*/

/*
 * VAXstar CSR select
 */
#define SE_CSR0			0x0000	/* CSR 0			*/
#define SE_CSR1			0x0001	/* CSR 1			*/
#define SE_CSR2			0x0002	/* CSR 2			*/
#define SE_CSR3			0x0003	/* CSR 3			*/

/*
 * General constant definitions
 */
#define SEALLOC 		1	/* Allocate an mbuf		*/
#define SENOALLOC		0	/* No mbuf allocation		*/
#define SE_CRC_INIT		1	/* initialize CRC table		*/

