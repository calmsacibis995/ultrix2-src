#ifndef	lint
static char *sccsid = "@(#)dli_timer.c	1.10	ULTRIX	1/30/87";
#endif	lint

/*
 * Program dli_timer.c,  Module DLI 
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
#include "../h/ioctl.h"

#include "../net/if.h"
#include "../netinet/in.h"
#include "../netinet/if_ether.h"

#include "../netdnet/evl.h"

#include "../netdnet/dli_var.h"

extern struct dli_timers lback_timers[];

extern struct dli_sysid_to sysid_to[];
extern struct sockaddr_dl sysid_dst;
extern u_short nqna;
extern u_char sysid_mcast[];
extern u_char sysid_msg[];
extern struct ether_pa *sysid_haddr_p;





/*
 *		d l i _ s l o w t i m o
 *
 * Slow timeout routine is entered every 500ms.  If loopback timer is
 * nonzero, it is decremented.  If the loopback timers makes a transition
 * to zero, a "passive loopback terminated" message is forwarded to EVL.
 *
 * Returns:		Nothing
 *
 * Inputs:		None
 */
dli_slowtimo()
{
    register int i;
    register struct ifnet *ifp;
    int s = splnet();

    for( i = 0; i < DLI_MAX_LBTIMR; i++ )
    {
	if ( lback_timers[i].tval != 0 )
		if ( --(lback_timers[i].tval) == 0 )
		{
			ifp = lback_timers[i].ifp;
			lback_timers[i].ifp = NULL;
			log_event(ifp, DLI_EVLOP_LBTER);
			if ( ifp->if_flags  & IFF_POINTOPOINT )
			{
				lback_timers[i].prev_devstate.if_next_family = lback_timers[i].prev_devstate.if_family;
				lback_timers[i].prev_devstate.if_family = AF_DLI;
				lback_timers[i].prev_devstate.if_xferctl = IFS_XFERCTL;
				lback_timers[i].prev_devstate.if_wrstate = IFS_WRSTATE;
				(*ifp->if_ioctl)(ifp, SIOCSTATE, &lback_timers[i].prev_devstate);
			}
		}
    }

	for(i = 0; i < nqna; i++)    /* check all qna's */
	{
		if ( (sysid_to[i].ifp->if_flags & IFF_RUNNING) == IFF_RUNNING)
		{
			if( !(sysid_to[i].ifp->if_flags & IFF_MOP) )
			{
				if(--sysid_to[i].to == 0)        /* time to tx? */
				{
					sysid_to[i].to = sysid_to[i].tr; /* reset timers */
					dli_snd_sysid(i, 0);            /* transmit sysid */
				}
			}
		}
	}
    splx(s);

    return;
}


/*
 *		d l i _ s n d _ s y s i d
 *
 * This routine is called to transmit the sysid for a particular qna
 *
 * Outputs:		None.
 *
 * Inputs:		Index to qna who is to transmit it's sysid.
 *
 * Version History:
 *
 */

dli_snd_sysid(i, receipt)
int i;
u_short receipt;
{

	int n;
	struct mbuf *m;
	register u_char *msgp;

	MGET(m, M_DONTWAIT, MT_DATA);
	if( m )
	{
		msgp = mtod(m, u_char *);
		(*sysid_to[i].ifp->if_ioctl)(sysid_to[i].ifp, SIOCRPHYSADDR, (caddr_t)&sysid_to[i].dev);
		*(struct ether_pa *)sysid_haddr_p = *(struct ether_pa *)sysid_to[i].dev.current_pa;
		sysid_dst.dli_device.dli_devnumber = sysid_to[i].ifp->if_unit;
		bcopy(sysid_msg, msgp, SYSID_MSGL);
		*(u_short *) (msgp+4) = receipt;
		m->m_next = NULL;
		m->m_len = SYSID_MSGL; 
		m->m_act = NULL;
		(*sysid_to[i].ifp->if_output)(sysid_to[i].ifp, m, &sysid_dst);
	}
	return;
}
