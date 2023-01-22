#ifndef	lint
static char *sccsid = "@(#)dli_subr.c	1.6	ULTRIX	1/15/87";
#endif	lint

/*
 * Program dli_subr.c,  Module DLI 
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
 * 1.00 22-May-1986
 *      DECnet-ULTRIX   V2.0
 *
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/protosw.h"
#include "../h/errno.h"

#include "../net/if.h"
#include "../net/if_to_proto.h"

#include "../netinet/in.h"
#include "../netinet/if_ether.h"

#include "../netdnet/dli_var.h"


extern struct dli_recv rcv;
extern struct if_isapt if_isapt[];
extern struct protosw *iftype_to_proto(), *iffamily_to_proto();

/*
 * initialize the system data structure
 * for 802.3 support
 */
osi_802init(ifp)
struct ifnet *ifp;
{
	static int k = 0;

	if(k >= MAX_BROADCSTDEV)
		return(-1);
	bzero(&if_isapt[k], sizeof(struct if_isapt));
	if_isapt[k++].ifp = ifp;
	return(NULL);
}

/*
* enable an isap in the system wide table
*/

osi_ena_802isap(ifp, so, socket)
struct ifnet *ifp;
struct sockaddr_802 *so;
struct socket *socket;
{
	int i, n, byte, error;
	u_char bit;

	if((so->eh_802.ssap & 3) || (so->eh_802.ssap >= 255) )
		return(EINVAL);
	n = so->eh_802.ssap>>2;    /* throw out low 2 bits */
	byte = n>>3;               /* byte in table = sap / 8 */
	bit = 1<<(n%8);            /* bit to set/clear = sap mod 8 */

	for(i = 0; i < MAX_BROADCSTDEV; i++)
	{
		if(if_isapt[i].ifp != ifp)
			continue;
		if(if_isapt[i].so[n] != 0)
			return(EADDRINUSE);

		if(so->svc == TYPE1)  
			if_isapt[i].svc[byte] |= bit;       /* set class 1 svc */
		else
			if_isapt[i].svc[byte] &= ~bit;      /* clear for user svc */
		if_isapt[i].so[n] = socket;
		return(NULL);
	}
	return(ENOTFND);
}


/*
* disable an isap in the system wide table
* and any gsaps that this isap has enabled
*/

osi_dis_802isap(ifp, sap, sop)
struct ifnet *ifp;
u_char sap;
struct socket *sop;
{
	int error, j, i, byte, bit;

	if( (sap & 3) || (sap >= 255) )
		return(EINVAL);

	byte = (sap >> 2)/8;
	bit = 1<< (sap >> 2)%8;
	for(i = 0; i < MAX_BROADCSTDEV; i++)
	{
		if(if_isapt[i].ifp != ifp)
			continue;
		if(if_isapt[i].so[sap>>2] == 0)
			return(EINVAL);
		if(if_isapt[i].so[sap>>2] != sop)
			return(EINVAL);
		if_isapt[i].so[sap>>2] = NULL;
		/* look for and disable any gsaps this isap has enabled */
		for(j = 0; j < NISAPS; j++)              
		{
			if(if_isapt[i].gsap[j][byte] & bit)
				if(error = osi_dis_802gsap(ifp, ((j<<2)|1), sap))
					return(error);
		}
		return(NULL);
	}
	return(ENOTFND);
}


/*
* enable gsap in the system wide table
*/

osi_ena_802gsap(ifp, gsap, sap)
struct ifnet *ifp;
int gsap, sap;
{
	int i, byte, bit;

	if( (sap >= 255) || (sap & 3))
		return(EINVAL);
	if( (gsap <= NULL) || !(gsap & 1) || (gsap > 255) )
	    return(EINVAL);
	byte = (sap >> 2) / 8; 
	bit = (sap >> 2) % 8;

	for(i = 0; i < MAX_BROADCSTDEV; i++)
	{
		if(if_isapt[i].ifp != ifp)
			continue;
		if_isapt[i].gsap[gsap>>2][byte] |= 1 << bit;
		return(NULL);
	}
	return(ENOTFND);
}


/*
* disable gsap in system wide table
*/

osi_dis_802gsap(ifp, gsap, sap)
struct ifnet *ifp;
int gsap, sap;
{
	int i, byte, bit;
	if( (sap >= 255) || (sap & 3))
		return(EINVAL);
	if( (gsap <= NULL) || !(gsap & 1) || (gsap > 255) )
	    return(EINVAL);
	byte = (sap >> 2) / 8;
	bit = (sap >> 2) % 8;

	for(i = 0; i < MAX_BROADCSTDEV; i++)
	{
		if(if_isapt[i].ifp != ifp)
			continue;
		if_isapt[i].gsap[gsap>>2][byte]  &= ~(1<< bit);
		return(NULL);
	}
	return(ENOTFND);
}


/*
* test if an isap has this gsap enabled in system wide table
*/

osi_tst_802gsap(ifp, gsap, sap)
struct ifnet *ifp;
int gsap, sap;
{
	int i, byte, bit;

	if( (sap >= 255) || (sap & 3))
		return(EINVAL);
	if( (gsap <= 0) || !(gsap & 1) || (gsap > 255) )
	    return(EINVAL);

	byte = (sap >> 2) / 8;
	bit = (sap >> 2) % 8;

	for(i = 0; i < MAX_BROADCSTDEV; i++)
	{
		if(if_isapt[i].ifp != ifp)
			continue;
		if(if_isapt[i].gsap[gsap>>2][byte]  & (1<< bit))
			return(1);
		else
			return(NULL);
	}
	return(ENOTFND);
}


/*
 * enable an 802.3 protocol (5 byte field)
 * for someone using the snap sap
 */

osi_ena_802pi(so_802, ifp)
register struct sockaddr_802 *so_802;
struct ifnet *ifp;
{
	u_char dmy_pi[5];
	extern struct dli_line dli_ltable[];
	register int i, error;
	register struct sockaddr_802 *search_eaddr;
	u_char	flags = so_802->ioctl;

	bzero(dmy_pi, 5);
	if(bcmp(so_802->eh_802.osi_pi, dmy_pi, 5) == 0) 
		return(EINVAL);

	/*
	 * Only one control flag should be set.
	 */
	switch (flags)
	{
		case DLI_EXCLUSIVE:
		case DLI_NORMAL:
		case DLI_DEFAULT:
			break;
		default:
			return(EINVAL);
			break;
	}

	/*
	 * Make sure address structure is unique per device.
	 */
	for ( i = 0; i < DLI_MAXLINE; i++ )
	{
		if (dli_ltable[i].dli_lineid.dli_substructype != DLI_802 ||
			dli_ltable[i].dli_so == NULL ||
			ifp != dli_ltable[i].dli_if )
		{
			continue;
		}
		search_eaddr = &dli_ltable[i].dli_lineid.choose_addr.dli_802addr;
		error = NULL;
		switch (search_eaddr->ioctl)
		{
			case NULL:
				break;

			case DLI_EXCLUSIVE:
				error = (bcmp(search_eaddr->eh_802.osi_pi, so_802->eh_802.osi_pi, 5) == NULL) ? EADDRINUSE : NULL;
				break;

			case DLI_NORMAL:
				switch (flags)
				{
					case DLI_EXCLUSIVE:
						error = (bcmp(search_eaddr->eh_802.osi_pi, so_802->eh_802.osi_pi, 5) == NULL) ? EADDRINUSE : NULL;
						break;

					case DLI_NORMAL:
						if(bcmp(search_eaddr->eh_802.osi_pi, so_802->eh_802.osi_pi, 5) == NULL)
						{
							error = match_targets(search_eaddr->eh_802.dst, so_802->eh_802.dst);
						}
						else 
						{
							error = NULL;
						}
						break;
					default:
						break;
				}
				break;

			case DLI_DEFAULT:
				switch (flags)
				{
					case DLI_EXCLUSIVE:
					case DLI_DEFAULT:
						error = (bcmp(search_eaddr->eh_802.osi_pi, so_802->eh_802.osi_pi, 5) == NULL) ? EADDRINUSE : NULL;
						break;

					default:
						break;
				}
				break;


			default:
				panic( "dli_bind: osi_ena_802pi:" );
				break;
		}
		if (error != NULL)
		{
			return(error);
		}
	}
	return(NULL);

}


/* 
 * build an 802 header for transmission
 */

struct mbuf *
osi_buildhdr(dst_addr)
struct sockaddr_dl *dst_addr;
{
	struct mbuf *temp, *m = NULL;
	u_char *cp;

	MGET(m, M_DONTWAIT, MT_DATA);
	if ( m )
	{
		cp = mtod(m,u_char *);
		PUT8B(cp, dst_addr->choose_addr.dli_802addr.eh_802.dsap);
		PUT8B(cp, dst_addr->choose_addr.dli_802addr.eh_802.ssap);
		if((dst_addr->choose_addr.dli_802addr.eh_802.ctl.U_fmt & 3) == 3)
		{
			PUT8B(cp, dst_addr->choose_addr.dli_802addr.eh_802.ctl.U_fmt);
			if(dst_addr->choose_addr.dli_802addr.eh_802.ssap == SNAP_SAP)
			{
				PUT8B(cp, dst_addr->choose_addr.dli_802addr.eh_802.osi_pi[0]);
				PUT8B(cp, dst_addr->choose_addr.dli_802addr.eh_802.osi_pi[1]);
				PUT8B(cp, dst_addr->choose_addr.dli_802addr.eh_802.osi_pi[2]);
				PUT8B(cp, dst_addr->choose_addr.dli_802addr.eh_802.osi_pi[3]);
				PUT8B(cp, dst_addr->choose_addr.dli_802addr.eh_802.osi_pi[4]);
				m->m_len = 8;
			}
			else
				m->m_len = 3;
		}
		else
		{
			PUT16B(cp, dst_addr->choose_addr.dli_802addr.eh_802.ctl.I_S_fmt);
			m->m_len = 4;
		}
	}
	return(m);
}


/*
 * handle 802 incoming packets
 * otherwise drop packet 
 * otherwise drop packet 
 * pull any header information from the 2nd mbuf to
 * guarantee that only data is there
 *
 * input packet as a chain of mbufs, first mbuf has garbage
 * 2nd mbuf will have the rest of the 802 header 
 * return nothing
 */

osi_802input(m)
struct mbuf *m;
{
	u_char *cp, bit; 
	int len, cnt, byte, i, j, k;

	struct osi_802hdr *eh;
	struct ifnet *ifp;
	struct {              /* used for gsaps to make copies of packet */
		int sap;
		struct mbuf *m;
	} m_ar[NISAPS];

	eh = &rcv.rcv_hdr.osi_header;
	cp = mtod(m->m_next, u_char *);   /* remove up to ctl fld */
	eh->dsap = *cp++;
	eh->ssap = *cp++;
	m_adj(m->m_next, 2);     
	eh->len -= 2;

	for(i = 0; i < MAX_BROADCSTDEV; i++)   /* look for matching device */
	{
		if(if_isapt[i].ifp != rcv.rcv_ifp)
			continue;

		/*
		 * special cases here
		 */
		if(eh->dsap == NULL || eh->dsap == SNAP_SAP)
		{
			if(((eh->ctl.U_fmt = (u_char)*cp++)& 3) != 3)
				goto dropit;
		    m_adj(m->m_next, 1);      /* pull ctrl field */
			--eh->len;
			if(eh->ctl.U_fmt == UI_NPCMD)  /* save pi for snap */
			{
				if(eh->dsap == NULL)
					goto dropit;
				for(i = 0; i < 5; i++)
					eh->osi_pi[i] = *cp++;
				m_adj(m->m_next, 5);      /* pull protocol */
				eh->len -= 5;
			}

		    if( (len = mbuf_len(m->m_next)) > eh->len) 
				m_adj(m->m_next, (eh->len - len)); /* adjust for padding */
			else if(len < eh->len)
				goto dropit;
		    osi_proc_ctl(m,i);
			return;
		}
/*
 * handle isaps 
 */
		else if((eh->dsap & 1) == NULL &&  
		if_isapt[i].so[(eh->dsap>>2)] != NULL) 
		{
			bit = 1 << ( (eh->dsap>>2) % 8);
/*
 * for class 1 service get ctl field
 */
			if(if_isapt[i].svc[(eh->dsap>>5)] & bit)
			{ 
			    if( (eh->ctl.U_fmt = (u_char)*cp) & 3 != 3)
					goto dropit;
			    m_adj(m->m_next, 1);    
				--eh->len;
			    if( (len = mbuf_len(m->m_next)) > eh->len) 
					m_adj(m->m_next, (eh->len - len)); 
				else if(len < eh->len)
					goto dropit;
			    osi_proc_ctl(m,i);
				return;
			}
/*
 * must be users supplied service, just pass it all up
 */
			else
			{
			    /* leave ctl in with data */
			    switch(*cp & 3)
			    {
					case 0:        /* information */
					case 1:        /* supervisory */
					case 2:        /* information */
						eh->ctl.I_S_fmt = (u_short)*cp;
						break;
					case 3:        /* unnumbered */
						eh->ctl.U_fmt = (u_char)*cp;
						break;
					default:
						goto dropit;
			    }
			   
			    if( (len = mbuf_len(m->m_next)) > eh->len) 
					m_adj(m->m_next, (eh->len - len)); /* adjust for padding */
				else if(len < eh->len)
					goto dropit;
			    found_user(m, if_isapt[i].so[eh->dsap>>2], DLI_802);
				return;
			}
		}
/*
* The only thing left should be gsaps in user supplied mode.
* Try and get a copy of the packet for each sap that has the 
* gsap enabled. If any fail, free any copies and the original 
*/
		else if( (eh->dsap & 1) && 
		( ffs(*(long *) &if_isapt[i].gsap[(eh->dsap>>2)][0]) || 
		ffs(*(long *) &if_isapt[i].gsap[(eh->dsap>>2)][4]) ))
		{
			if( (len = mbuf_len(m->m_next)) > eh->len) 
				m_adj(m->m_next, (eh->len - len)); /* adjust for len of data */
			else if(len < eh->len)
				goto dropit;
			/* search gsap table for any isaps with this gsap enabled */
			bit = 0;
			cnt = 0;
			for(j = 0; j < 8; j++)
			{
				byte = if_isapt[i].gsap[eh->dsap>>2][j];
/* 
 * 
 * if bit is set, bit pos = isap with this gsap enabled
 *
 */
				for(k = 0; k < 8; k++)
				{
					if(byte & 1)
					{
						if(!(m_ar[cnt].m = m_copy(m, 0, M_COPYALL))){
							while(--cnt >= 0)
								m_freem(m_ar[cnt].m);
							goto dropit;                 /* drop original */
						}
						m_ar[cnt++].sap = bit;
					}
					++bit;
					byte >>= 1;
				}
			}

		    m_freem(m);             /* done with original */
		    while(--cnt >= 0) {
				found_user(m_ar[cnt].m, if_isapt[i].so[m_ar[cnt].sap], DLI_802);
		    }
			return;
		}
		goto dropit;
    }

dropit:
    if(m)
		m_freem(m);
    return;
}


/*
 * process the control field on incoming packets
 * for class 1 service only
 * the packet header must have been adjusted by now
 */

osi_proc_ctl(m,i)
struct mbuf *m;
int i;
{
	u_char dsap;
	int len;

    dsap = rcv.rcv_hdr.osi_header.dsap;

	/* only ctl allowed for class 1 */

    switch(rcv.rcv_hdr.osi_header.ctl.U_fmt)   
    {
	    case XID_PCMD:
	    case XID_NPCMD:
	    case TEST_PCMD:
	    case TEST_NPCMD:
		    if( rcv.rcv_hdr.osi_header.ssap & 1) /* got a response */
		    {
				switch(dsap)
				{
					case NULL:   /* nobody to pass response to */
					case SNAP_SAP:
						if(m) m_freem(m);
						return;
					break;
					default:
						found_user(m, if_isapt[i].so[dsap>>2], DLI_802);
						return;
					break;
				}
		    }
		    else                     /* command, gotta respond for these guys */
		    {
			    osi_rspndr(m);
			    return;
		    }
		    break;
	    case UI_NPCMD:                /* only type of ui */
			switch (dsap)
			{
				case NULL:
					if(m) m_freem(m);
					return;
				break;
				case SNAP_SAP:
					forward_to_user(m);
					return;
				break;
				default:              /* got somebody */
					found_user(m, if_isapt[i].so[dsap>>2], DLI_802);
					return;
				break;
			}
			break;
		default:                      /* throw it out */
			if(m) m_freem(m);
			return;
		break;
	}  /***** end switch *****/

	if(m) m_freem(m);                 /* safety net */
	return;
}


/* 
* respond to xid or test packet for class 1
* service, for the null sap or the snap sap only
* 
* input	packet as a chain of mbuf's, 1st mbuf garbage
* return nothing, attempt is made to tx packet (no guarantees)
*/

osi_rspndr(m)
struct mbuf *m;
{
	u_char *cp;
	u_short len;
	struct osi_802hdr *eh = &rcv.rcv_hdr.osi_header;
	struct sockaddr_dl out_addr; 
	struct mbuf *m0, *temp, *osi_buildhdr();
	struct ifnet *ifp = rcv.rcv_ifp;

	/* got to do this for output routine */
	bzero(&out_addr, sizeof(struct sockaddr_dl));
	out_addr.dli_family = AF_DLI;
	out_addr.dli_substructype = DLI_802;
	out_addr.choose_addr.dli_802addr.svc = TYPE1;
	out_addr.dli_device.dli_devnumber = ifp->if_unit;
	len = strlen(ifp->if_name);
	bcopy(ifp->if_name, out_addr.dli_device.dli_devname, len);
	bcopy(rcv.rcv_hdr.osi_header.src, out_addr.choose_addr.dli_802addr.eh_802.dst, DLI_EADDRSIZE);

	/* return it with the response bit set */
	out_addr.choose_addr.dli_802addr.eh_802.ssap = eh->dsap | (u_char)1; 
	out_addr.choose_addr.dli_802addr.eh_802.dsap = eh->ssap;

	switch(eh->ctl.U_fmt & 0xEF)
	{
		case XID:
			if(eh->len != 3)
				goto bad;
			cp = mtod(m, u_char *);
			PUT8B(cp, 0x81);              
			PUT8B(cp, 0x01);
			PUT8B(cp, NULL);
			m->m_len = 3;
			m_freem(m->m_next);          /* don't need the rest */
			m->m_next = NULL;
			break;
		case TEST:   /* just turn it around, they may have sent data */
			m = m_free(m);         /* 1st mbuf has garbage */
			break;
		default:
			goto bad;
	}

	/*
	 * just have to return the control field
	 */
	out_addr.choose_addr.dli_802addr.eh_802.ctl.U_fmt = eh->ctl.U_fmt;
	if(m0 = osi_buildhdr(&out_addr))
	{
		m0->m_next = m;
		(*ifp->if_output)(ifp, m0, &out_addr); 
		return;
	}

bad:
	if(m)
	    m_freem(m);
	return;
}
