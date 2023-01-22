/* #ifndef lint
 * static	char	*sccsid = "@(#)if_dpv_data.c	1.1	(ULTRIX)	5/22/86";
 *#endif lint
 */
/************************************************************************
 *									*
 *			Copyright (c) 1984 by				*
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
#include "dpv.h"

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/buf.h"
#include "../h/socket.h"
#include "../h/vmmac.h"	
#include "../h/ioctl.h"
#include "../h/errno.h"

#include "../h/dir.h"
#include "../h/user.h"
#include "../h/map.h"
#include "../h/uio.h"

#include "../net/if.h"
#include "../netbsc/bsc.h"
#include "../net/netisr.h"
#include "../net/route.h"
#include "../h/protosw.h"

#include "../vaxif/if_uba.h"
#include "../vaxif/if_dpvreg.h"

#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"

#include "../netbsc/bsc_var.h"
#include "../netbsc/bsc_messages.h"
#include "../netbsc/bsc_states.h"

#include "../vax/cpu.h"
#include "../vax/mtpr.h"

#ifdef BINARY

extern struct uba_device *dpvinfo[];

struct	dpv_softc {
	struct   ifnet  dpv_if;
	int		error;
	unsigned char	state;
	unsigned char	mode;
	unsigned short	crc;
	unsigned char	*pnt;
	unsigned char	*end;
	unsigned char	buf[DPVSIZE];
	int		counter;
	int		len;
} dpv_softc[]; 

extern int nNDPV;

#else

extern struct uba_device *dpvinfo[NDPV];

struct	dpv_softc {
	struct ifnet dpv_if;
	int 		error;
	unsigned char	state;
	unsigned char	mode;
	unsigned short	crc;
	unsigned char	*pnt;
	unsigned char	*end;
	unsigned char	buf[DPVSIZE];
	int		counter;
	int		len;
} dpv_softc[NDPV]; 

int nNDPV = NDPV;

#endif 
