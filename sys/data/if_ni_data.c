/* @(#)if_ni_data.c	1.4	(ULTRIX)	12/4/86 */

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
 *									*
 * 	12-Dec-86  -- lp	Added dpaddr.
 * 
 * 	5-Jun-86   -- jaw 	changes to config.
 *
 *	09-Apr-86 -- lp							*
 *	Created by Larry Palmer (lp!decvax)				*
 *									*
 ************************************************************************/

#include "bvpni.h"

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/buf.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../h/vmmac.h"
#include "../h/ioctl.h"
#include "../h/errno.h"
#include "../h/time.h"
#include "../h/kmalloc.h"

#include "../net/if.h"
#include "../net/netisr.h"
#include "../net/route.h"
#include "../netinet/in.h"
#include "../netinet/in_var.h"
#include "../netinet/in_systm.h"
#include "../netinet/ip.h"
#include "../netinet/ip_var.h"
#include "../netinet/if_ether.h"
#include "../netpup/pup.h"

#include "../vax/cpu.h"
#include "../vax/mtpr.h"
#include "../vaxif/if_dereg.h"
#include "../vaxif/if_uba.h"

#include "../vaxbi/bireg.h"
#include "../vaxbi/buareg.h"
#include "../vaxbi/nireg.h"

#include "../vaxuba/ubavar.h"


#ifdef	BINARY

extern	struct	ni niinfo[];
/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * ds_if, which the routing code uses to locate the interface.
 */
struct	ni_softc {
	struct	arpcom ds_ac;		/* Ethernet common part */
#define	ds_if	ds_ac.ac_if		/* network-visible interface */
#define	ds_addr	ds_ac.ac_enaddr		/* hardware Ethernet address */
	int	ds_flags;
	long	ds_devid;
	u_char  ds_multi[NMULTI][8];
	long	ds_muse[NMULTI];
	u_char  ds_dpaddr[6];
} ni_softc[];



extern int	nNI;

#else

struct ni niinfo[NBVPNI];
/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * ds_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
struct	ni_softc {
	struct	arpcom ds_ac;		/* Ethernet common part */
#define	ds_if	ds_ac.ac_if		/* network-visible interface */
#define	ds_addr	ds_ac.ac_enaddr		/* hardware Ethernet address */
	int	ds_flags;
	long	ds_devid;
	u_char  ds_multi[NMULTI][8];
	long	ds_muse[NMULTI];
	u_char  ds_dpaddr[6];
} ni_softc[NBVPNI];

int nNI = NBVPNI;


#endif

