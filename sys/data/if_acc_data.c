/*
 * @(#)if_acc_data.c	1.4	(ULTRIX)	3/16/85
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

#include "acc.h"


/*
 * ACC LH/DH ARPAnet IMP interface driver.
 */
#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/buf.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../h/vmmac.h"

#include "../net/if.h"
#include "../netimp/if_imp.h"

#include "../vax/cpu.h"
#include "../vax/mtpr.h"
#include "../vaxif/if_accreg.h"
#include "../vaxif/if_uba.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"

/*
 * "Lower half" of IMP interface driver.
 *
 * Each IMP interface is handled by a common module which handles
 * the IMP-host protocol and a hardware driver which manages the
 * hardware specific details of talking with the IMP.
 *
 * The hardware portion of the IMP driver handles DMA and related
 * management of UNIBUS resources.  The IMP protocol module interprets
 * contents of these messages and "controls" the actions of the
 * hardware module during IMP resets, but not, for instance, during
 * UNIBUS resets.
 *
 * The two modules are coupled at "attach time", and ever after,
 * through the imp interface structure.  Higher level protocols,
 * e.g. IP, interact with the IMP driver, rather than the ACC.
 */

#ifdef	BINARY

extern	struct	acc_softc {
	struct	ifnet *acc_if;		/* pointer to IMP's ifnet struct */
	struct	impcb *acc_ic;		/* data structure shared with IMP */
	struct	ifuba acc_ifuba;	/* UNIBUS resources */
	struct	mbuf *acc_iq;		/* input reassembly queue */
	short	acc_olen;		/* size of last message sent */
	char	acc_flush;		/* flush remainder of message */
} acc_softc[];


extern	struct  uba_device *accinfo[];

extern	int	nNACC;

#else

struct	acc_softc {
	struct	ifnet *acc_if;		/* pointer to IMP's ifnet struct */
	struct	impcb *acc_ic;		/* data structure shared with IMP */
	struct	ifuba acc_ifuba;	/* UNIBUS resources */
	struct	mbuf *acc_iq;		/* input reassembly queue */
	short	acc_olen;		/* size of last message sent */
	char	acc_flush;		/* flush remainder of message */
} acc_softc[NACC];


struct  uba_device *accinfo[NACC];
	
int	nNACC = NACC;

#endif

