#ifndef	lint
static char *sccsid = "@(#)dli_output.c	1.9	ULTRIX	1/9/87";
#endif	lint

/*
 * Program dli_output.c,  Module DLI 
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



/*
 * DLI output subroutine.
 *
 *
 *
 *
 * Attempt to transmit all pending messages for the specified virtual circuit.
 *
 * Outputs:		None.
 *
 * Inputs:
 *	user_line	= Pointer to the user's line descriptor.
 *	m		= Pointer to mbuf chain to be transmitted.
 *	dst_addr	= Pointer to structure containing target address.
 */
dli_output( user_line, m, dst_addr )
register struct dli_line *user_line;
struct mbuf *m;
struct sockaddr_dl *dst_addr;
{
	register struct ifnet *ifp;
	register struct sockaddr_dl *out_addr;
	struct ifnet *match_device();
	register struct mbuf *m0 = NULL;
	struct mbuf *osi_buildhdr();
	int len;

	if ( dst_addr )
	{
		if ( dst_addr->dli_family != AF_DLI )
		{
			m_freem(m);
			return(EAFNOSUPPORT);
		}

		switch (dst_addr->dli_substructype)
		{
			case DLI_ETHERNET:
			case DLI_POINTOPOINT:
				m0 = m;
			case DLI_802:
				ifp = match_device(&dst_addr->dli_device);
				break;

			default:
				m_freem(m);
				return(EOPNOTSUPP);
				break;
		}

		out_addr = dst_addr;
	}
	else
	{
		if ( (out_addr = &user_line->dli_lineid )->dli_substructype != DLI_802 )
			m0 = m;
		ifp = user_line->dli_if;
	}

	if(ifp == NULL)
	{
		m_freem(m);
		return(ENODEV);
	}

	if ( ! (ifp->if_flags & IFF_RUNNING) || ! user_line->dli_sockopt.dli_state )
	{
		m_freem(m);
		return(ENETDOWN);
	}

	/*
	 * for 802 packets, validate user data length
	 * and then build the 802 header for transmission
	 */
	if(m0 == NULL)
	{
		len = mbuf_len(m);
		if(user_line->dli_lineid.choose_addr.dli_802addr.eh_802.ssap == SNAP_SAP && len > MAX802DATAP)
		{
			m_freem(m);
			return(EMSGSIZE);
		}
		else if(len > MAX802DATANP)
		{
			m_freem(m);
			return(EMSGSIZE);
		}
		if( (m0 = osi_buildhdr(out_addr)) == NULL)
		{
			m_freem(m);
			return(ENOBUFS);
		}
		m0->m_next = m;
	}

	return( (*ifp->if_output)(ifp, m0, out_addr) ); 
}
