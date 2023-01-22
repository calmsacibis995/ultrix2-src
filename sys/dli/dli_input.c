#ifndef	lint
static char *sccsid = "@(#)dli_input.c	1.13	ULTRIX	1/30/87";
#endif	lint

/*
 * Program dli_input.c,  Module DLI 
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
#include "../netdnet/dn.h"

#include "../netdnet/dli_var.h"

extern struct ifqueue dli_intrq;

extern struct dli_recv rcv;

extern struct ether_header no_enet_header;

extern u_char sysid_msg;
extern struct dli_sysid_to sysid_to[];
extern struct sockaddr_dl sysid_dst;
extern u_short nqna;



/*
 * DLI domain received message processing.
 */

 /*
 *		d l i i n t r
 *
 * DLI domain input routine. This routine is 'called' from the network software
 * ISR routine to process incoming packets. The first MBUF in any chain
 * contains a DLI receive descriptor containing the received ethernet header
 * and a pointer to the interface structure.
 *
 * Outputs:		None.
 *
 * Inputs:		None.
 */
dliintr()
{

    register struct mbuf *m;
    register struct dli_recv *recv;
    struct mbuf *loopback_enet_msg(), *loopback_ptop_msg();
	u_short proto_type;
    int s;
	u_char sysid_sc;


next:
    /*
     * Try to pull an input message (MBUF chain) from the DLI input queue.
     */
    s = splimp();
    IF_DEQUEUE(&dli_intrq, m);
    splx(s);
    if (m)
    {
	recv = mtod(m, struct dli_recv *);
	rcv.rcv_ifp = recv->rcv_ifp;
	bcopy(&recv->rcv_hdr.rcv_ether, &rcv.rcv_hdr.rcv_ether, sizeof(struct ether_header));
	proto_type = rcv.rcv_hdr.rcv_ether.ether_type;
	if ( bcmp(&(recv->rcv_hdr.rcv_ether), &no_enet_header, sizeof(recv->rcv_hdr.rcv_ether)) == 0 )
	{
		if ( m = loopback_ptop_msg(m, (rcv.rcv_ifp->if_flags & IFF_MOP)) )
		{
			forward_to_user(m);
		}
	}
	else
	{
		/*
		 * if ethernet protocol is < 0x5dc
		 * then this is the length field
		 * for an 802.3 packet
		 */
		if(proto_type <= OSI802)
		{
			osi_802input(m);
		}
		else if( (proto_type == SYSIDPROTO) && 
		(sysid_sc = *(mtod(m->m_next, u_char *) + sizeof(u_short)) == REQSYSID) )
		{
				dli_proc_reqsysid(m, recv);
		}
		else if ( (proto_type != DLI_LBACK) || (m = loopback_enet_msg(m)) )
		{
			forward_to_user(m);
		}
	}
	/*
	 * clear it out for the next one 
	 */
	bzero(&rcv, sizeof(struct dli_recv));
	goto next;
    }
}



/*
 *		f o r w a r d _ t o _ u s e r 	
 *
 * This routine attempts to find the user to whom the current
 * packet belongs.  If successful, the message is placed on
 * the user's socket receive queue.  Otherwise, the packet is
 * dropped.
 *
 * Outputs:		mbuf chain (possibly) place on user's rcv queue.
 *
 * Inputs:		m = mbuf chain containing packet.  Note that the
 *				first mbuf contains junk at this point.
 */

extern struct dli_line dli_ltable[];

forward_to_user( m )
register struct mbuf *m;
{
	register struct sockaddr_edl *eaddr;
	register struct sockaddr_802 *oaddr;
	register struct sockaddr_pdl *paddr;
	register int i;
	register int save_i = -1;


	for ( i = 0; i < DLI_MAXLINE; i++ )
	{
		if ( rcv.rcv_ifp != dli_ltable[i].dli_if )
 		{
 			continue;
 		}
	
		switch ( dli_ltable[i].dli_lineid.dli_substructype )
		{
			case DLI_802:
				/*
				 * the only valid way to get here 
				 * is from the 802 input routine
				 * for snap saps.
				 * all other saps have either been
				 * handled by the responder, or gone 
				 * directly to found_user
				 */
				if(rcv.rcv_hdr.osi_header.dsap != SNAP_SAP)
					break;
				oaddr = &dli_ltable[i].dli_lineid.choose_addr.dli_802addr;
				switch ( oaddr->ioctl )
				{
					case DLI_EXCLUSIVE:
						if(bcmp(rcv.rcv_hdr.osi_header.osi_pi, oaddr->eh_802.osi_pi, 5) == NULL)
						{
							found_user(m, dli_ltable[i].dli_so, DLI_802);
							return;
						}
						break;

					case DLI_NORMAL:
						if( (bcmp(rcv.rcv_hdr.osi_header.osi_pi, oaddr->eh_802.osi_pi, 5) == NULL) &&
							match_targets(oaddr->eh_802.dst, rcv.rcv_hdr.osi_header.src) &&
							match_mcast(dli_ltable[i].dli_sockopt.dli_mcast,
							rcv.rcv_hdr.osi_header.dst))
						{
							found_user(m, dli_ltable[i].dli_so, DLI_802);
							return;
						}
						break;

					case DLI_DEFAULT:
						if(bcmp(rcv.rcv_hdr.osi_header.osi_pi, oaddr->eh_802.osi_pi, 5) == NULL) 
						{
							save_i = i;
						}
						break;

					default:
						break;
				}
				break;

			case DLI_ETHERNET:
				eaddr = &dli_ltable[i].dli_lineid.choose_addr.dli_eaddr;
				switch ( eaddr->dli_ioctlflg )
				{
					case DLI_EXCLUSIVE:
						if (rcv.rcv_hdr.rcv_ether.ether_type == eaddr->dli_protype)
						{
							found_user(m, dli_ltable[i].dli_so, DLI_ETHERNET);
							return;
						}
						break;

					case DLI_NORMAL:
						if ( (rcv.rcv_hdr.rcv_ether.ether_type ==  eaddr->dli_protype) &&
						    match_targets(eaddr->dli_target, rcv.rcv_hdr.rcv_ether.ether_shost) &&
						    match_mcast(dli_ltable[i].dli_sockopt.dli_mcast,
						    rcv.rcv_hdr.rcv_ether.ether_dhost))
						{
							found_user(m, dli_ltable[i].dli_so, DLI_ETHERNET);
							return;
						}
						break;

					case DLI_DEFAULT:
						if (rcv.rcv_hdr.rcv_ether.ether_type == eaddr->dli_protype)
						{
							save_i = i;
						}
						break;

					default:
						break;
				}
				break;

			case DLI_POINTOPOINT:
				found_user(m, dli_ltable[i].dli_so, DLI_POINTOPOINT);
				return;
				break;

			default:
				panic( "dli_input: forward_to_user" );
				break;
		}

	}
	if ( save_i >= 0 && save_i <= DLI_MAXLINE )
	{
		found_user(m, dli_ltable[save_i].dli_so, dli_ltable[save_i].dli_lineid.dli_substructype);
		return;
	}
	m_freem(m);
	return;
}







/*
 *		m a t c h _ m c a s t
 *
 * This routine looks at the destination address, and if it is
 * multicast, it checks to see if the address matches the user's
 * multicast addresses.  Note that if the user hasn't specified
 * any, the test fails.
 *
 * Outputs:		NULL if no match, otherwise 1.
 *
 * Inputs:		mcast1 = string containing Ethernet dest address.
 * 			mcast2 = string containing user's multicast address(es).
 */
match_mcast( mcast2, mcast1 )
register u_char *mcast2, *mcast1;
{
	register i;

	/*
	 * If destination addr not multicast, pass test.
	 */
	
	if ( !(mcast1[0] & 1) )
	{
		return(1);
	}
	/*
	 * If multicast, perform test.
	 */
	for( i = 0; i < MCAST_ASIZE; i += MCAST_SIZE)
	{
		if ( bcmp(mcast1, mcast2+i, MCAST_SIZE) == NULL )
		{
			return(1);
		}
	}

	
	return(NULL);

}







/*
 *		f o u n d _ u s e r
 *
 * This routine places an mbuf chain on a user's receive queue.
 *
 * Outputs:		None.
 *
 * Inputs:		m = Pointer to mbuf chain; first mbuf is
 *				garbage.
 * 			so = Pointer to user's socket structure.
 *			link_type = type of link ( ethernet, point to point )
 */
found_user( m, so, link_type )
register struct mbuf *m;
register struct socket *so;
u_char link_type;
{
	register struct sockaddr_dl *so_dl;
	register struct mbuf *temp;
	u_short len;

	if ( so == NULL )
	{
		panic( "dli_input: found_user1" );
	}

	/*
	 * Make sure there's enough room on Rx Q
	 */
	if ( sbspace(&so->so_rcv) < 0 )
	{
		m_freem(m);
		return;
	}

	/*
	 * clear out first mbuf and place address info in it.
	 */
	so_dl = mtod(m, struct sockaddr_dl *);
	bzero((u_char *) so_dl, sizeof(struct sockaddr_dl));
	so_dl->dli_family = AF_DLI;
	switch ( so_dl->dli_substructype = link_type )
	{
		case DLI_802:
			/*
			 * only individual saps should be coming through here
			 */
			so_dl->choose_addr.dli_802addr.eh_802.len = rcv.rcv_hdr.osi_header.len;
			*(struct ether_pa *) so_dl->choose_addr.dli_802addr.eh_802.dst = *(struct ether_pa *) rcv.rcv_hdr.rcv_ether.ether_shost;
			*(struct ether_pa *) so_dl->choose_addr.dli_802addr.eh_802.src = *(struct ether_pa *) rcv.rcv_hdr.rcv_ether.ether_dhost;
			/* this is who enabled the sap */
			so_dl->choose_addr.dli_802addr.eh_802.ssap = rcv.rcv_hdr.osi_header.dsap;
			/* this is who it came from */
			so_dl->choose_addr.dli_802addr.eh_802.dsap = rcv.rcv_hdr.osi_header.ssap;
			if( (rcv.rcv_hdr.osi_header.ctl.U_fmt & 3) == 3)
			{
				so_dl->choose_addr.dli_802addr.eh_802.ctl.U_fmt = rcv.rcv_hdr.osi_header.ctl.U_fmt;
			}
			else
			{
				so_dl->choose_addr.dli_802addr.eh_802.ctl.I_S_fmt = rcv.rcv_hdr.osi_header.ctl.I_S_fmt;
			}
			break;

		case DLI_ETHERNET:
			so_dl->choose_addr.dli_eaddr.dli_protype = rcv.rcv_hdr.rcv_ether.ether_type;
			*(struct ether_pa *) so_dl->choose_addr.dli_eaddr.dli_target = *(struct ether_pa *) rcv.rcv_hdr.rcv_ether.ether_shost;
			*(struct ether_pa *) so_dl->choose_addr.dli_eaddr.dli_dest = *(struct ether_pa *) rcv.rcv_hdr.rcv_ether.ether_dhost;
			break;

		case DLI_POINTOPOINT:
			break;

		default:
			panic( "dli, found_user2" );
			break;
	}

	so_dl->dli_device.dli_devnumber = rcv.rcv_ifp->if_unit;
	bcopy(rcv.rcv_ifp->if_name, so_dl->dli_device.dli_devname, strlen(rcv.rcv_ifp->if_name));
	if ( sbappendanyaddr(&so->so_rcv, (u_char *) so_dl, sizeof(struct sockaddr_dl), m->m_next, NULL) == 0 )
	{
		m_freem(m);
		return;
	}
	MFREE(m, temp);
	sorwakeup(so);
	return;
}



extern struct dli_timers lback_timers[];
/*
 *		l o o p b a c k _ e n e t _ m s g
 *
 *		This routine processes Ethernet loopback messages.  If the
 *		function code is "forward data," then the message
 *		is forwarded to its next destination.  Otherwise,
 *		nothing is done.
 *
 * Outputs:		mbuf chain given to driver if message to be forwared.
 *			returns NULL if message looped, otherwise mbuf pointer returned.
 *
 * Inputs:		m = mbuf chain containing packet.  
 */
struct mbuf *loopback_enet_msg( m )
register struct mbuf *m;
{
    static struct sockaddr_dl dst_addr;
    u_char *loop_msg;
    u_short loop_sc, timer_active, i;
    struct ifdevea ifd;

    /*
     * pull up header into second mbuf
     */
    if ( ! pull_header(m, 30) )
    {
	return(NULL);
    }

    /*
     * determine if message should be looped back
     */
    if ( (loop_sc = *(mtod(m->m_next, u_short *)) + sizeof(loop_sc)) > 28 )
    {
	return(m);
    }
    loop_msg = mtod(m->m_next, u_char *);
    if ( loop_msg[loop_sc++] != DLI_LBACK_FWD || loop_msg[loop_sc++] != NULL )
    {
	return(m);
    }
    if ( (rcv.rcv_ifp->if_ioctl(rcv.rcv_ifp, SIOCRPHYSADDR, (caddr_t)&ifd))
	|| (bcmp(ifd.current_pa, rcv.rcv_hdr.rcv_ether.ether_shost, sizeof(rcv.rcv_hdr.rcv_ether.ether_dhost)) == 0) )
    {
		m_freem(m);
		return(NULL);
    }
    m = m_free(m);

    /*
     * log passive loopback initiated event if not already given for
     * present node.
     */
     timer_active = NULL;
     for( i = 0; i < DLI_MAX_LBTIMR; i++)
     {
	if ( (lback_timers[i].tval != 0) && 
     		bcmp(rcv.rcv_hdr.rcv_ether.ether_shost, lback_timers[i].actv_addr, DLI_EADDRSIZE) == NULL )
	{
		lback_timers[i].tval = DLI_LBEVL_WAIT;
		timer_active = 1;
		break;
	}
     }
     if ( ! timer_active && establish_event( &rcv.rcv_hdr.rcv_ether ) )
     {
	log_event(rcv.rcv_ifp, DLI_EVLOP_LBINI);
     }


    /*
     * forward loopback message.
     */
    dst_addr.dli_family = AF_DLI;
    dst_addr.dli_substructype = DLI_ETHERNET;
    *(struct ether_pa *) dst_addr.choose_addr.dli_eaddr.dli_target = *(struct ether_pa *) (loop_msg+loop_sc);
    dst_addr.choose_addr.dli_eaddr.dli_protype = rcv.rcv_hdr.rcv_ether.ether_type;
    loop_sc += (DLI_EADDRSIZE - sizeof(loop_sc));
    *(u_short *)loop_msg = loop_sc;
    rcv.rcv_ifp->if_output(rcv.rcv_ifp, m, &dst_addr);
    return(NULL);

}



/*
 *		l o o p b a c k _ p t o p _ m s g
 *
 *		This routine processes Point to Point loopback messages.  
 *
 * Outputs:		mbuf chain given to driver if message to be forwared.
 *			returns NULL if message looped, otherwise mbuf pointer returned.
 *
 * Inputs:		m = mbuf chain containing packet.  
 *			mop = 1 if MOP, 0 if not MOP mode
 */
struct mbuf *loopback_ptop_msg( m, mop )
register struct mbuf *m;
int mop;
{
	u_char *mop_code;
	u_short i;
	static struct sockaddr_dl dst_addr;

	/*
	 * make sure device is currently looping; if so, reset loopback timer;
	 * if not, packet belongs to a user.
	 */
 	for( i = 0; i < DLI_MAX_LBTIMR; i++)
 	{
		if ( (lback_timers[i].tval != 0) &&  (rcv.rcv_ifp == lback_timers[i].ifp) )
		{
			lback_timers[i].tval = DLI_LBEVL_POP;
			break;
		}
     	}
	if ( i == DLI_MAX_LBTIMR && ! mop )
	{
		return(m);
	}

	/*
	 * pull MOP code into second mbuf if not already there
	 */
	if ( ! pull_header(m, 1) )
	{
		return(NULL);
	}
	else if ( *(mop_code = mtod(m->m_next, u_char *)) == DLI_LBACK_LOOP )
	{
		/* free first mbuf which has no relevant data */
		m = m_free(m);

		/*
		 * forward loopback message.
		 */
		dst_addr.dli_family = AF_DLI;
		dst_addr.dli_substructype = DLI_POINTOPOINT;
		*mop_code = (u_char) DLI_LBACK_LOOPED;
		rcv.rcv_ifp->if_output(rcv.rcv_ifp, m, &dst_addr);
		return(NULL);
	}
	else
	{
		return(m);
	}
}



/*
 *		l o g _ e v e n t
 *
 *	This subroutine logs a passive loopback message to evl.
 *
 * Outputs:		None.
 *
 * Inputs:		None.  
 */
log_event(ifp, evl_op)
register struct ifnet *ifp;
u_char evl_op;
{
    struct protosw *evl_ptr;
    static struct event event;
    register int i;

    /*
     * log event only if evl is present
     */
    if ( ! (evl_ptr = pffindproto( AF_DECnet, DNPROTO_EVR )) )
    {
	return;
    }


    /*
     * init event structure.
     */
    event.e_class = DLI_LBEVL_CLASS;
    event.e_type = DLI_LBEVL_TYPE;
    event.e_ent_type = DLI_LBEVL_ETYPE;
    i = fetch_decnet_devname( ifp, event.e_ent_id );
    event.e_ent_id[i++] = ifp->if_unit + '0';
    event.e_ent_id[i] = NULL;
    event.e_data[0] = DLI_EVLOP_CODE;
    event.e_data[1] = NULL;
    event.e_data[2] = DLI_EVLOP_DESC;
    event.e_data[3] = evl_op;
    event.e_data_len = 4;


    /*
     * log event
     */
    (evl_ptr->pr_input)( &event );
    return;

}



/*
 *		s c m p
 *
 *	This subroutine compares two strings.
 *
 * Outputs:		0 if strings unequal, 1 if strings equal.
 *
 * Inputs:		s1, s2 = pointers to strings to be compared.  
 */
scmp(s1, s2)
register char *s1, *s2;
{
    while ( *s1 == *s2++ )
	if ( *s1++ == NULL )
		return(1);
    return(0);
}



/*
 *		p u l l _ h e a d e r
 *
 *		This routine pulls up a header into the second mbuf.  
 * 		NOTE: first mbuf contains info placed by dli_ifinput.
 *
 * Outputs:		1 if successful, 0 if failure.
 *
 * Inputs:		m = mbuf chain containing packet.  
 *			hsiz = size of header to be pulled up.
 */
pull_header( m, hsiz )
register struct mbuf *m;
register short hsiz;
{

    register int i = 0;
    register struct mbuf *tm = m->m_next;

    while ( tm )
    {
	i += tm->m_len;
	tm = tm->m_next;
    }
    if ( (m->m_next = m_pullup(m->m_next, ((i < hsiz) ? i : hsiz))) == NULL )
    {
	m_free(m);
	return(NULL);
    }
    return(1);

}



/*
 *		e s t a b l i s h _ e v e n t
 *
 *	This routine sets up a loopback event in the
 *	loopback timer table.
 *
 *
 * Inputs:		header = address of packet header.  
 *				 (zeroed out for point to point)
 *
 * Outputs:		1 if success, otherwise NULL.
 */
establish_event( eh )
register struct ether_header *eh;
{
	int i = -1;

	while (lback_timers[++i].tval != 0 && i < DLI_MAX_LBTIMR) ;

	if ( i < DLI_MAX_LBTIMR )
	{
		lback_timers[i].tval = DLI_LBEVL_WAIT;
		lback_timers[i].ifp = rcv.rcv_ifp;
	     	*(struct ether_pa *) lback_timers[i].actv_addr = *(struct ether_pa *) eh->ether_shost;
		return(1);
	}
	else
	{
		return(0);
	}

}






/*
 *		f e t c h _ d e c n e t _ d e v n a m e
 *
 *	This routine translates the ULTRIX device name into the DECnet
 *	device name.
 *
 * Inputs:		dn_devname = pointer where DECnet device name is to 
 *					placed.
 *
 * Outputs:		dn_devname = DECnet device name.
 *
 * Returns:		number of characters in device name.
 *
 */
fetch_decnet_devname( ifp, dn_devname )
register struct ifnet *ifp;
register u_char *dn_devname;
{
    register int i = 0;

    if ( scmp(ifp->if_name, "qe") )
    {
	bcopy( "QNA-", dn_devname, (i = 4) );
    }
    else if ( scmp(ifp->if_name, "de") )
    {
	bcopy( "UNA-", dn_devname, (i = 4) );
    }
    else if ( scmp(ifp->if_name, "ni") )
    {
	bcopy( "BNT-", dn_devname, (i = 4) );
    }
    else if ( scmp(ifp->if_name, "se") )
    {
	bcopy( "SVA-", dn_devname, (i = 4) );
    }
    else if ( scmp(ifp->if_name, "dmc") )
    {
	bcopy( "DMC-", dn_devname, (i = 4) );
    }
    else if ( scmp(ifp->if_name, "dmv") )
    {
	bcopy( "DMV-", dn_devname, (i = 4) );
    }
    else
    {
	bcopy( "???-", dn_devname, (i = 4) );
    }

    return(i);

}

/*
 *		m b u f _ l e n
 *
 * Compute the number of bytes in a (non-empty) MBUF chain.
 *
 * Returns:		The number of bytes in the chain.
 *
 * Inputs:
 *	m		= Pointer to the MBUF chain.
 */
mbuf_len( m )
register struct mbuf *m;
{
    register struct mbuf *m0 = m;
    register int len = 0;

	while( m0 )
    {
		len += m0->m_len;
		m0 = m0->m_next;
    }

    return (len);
}


/*
 *		d l i _ p r o c _ r e q s y s i d
 *
 * This routine is called to process requests for sysid 
 * If this is a request for a sysid from a node
 *    find the matching sysid_to struct to tx on
 *    copy requestor node address to the target
 *    transmit the sysid 
 *
 * Outputs:		None.
 *
 * Inputs:		pointer to mbuf chain from driver
 *              pointer to dli_recv structure.
 *
 * Version History:
 * 1.0	JA
 *
 */

dli_proc_reqsysid(m, recv)
struct mbuf *m;
struct dli_recv *recv;
{
	register int i;

	for(i = 0; i < nqna; i++)   /* find matching sysid_to struct */
	{
		if(sysid_to[i].ifp == recv->rcv_ifp)
		{
			if( (sysid_to[i].ifp->if_flags & IFF_RUNNING) == IFF_RUNNING)
			{
				*(struct ether_pa *)sysid_dst.choose_addr.dli_eaddr.dli_target = *(struct ether_pa *)recv->rcv_hdr.rcv_ether.ether_shost; 
				dli_snd_sysid(i, *(u_short *) (mtod(m->m_next, u_char *) + 4) );
			}
		}
	}
	m_free(m);
}
