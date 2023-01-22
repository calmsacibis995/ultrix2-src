/*
 *	@(#)if_se_data.c	1.7	(ULTRIX)	10/23/86
 */

/************************************************************************
 *									*
 *			Copyright (c) 1985, 1986 by			*
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
 *  Modification History:						*
 *									*
 *  20-Oct-86 -- jsd (John Dustin)					*
 *	Added use_type1 declaration if NFS & RPC are not configured	*
 *	Increased the number of recv descriptors from 16 to 32		*
 *									*
 *  26-Sep-86 -- jsd (John Dustin)					*
 *	Added ../h/cpudata.h for panic detection.			*
 *									*
 *  5-Sep-86  -- jsd (John Dustin)					*
 *	Removed obsolete code, cleaned up comments, etc.		*
 *									*
 * 14-Aug-86  -- jsd (John Dustin)					*
 *	Minor change for improved driver.				*
 *									*
 *   5-Aug-86  jsd (John Dustin)					*
 *	Rewrite for real VAXstar ethernet driver.			*
 *									*
 *   2-Jul-86  fred (Fred Canter)					*
 *	Fixed more collisions with symbols in if_qe.c.			*
 *									*
 *									*
 *  18-Jun-86  jsd (John Dustin)					*
 *	Created this data file for the VAXstar ethernet driver.		*
 *	Derived from if_qe_data.c.					*
 *									*
 ************************************************************************/

/*
 * Digital VAXstar NI
 */

#include "se.h"

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/buf.h"
#include "../h/cpudata.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../h/vmmac.h"
#include "../h/ioctl.h"
#include "../h/errno.h"
#include "../h/time.h"
#include "../h/kernel.h"
#include "../h/kmalloc.h"

#include "../net/if.h"
#include "../net/netisr.h"
#include "../net/route.h"
#include "../netinet/in.h"
#include "../netinet/in_systm.h"
#include "../netinet/in_var.h"
#include "../netinet/ip.h"
#include "../netinet/ip_var.h"
#include "../netinet/if_ether.h"
#include "../netpup/pup.h"

#include "../vax/cpu.h"
#include "../vax/mtpr.h"
#include "../vaxif/if_sereg.h"
#include "../vaxif/if_uba.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"

#define RLEN	5	/* 2**5 = 32 (1K each) receive descriptors */
#define TLEN	5	/* 2**5 = 32 (112 - 1K each) transmit descriptors */

#define NRCV	(0x0001<<RLEN) 	/* Receive descriptors */
#define NXMT	(0x0001<<TLEN) 	/* Transmit descriptors	*/
#define NTOT	(NXMT + NRCV)
#define NMULTI	12		/* Number of multicast addresses*/
#define MINDATA 64

/*
 * VAXstar NI sets the use_type1 flag, so must
 * define this variable here if NFS is not configured.
 */
#ifndef NFS
#ifndef RPC
int	use_type1;	/* dummy */
#endif
#endif

struct se_multi {
	u_char	se_multi_char[6];
};
#define MULTISIZE sizeof(struct se_multi)

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * is_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
#define	is_if	is_ac.ac_if		/* network-visible interface 	*/
#define	is_addr	is_ac.ac_enaddr		/* hardware Ethernet address 	*/

struct	se_softc {
	struct	se_ring *rringaddr;	/* Receive ring desc address 	*/
	struct	se_ring *tringaddr;	/* Transmit ring desc address 	*/
	struct  mbuf *rmbuf[NRCV+1];	/* Receive mbuf chains		*/
	struct  mbuf *tmbuf[NXMT+1];	/* Transmit mbuf chains		*/
	struct	arpcom is_ac;		/* Ethernet common part 	*/
	struct	se_multi multi[NMULTI];	/* Multicast address list	*/
	struct	estat ctrblk;		/* Counter block		*/
 	u_char	muse[NMULTI];		/* Multicast address usage count*/
	long	ztime;			/* Time counters last zeroed	*/
	int	rindex;			/* Receive index		*/
	int	tindex;			/* Transmit index		*/
	int	otindex;		/* Old transmit index		*/
 	int	nxmit;			/* Transmits in progress	*/
};

#ifdef BINARY

extern	struct	se_softc se_softc;
extern	struct	se_initb se_initb;
extern	struct	uba_device *seinfo[];
extern  int	nSENRCV;
extern	int	nSENXMT;
extern	int	nSENTOT;

#else

struct	se_softc  se_softc;
struct	se_initb  se_initb;
struct	uba_device *seinfo[NSE];
int	nSENXMT = NXMT;
int 	nSENRCV = NRCV;
int	nSENTOT = NTOT;

#endif

/*
 * VAXstar CSR network registers
 * see nb1_regs struct in src/sys/vaxuba/ubareg.h for nb_ni_rap, etc.
 */
#define	se_csrselect	nb_ni_rap	/* csr select (0-3) */
#define	se_csr		nb_ni_rdp	/* csr data register */

