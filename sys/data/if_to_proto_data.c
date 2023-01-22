/*
 * static	char	*sccsid = "@(#)if_to_proto_data.c	1.8	(ULTRIX)	3/12/86"
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

/************************************************************************
 *			Modification History				*
 *									*
 *	U. Sinkewicz - 03/12/86						*
 *		Added BSC info 2780/3780 Emulator			*
 *	Larry Cohen  -	09/16/85					*
 * 		Add 43bsd alpha tape changes for subnet routing		*
 *									*
 ************************************************************************/

#include "../h/param.h"
#include "../h/mbuf.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../net/af.h"
#include "../net/if.h"
#include "../net/if_to_proto.h"
#include "../netinet/in.h"
#include "../netinet/if_ether.h"


#define	IFNULL \
	{ 0,	0,	0,	0 }

#define IFEND \
	{ -1,	-1,	-1,	0}


#ifdef INET
#include "inet.h"
/*
 * need an entry for each device that has a data type field. 
 * ethernets and hyperchannels are examples
 */
#define	ETHER_IP  \
	{ ETHERTYPE_IP,	AF_INET,	IPPROTO_IP,	0 }
#else
#define	ETHER_IP	IFNULL
#endif



#ifdef DECNET
#include "decnet.h"
#include "../netdnet/dn.h"
#define	ETHER_DECNET  \
	{ ETHERTYPE_DN,	AF_DECnet,	DNPROTO_NSP,	0 }
#else
#define	ETHER_DECNET	IFNULL
#endif

#ifdef BSC
#include "bsc.h"
#include "../netbsc/bsc.h"
#define	E_BSC  \
	{ 0x0100,	AF_BSC,		0,		0 }
#else
#define	E_BSC	IFNULL
#endif

#ifdef LAT
#include "lat.h"
#define ETHER_LAT \
	{ ETHERTYPE_LAT,	AF_LAT,		0,		0 }
#else
#define ETHER_LAT	IFNULL
#endif

/*
 * The DLI entry should be the last one in the table since it will be the
 * destination for all packets which do not match any earlier entries.
 */
#ifdef DLI
#include "dli.h"
#define ETHER_DLI \
	{ -1,			AF_DLI,		0,		0 }
#else
#define ETHER_DLI	IFNULL
#endif

#ifdef	BINARY

extern struct if_family if_family[];

#else

/* INET specific stuff is kept in drivers for now */

struct if_family if_family[] = {
	ETHER_DECNET,	E_BSC,   ETHER_LAT,	ETHER_DLI,	IFEND
};

#endif
