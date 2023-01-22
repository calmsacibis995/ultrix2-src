/*
 * @(#)if_un_data.c	1.4	(ULTRIX)	3/16/85
 */
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

#include "un.h"
/*
 * Ungermann-Bass network/DR11-W interface driver
 */
#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/buf.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../h/vmmac.h"
#include "../h/errno.h"
#include "../h/time.h"
#include "../h/kernel.h"
#include "../h/ioctl.h"

#include "../net/if.h"
#include "../net/netisr.h"
#include "../net/route.h"
#include "../netinet/in.h"
#include "../netinet/in_systm.h"
#include "../netinet/ip.h"
#include "../netinet/ip_var.h"

#include "../vax/cpu.h"
#include "../vax/mtpr.h"
#include "../vaxif/if_un.h"
#include "../vaxif/if_unreg.h"
#include "../vaxif/if_uba.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"

/*
 * Ungermann-Bass software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * us_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address,
 * etc.  We also have, for each interface, a UBA interface structure, which
 * contains information about the UNIBUS resources held by the interface:
 * map registers, buffered data paths, etc.  Information is cached in this
 * structure for use by the if_uba.c routines in running the interface
 * efficiently.
 */

#ifdef BINARY

extern	struct un_softc {
	struct	ifnet us_if;		/* network-visible interface */
	struct	ifuba us_ifuba;		/* UNIBUS resources */
	short	us_state;		/* device state */
	short	us_errcnt;		/* number of errors since time set */
	short	us_restart;		/* restart interval */
	u_char	us_maxtime;		/* interval for error counting */
	u_char	us_maxerr;		/* errors allowed in interval */
	time_t	us_errtime;		/* time for error counting */
} un_softc[];
extern	struct	uba_device *uninfo[];
extern	int	nNUN;
#else

struct un_softc {
	struct	ifnet us_if;		/* network-visible interface */
	struct	ifuba us_ifuba;		/* UNIBUS resources */
	short	us_state;		/* device state */
	short	us_errcnt;		/* number of errors since time set */
	short	us_restart;		/* restart interval */
	u_char	us_maxtime;		/* interval for error counting */
	u_char	us_maxerr;		/* errors allowed in interval */
	time_t	us_errtime;		/* time for error counting */
} un_softc[NUN];
struct	uba_device *uninfo[NUN];

int	nNUN = NUN;
#endif
