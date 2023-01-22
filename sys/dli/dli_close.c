#ifndef	lint
static char *sccsid = "@(#)dli_close.c	1.9	ULTRIX	1/15/87";
#endif	lint

/*
 * Program dli_close.c,  Module DLI 
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
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/protosw.h"
#include "../h/errno.h"
#include "../h/ioctl.h"

#include "../net/if.h"

#include "../netinet/in.h"
#include "../netinet/if_ether.h"

#include "../netdnet/dli_var.h"



/*
 *		d l i _ c l o s e
 *
 * Process a DLI close socket request.
 *
 * Returns:		Nothing
 *
 * Inputs:
 *	uentry		= Pointer to the line table entry for this request.
 */
dli_close( uentry )
register struct dli_line *uentry;
{

	switch ( uentry->dli_lineid.dli_substructype )
	{

		case DLI_802:
			/*
			 * disable the individual sap in the system tables
			 * this will also disable any group saps that this
			 * isap has enabled 
			 * also disable any multicast addresses this isap may have
			 */
			osi_dis_802isap(uentry->dli_if, uentry->dli_lineid.choose_addr.dli_802addr.eh_802.ssap, uentry->dli_so);

		case DLI_ETHERNET:
			/*
			 * disable possible multicast addresses
			 */
			mcast_cmd(uentry->dli_sockopt.dli_mcast, SIOCDELMULTI, uentry->dli_if);
			break;

		case DLI_POINTOPOINT:
			/*
			 * relinquish ownership of device
			 */
			uentry->dli_sockopt.dli_state = DLS_OFF;
			uentry->dli_lineid.choose_addr.dli_paddr.dev_cstate.if_ustate = IFS_USROFF;
			uentry->dli_lineid.choose_addr.dli_paddr.dev_pstate.if_next_family = uentry->dli_lineid.choose_addr.dli_paddr.dev_pstate.if_family;
			uentry->dli_lineid.choose_addr.dli_paddr.dev_pstate.if_family = AF_DLI;
			uentry->dli_lineid.choose_addr.dli_paddr.dev_pstate.if_wrstate = IFS_WRSTATE;
			uentry->dli_lineid.choose_addr.dli_paddr.dev_pstate.if_xferctl = IFS_XFERCTL;
			(*uentry->dli_if->if_ioctl)(uentry->dli_if, SIOCSTATE, &uentry->dli_lineid.choose_addr.dli_paddr.dev_pstate);
			break;

		default:
			panic("dli_close");
			break;
	}

	/*
	 * clear out entry in table
	 */
	bzero((u_char *) uentry, sizeof(struct dli_line));
	return(NULL);
}
