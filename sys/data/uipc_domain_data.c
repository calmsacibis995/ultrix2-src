/*
 * @(#)uipc_domain_data.c	1.9	(ULTRIX)	8/10/86
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

#include "../h/param.h"
#include "../h/socket.h"
#include "../h/protosw.h"
#include "../h/domain.h"
#include "../h/time.h"
#include "../h/kernel.h"

#define	ADDDOMAIN(x)	{ \
	extern struct domain x/**/domain; \
	x/**/domain.dom_next = domains; \
	domains = &x/**/domain; \
}


#define	ADDDOMAIN(x)	{ \
	extern struct domain x/**/domain; \
	x/**/domain.dom_next = domains; \
	domains = &x/**/domain; \
}

#ifndef BINARY
domaininit()
{
	register struct domain *dp;
	register struct protosw *pr;

#ifndef lint
	ADDDOMAIN(unix);
#ifdef INET
#include "inet.h"
	ADDDOMAIN(inet);
#endif
#ifdef PUP
#include "pup.h"
	ADDDOMAIN(pup);
#endif
#include "imp.h"
#if NIMP > 0 && defined(INET)
	ADDDOMAIN(imp);
#endif
#ifdef DECNET
#include "decnet.h"
	ADDDOMAIN(decnet);
#endif
#ifdef LAT
#include "lat.h"
	ADDDOMAIN(lat);
#endif
#ifdef BSC
#include "bsc.h"
	ADDDOMAIN(bsc);
#endif
#ifdef DSS
	ADDDOMAIN(dss);
#endif
#ifdef DLI
#include "dli.h"
	ADDDOMAIN(dli);
#endif
#endif

	for (dp = domains; dp; dp = dp->dom_next)
		for (pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if (pr->pr_init)
				(*pr->pr_init)();
	if_to_proto_init();  /* init if_family structure */
	pffasttimo();
	pfslowtimo();
}

#endif
