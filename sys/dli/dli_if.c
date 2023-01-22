#ifndef	lint
static char *sccsid = "@(#)dli_if.c	1.9	ULTRIX	1/9/87";
#endif	lint

/*
 * Program dli_if.c,  Module DLI 
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
 *      DECnet-ULTRIX   V2.0
 *		- added sysid and point to point support
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/errno.h"

#include "../net/if.h"
#include "../net/netisr.h"
#include "../netinet/in.h"
#include "../netinet/if_ether.h"

#include "../netdnet/dli_var.h"

#include "../vax/cpu.h"
#include "../vax/mtpr.h"

extern struct ifqueue dli_intrq;



/*
 * Routines to handle protocol specific needs of interface drivers.
 */

/*
 *		d l i _ i f o u t p u t
 *
 * Perform protocol specific functions for message transmission.
 *
 * Returns:		Nothing
 *
 * Inputs:
 *	ifp		= Pointer to output device structure
 *	m		= Pointer to MBUF chain to be output
 *	sockdst		= Destination socket address
 *	type		= Location to return link level protocol type
 *	net_dst		= Location(s) to return destination address
 */
dli_ifoutput( ifp,m,sockdst,type,net_dst )
struct ifnet *ifp;
struct mbuf *m;
struct sockaddr_dl *sockdst;
int *type;
char *net_dst;
{

    switch ( sockdst->dli_substructype )
    {

	case DLI_802:
		*(struct ether_pa *)net_dst = *(struct ether_pa *)sockdst->choose_addr.dli_802addr.eh_802.dst;
		/*
		 * 802.3 - the ethernet protocol field is the length 
		 * of the 802.3 header information and the user data
		 */
		*type = (u_short)mbuf_len(m);
		break;

	case DLI_ETHERNET:
		*(struct ether_pa *) net_dst = *(struct ether_pa *) sockdst->choose_addr.dli_eaddr.dli_target;
    		*type = sockdst->choose_addr.dli_eaddr.dli_protype;
		break;

	case DLI_POINTOPOINT:
		break;

	default:
		panic( "dli_ifoutput" );
		break;
    }
}



/*
 *		d l i _ i f i n p u t
 *
 * Perform protocol specific functions for message reception.
 *
 * Returns:		Pointer to the (possibly modified) MBUF chain of
 *			the received message
 *			0 if allocation failed.
 *
 * Inputs:
 *	m		= Pointer to MBUF chain of received message
 *	ifp		= Pointer to input device structure
 *	inq		= Location to return pointer to protocol input queue
 *	eh		= Pointer to received Ethernet header, if appropriate
 */


extern struct dli_line dli_ltable[];
extern struct ether_header no_enet_header;


struct mbuf *dli_ifinput( m,ifp,inq,eh )
struct mbuf *m;
struct ifnet *ifp;
struct ifqueue **inq;
struct ether_header *eh;
{
    struct mbuf *m0;
    struct dli_recv *recv;

    MGET(m0, M_DONTWAIT, MT_HEADER);
    if ( m0 )
    {
	m0->m_next = m;
	m0->m_act = NULL;
	m0->m_len = sizeof(struct dli_recv);
	recv = mtod(m0, struct dli_recv *);
	if ( eh )
		recv->rcv_hdr.rcv_ether = *eh;
	else
		recv->rcv_hdr.rcv_ether = no_enet_header;
	recv->rcv_ifp = ifp;
	schednetisr(NETISR_DLI);
	*inq = &dli_intrq;
	return (m0);
    }
    m_freem(m);
    return (0);
}



extern struct dli_timers lback_timers[];
/*
 *		d l i _ i f s t a t e
 *
 * Perform protocol specific function for device state change.
 * Should be called from a driver.
 *
 * Returns:		0 = false, 1 = true.
 *
 * Inputs:
 *	ifp		= Pointer to device structure
 *	state		= I/O control command
 *	data		= Pointer to I/O control data
 */
dli_ifstate( ifp,state,data )
struct ifnet *ifp;
int state;
caddr_t data;
{
	int i;
	struct ifstate  *dmcstate = (struct ifstate *) data;

	switch ( state )
	{
		case IFS_HALTED:
		case IFS_HALTING:
			if ( dmcstate->if_family == AF_DLI ) {
				for ( i = 0 ; i < DLI_MAXLINE; i++ )
				{
					if ( (ifp == dli_ltable[i].dli_if) &&
						(dli_ltable[i].dli_sockopt.dli_state == DLS_ON) ) {
						*dmcstate = dli_ltable[i].dli_lineid.choose_addr.dli_paddr.dev_cstate;
						dmcstate->if_wrstate = ~IFS_WRSTATE;
						dmcstate->if_rdstate = ~IFS_RDSTATE;
						dmcstate->if_xferctl = ~IFS_XFERCTL;
					}
				}
			} else {
				dmcstate->if_family = dmcstate->if_next_family;
			}
			return(1);
			break;

		case IFS_ENTEREDMOP:
			for ( i = 0; i < DLI_MAX_LBTIMR; i++ )
			{
				if ( ! lback_timers[i].ifp || lback_timers[i].ifp == ifp)
				{
					break;
				}
			}
			if ( i == DLI_MAX_LBTIMR )
			{
				if ( (dmcstate->if_family = dmcstate->if_next_family) == AF_DECnet )
				{
					dmcstate->if_next_family = AF_UNSPEC;
					dmcstate->if_mode = IFS_DDCMPFDX;
					dmcstate->if_ustate = IFS_USRON;
				}
				else
				{
					dmcstate->if_ustate = IFS_USROFF;
				}

			}
			else
			{
				lback_timers[i].ifp = ifp;
				lback_timers[i].tval = DLI_LBEVL_POP*2;
				bzero(lback_timers[i].actv_addr, DLI_EADDRSIZE);
				lback_timers[i].prev_devstate = *dmcstate;
				log_event(ifp, DLI_EVLOP_LBINI);
				dmcstate->if_family = AF_DLI;;
				dmcstate->if_next_family = AF_UNSPEC;
				dmcstate->if_mode = IFS_MOP;
				dmcstate->if_nomuxhdr = IFS_NOMUXHDR;
				dmcstate->if_ustate = IFS_USRON;
			}
			return(1);
			break;

		case IFS_RUNNING:
			if ( (ifp->if_flags & IFF_POINTOPOINT) && (dmcstate->if_family == AF_DLI) ) {
				for ( i = 0 ; i < DLI_MAXLINE; i++ )
				{
					if (ifp == dli_ltable[i].dli_if) {
						if ( dli_ltable[i].dli_sockopt.dli_state == DLS_SLEEP ) {
							dli_ltable[i].dli_sockopt.dli_state = DLS_ON; 
							wakeup((caddr_t) ifp);
						}
						break;
					}
				}
				if ( i == DLI_MAXLINE ) {
					for ( i = 0; i < DLI_MAX_LBTIMR; i++ )
					{
						if ( lback_timers[i].ifp == ifp)
						{
							struct sockaddr_dl dst;
							struct mbuf *m;
    							MGET(m, M_DONTWAIT, MT_DATA);
    							if ( m )
    							{
								m->m_next = NULL;
								m->m_act = NULL;
								m->m_len = 1;
								*mtod(m, u_char *) = 0xc4;
								dst.dli_family = AF_DLI;
								dst.dli_substructype = DLI_POINTOPOINT;
								ifp->if_output(ifp, m, &dst);
							}
							break;
						}
					}
				}
			} 
			break;

		case IFS_STARTING:
		default:
			break;
	}

	return(0);
}


/*
 *		d l i _ i f i o c t l
 *
 * Perform protocol specific function for I/O control processing.
 * Should be called from a driver.
 *
 * Returns:		Nothing
 *
 * Inputs:
 *	ifp		= Pointer to device structure
 *	cmd		= I/O control command
 *	data		= Pointer to I/O control data
 */
dli_ifioctl( ifp,cmd,data )
struct ifnet *ifp;
int cmd;
caddr_t data;
{
}
