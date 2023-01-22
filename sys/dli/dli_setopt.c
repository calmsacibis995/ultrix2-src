#ifndef	lint
static char *sccsid = "@(#)dli_setopt.c	1.8	ULTRIX	1/9/87";
#endif	lint

/*
 * Program dli_setopt.c,  Module DLI 
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
 *		d l i _ s e t o p t
 *
 * Process a DLI request.
 *
 * Returns:		Error code if error, otherwise NULL.
 *
 * Inputs:
 *	uentry		= Pointer to the user's line table entry for this request.
 *	optbuf		= Buffer containg option.
 *	optlen		= Length of data in option buffer.
 *	optnam		= Name of option.
 */
dli_setopt( uentry,optbuf,optlen,optnam ) 
register struct dli_line *uentry;
register u_char *optbuf;
short optlen;
int optnam;
{
	u_short i, nmulti;
	register int error;
	register struct dli_sopt *option;
	extern struct dli_line dli_ltable[];

	option = &uentry->dli_sockopt;

	/*
	 * Make sure a device has been bound to, otherwise
	 * operation would not make sense.
	 */
	if ( ! uentry->dli_if )
	{
		return(ENODEV);
	}
	
	/*
	 * store option only if it is valid.
	 */
	switch (optnam)
	{
		case DLI_SET802CTL:
			/*
			 * store 802.3 control field
			 * for isaps with class 1 service, validate it first
			 */
			if(uentry->dli_lineid.choose_addr.dli_802addr.svc == TYPE1)
			{
				if(optlen != 1)    /* only 1 byte ctl field allowed */
					return(EINVAL);
				if(uentry->dli_lineid.choose_addr.dli_802addr.eh_802.ssap == SNAP_SAP && ((u_char)optbuf[0] != UI_NPCMD) )
						return(EINVAL);
				switch((u_char)optbuf[0])
				{
					case UI_NPCMD:
					case TEST_PCMD:
					case TEST_NPCMD:
					case XID_PCMD:
					case XID_NPCMD:
						uentry->dli_lineid.choose_addr.dli_802addr.eh_802.ctl.U_fmt = (u_char)optbuf[0];
						break;
					default:
						return(EINVAL);
				}
			}
			else    /* user service here */
			{
				if(optlen == 1) 
					uentry->dli_lineid.choose_addr.dli_802addr.eh_802.ctl.U_fmt = (u_char)optbuf[0];
				else if(optlen == 2) 
					uentry->dli_lineid.choose_addr.dli_802addr.eh_802.ctl.I_S_fmt = (u_short)optbuf;
				else return(EINVAL);
			}
			break;

		case DLI_ENAGSAP:
			/* 
			 *enable a list of group saps for this isap
			 */
			if(uentry->dli_lineid.choose_addr.dli_802addr.svc == TYPE1)
				return(ENOPROTOOPT);
			/* find the matching line table entry */
			for(i = 0; i < optlen; i++)
			{
				if((u_char)optbuf[i] < 5 || (u_char)optbuf[i] > 255 || 
				!((u_char)optbuf[i] & 1))
					return(EINVAL);
				if(error = osi_ena_802gsap(uentry->dli_if, (u_char)optbuf[i], uentry->dli_lineid.choose_addr.dli_802addr.eh_802.ssap))
					return(error);
			}
			break;

		case DLI_DISGSAP:
			/*
			 * disable a list of group saps that 
			 * that this isap has enabled
			 */
			for(i = 0; i < optlen; i++)
			{
				if( (u_char)optbuf[i] <= 5 || (u_char)optbuf[i] > 255 || 
				!((u_char)optbuf[i] & 1))
					return(EINVAL);
				if(error = osi_dis_802gsap(uentry->dli_if, (u_char)optbuf[i], uentry->dli_so))
					return(error);
			}
			break;

		case DLI_STATE:
			return(ENOPROTOOPT);
			break;

		case DLI_INTERNALOOP:
			if ( optlen != 1 || (*optbuf != DLP_IOFF && *optbuf != DLP_ION))
			{
				return(ENOPROTOOPT);
			}
			option->dli_iloop = *optbuf;
			break;


		/*
		 * Set multicast address(es). Verify correct number and
		 * validity of addresses.
		 */
		case DLI_MULTICAST:
			if ( (optlen % MCAST_SIZE) || ((nmulti = optlen/MCAST_SIZE) > MCAST_MAXNUM) )
			{
				return(ENOPROTOOPT);
			}
			for( i = 0; i < nmulti; i++ )
			{
				if( ! (optbuf[i*MCAST_SIZE] & 1) )
				{
					return(ENOPROTOOPT);
				}
			}
			if ( error = mcast_cmd(option->dli_mcast, SIOCDELMULTI, uentry->dli_if) )
			{
				return(error);
			}
			bcopy(optbuf, option->dli_mcast, optlen);
			if ( (error = mcast_cmd(option->dli_mcast, SIOCADDMULTI, uentry->dli_if)) )
			{
				mcast_cmd(option->dli_mcast, SIOCDELMULTI, uentry->dli_if);
			}
			return(error);
			break;

		default:
			return(ENOPROTOOPT);
			break;
	}
	return(NULL);
}






/*
 *		m c a s t _ c m d
 *
 * Add/delete multicast command.  This routine keeps an
 * accurate record of the multicast addresses it has enabled.
 *
 * Returns:		Status code from driver.
 *
 * Inputs:
 *	mcast_buf	= Buffer containing multicast address(es).
 *	cmd		= Command given to driver.
 *	ifp		= Pointer to ifnet structure to access driver.
 */
mcast_cmd( mcast_buf,cmd,ifp ) 
register u_char *mcast_buf;
register struct ifnet *ifp;
int cmd;
{
	struct ifreq dreq;
	register int i;
	register int error;


	for( i = 0; i < MCAST_ASIZE; i += MCAST_SIZE)
	{
		if ( *(mcast_buf+i) & 1 )
		{
			*(struct ether_pa *) dreq.ifr_addr.sa_data = *(struct ether_pa *) (mcast_buf+i);
			if ( (error = (*ifp->if_ioctl)(ifp, cmd, &dreq)) )
			{
				if ( cmd == SIOCADDMULTI )
				{
					bzero(mcast_buf+i, (MCAST_ASIZE - i));
				}
				else
				{
					bcopy(mcast_buf+i, mcast_buf, (MCAST_ASIZE - i));
					bzero(mcast_buf+(MCAST_ASIZE-i), i);
				}
				return(error);

			}
		}
	}
	if ( cmd == SIOCDELMULTI )
	{
		bzero(mcast_buf, MCAST_ASIZE);
	}
	return(NULL);
}



/*
 *		c h a n g e _ s t a t e
 *
 *
 * Returns:		Status code from driver.
 *
 * Inputs:
 *	ifp		= Pointer to ifnet structure to access driver.
 *	new_state	= User requested state (off, on, mop)
 *	state_var	= pointer to current state information
 *	old_state	= if addr, store current state information 
 */
change_dlistate( ifp, new_state, state_var, old_state )
register struct ifnet *ifp;
register struct ifstate *new_state;
register struct ifstate *state_var;
register struct ifstate *old_state;
{

	int status = 0;

	if ( old_state )
	{
		old_state->if_family = ifp->if_addr.sa_family;
		old_state->if_rdstate = IFS_RDSTATE;
		old_state->if_wrstate = ~IFS_WRSTATE;
		old_state->if_xferctl = ~IFS_XFERCTL;
		if (  status = (*ifp->if_ioctl)(ifp, SIOCSTATE, old_state) )
		{
			return(status);
		}
	}

	new_state->if_family = AF_DLI;
	if ( new_state->if_ustate == IFS_USRON )
	{
		new_state->if_nomuxhdr = IFS_NOMUXHDR;
	}
	new_state->if_wrstate = IFS_WRSTATE;
	new_state->if_rdstate = IFS_RDSTATE;
	if (  ! (status = (*ifp->if_ioctl)(ifp, SIOCSTATE, new_state)) )
	{
		*state_var = *new_state;
	}
	return(status);
}
/*
change_dlistate( ifp, new_state, state_var )
register struct ifnet *ifp;
register u_char new_state;
register u_char *state_var;
{

	u_char temp_state;
	int status = 0;
	struct ifstate ptop_state;

	bzero(&ptop_state, sizeof(ptop_state));
	ptop_state.if_family = AF_DLI;
	ptop_state.if_mode = IFS_DDCMPFDX;
	if ( new_state == DLS_ON )
	{
		ptop_state.if_ustate = IFS_USRON;
		ptop_state.if_nomuxhdr = IFS_NOMUXHDR;
	}
	else
	{
		ptop_state.if_ustate = IFS_USROFF;
	}
	temp_state = *state_var;
	*state_var = new_state;
	if (  (status = (*ifp->if_ioctl)(ifp, SIOCSTATE, &ptop_state)) )
	{
		*state_var = temp_state;
	}
	return(status);
}
*/
