#ifndef	lint
static char *sccsid = "@(#)dli_data.c	1.8	ULTRIX	1/9/87";
#endif	lint

/*
 * Program dli_data.c,  Module DLI 
 *
 * Copyright (C) 1985 by
 * Digital Equipment Corporation, Maynard, Mass.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 *
 * Networks & Communications Software Engineering
 *
 * IDENT HISTORY:
 *
 * 1.00 10-Jul-1985
 *      DECnet-ULTRIX   V1.0
 *
 * 2.00 18-Apr-1986
 *		DECnet-Ultrix	V2.0
 *
 * Added sysid and point-to-point support
 *
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/errno.h"

#include "../net/if.h"

#include "../netinet/in.h"
#include "../netinet/if_ether.h"

#include "../netdnet/dli_var.h"


struct dli_recv rcv;

struct ether_header no_enet_header;	/* Empty Ethernet header */
struct ifqueue dli_intrq;		/* input packet queue */
struct dli_line dli_ltable[DLI_MAXLINE]; /* DLI line table */
struct dli_timers lback_timers[DLI_MAX_LBTIMR]; /* loopback evl timers */

u_short	nqna;                            /* number of qna's */
u_char	sysid_mcast[] = SYSIDMCAST;      /* mcast addr for sysid */
u_char	sysid_msg[SYSID_MSGL];           /* sysid buffer */
struct	dli_sysid_to sysid_to[MAXQNAS];  /* time out structs */
struct	ether_pa *sysid_haddr_p;         /* ptr to dev addr in buff */
struct	sockaddr_dl sysid_dst;           /* destination info */

struct	if_isapt	if_isapt[MAX_BROADCSTDEV];	/* isap struct per device */
