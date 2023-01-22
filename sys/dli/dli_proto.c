#ifndef	lint
static char *sccsid = "@(#)dli_proto.c	1.9	ULTRIX	1/9/87";
#endif	lint

/*
 * Program dli_proto.c,  Module DLI 
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
#include "../h/socket.h"
#include "../h/protosw.h"
#include "../h/domain.h"
#include "../h/mbuf.h"

#include "../net/if.h"

#include "../netinet/in.h"
#include "../netinet/if_ether.h"

#include "../netdnet/dli_var.h"


/*
 * Definitions of protocols supported in the DLI domain.
 */
extern int dli_usrreq(),dli_init(),dli_ifoutput(),dli_ifinput(),dli_ifioctl(),dli_slowtimo(), dli_ifstate();


struct protosw dlisw[] =
{
    { SOCK_DGRAM,     &dlidomain,           DLPROTO_DLI,     PR_ATOMIC | PR_ADDR,
      0,              0,                0,               0,
      dli_usrreq,     dli_init,         0,               dli_slowtimo,
      0,              dli_ifoutput,     dli_ifinput,     dli_ifioctl,
      dli_ifstate
    }
};

struct domain dlidomain =
    { AF_DLI,  "DLI", 0, 0, 0, dlisw, &dlisw[sizeof(dlisw)/sizeof(dlisw[0])] };

