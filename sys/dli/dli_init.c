#ifndef	lint
static char *sccsid = "@(#)dli_init.c	1.8	ULTRIX	1/9/87";
#endif	lint

/*
 * Program dli_init.c,  Module DLI 
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
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kernel.h"

#include "../net/if.h"

#include "../netinet/in.h"
#include "../netinet/if_ether.h"

#include "../netdnet/dli_var.h"

#define DLI_IFQ_MAXLEN	10	/* maximum number of packets placed on DLI Rx Q */

extern struct ifqueue dli_intrq;
extern struct ether_header no_enet_header;
extern struct dli_line dli_ltable[];
extern struct dli_timers lback_timers[];

extern u_char sysid_msg[];
extern struct ether_pa *sysid_haddr_p;
extern struct sockaddr_dl sysid_dst;
extern struct dli_sysid_to sysid_to[];
extern u_short nqna;
extern u_char sysid_mcast[];

extern struct if_isapt if_isapt[];




/*
 *		d l i _ i n i t
 *
 * This routine is called during the system boot sequence to initialise the
 * DLI protocol module.
 *
 * Outputs:		None.
 *
 * Inputs:		None.  
 *
 * Version History:
 *	3/7/85	- ejf - account for MOP loopback capability
 */
dli_init()
{
	int i, k = 0, n;

	register struct ifnet *ifp = ifnet;
	struct ifreq dreq;

	dli_intrq.ifq_maxlen = DLI_IFQ_MAXLEN;

	/*
	 * init local variables
	 */
	bzero(&no_enet_header, sizeof(no_enet_header));

	/*
	 * Clear out array containing structures for each line.
	 */
	for( i = 0; i < DLI_MAXLINE; i++ )
	{
		bzero((u_char *) &dli_ltable[i], sizeof(struct dli_line));
	}

	/*
	 * Disable loopback timer data base.
	 */
	for( i = 0; i < DLI_MAX_LBTIMR; i++ )
	{
		lback_timers[i].tval = 0;
	}

	/*
	 * enable MOP Loopback Multicast Address on each device.
	 */
	bzero(dreq.ifr_addr.sa_data, DLI_EADDRSIZE);
	dreq.ifr_addr.sa_data[0] = 0xcf;
	i = 0;

	while ( ifp )
	{
		if ( (ifp->if_flags & (IFF_BROADCAST | IFF_DYNPROTO)) == (IFF_BROADCAST | IFF_DYNPROTO))
		{
			char *cp;

			(*ifp->if_ioctl)(ifp, SIOCADDMULTI, &dreq);
			/*
			 * enable 802.3 data structures
			 * 1 per ethernet device
			 */
			if( osi_802init(ifp) < 0)
				printf("dli_init: osi_802init: device %d isap struct init failed\n", ifp->if_unit);
			cp = ifp->if_name;
			if( (*cp++ == 'q') && (*cp == 'e') && (i <= MAXQNAS) ) 
			{
				if(qe_init_sysidto(ifp) < 0)
					printf("dli_init: qe_init_sysidto: timeout struct %d init failed\n", ifp->if_unit);
				else
					i++;
			}
		}
		ifp = ifp->if_next;
	}
	if(nqna = i)                 /* only build sysid msg if uVAX */
		qe_init_sysidmsg();
}



/*
 *
 * Build sysid messasge for uVAX's
 * hardware should have done this
 * 
 */

qe_init_sysidmsg()
{
	u_char *msgp = sysid_msg;
	int i;

	/* outgoing information */
	sysid_dst.dli_family = AF_DLI;           
	sysid_dst.dli_substructype = DLI_ETHERNET;
	bcopy("qe", sysid_dst.dli_device.dli_devname, 2);
	sysid_dst.choose_addr.dli_eaddr.dli_protype = SYSIDPROTO;
	*(struct ether_pa *)sysid_dst.choose_addr.dli_eaddr.dli_target = *(struct ether_pa *)sysid_mcast; 

/* initialize sysid message, have to do it this way to avoid padding */
	PUT16B(msgp, SYSID_MSGL);           /* length header */
	PUT8B(msgp, SYSID_CODE);            /* sysid code */
	PUT8B(msgp, NULL);                  /* reserved */
	PUT16B(msgp, NULL);                 /* receipt number */
	PUT16B(msgp, MAINTV);               /* info type, maint ver */
	PUT16B(msgp, (3*sizeof(u_char)));   /* length of info */
	PUT8B(msgp, VER);                   /* version */
	PUT8B(msgp, ECO);                   /* eco */
	PUT8B(msgp, USER_ECO);              /* user eco */
	PUT16B(msgp, FUNCTIONS);            /* info type, functions */
	PUT16B(msgp, sizeof(u_short));      /* length of info */
	PUT16B(msgp, (MAINTV | FUNCTIONS | HADDR | COMDEV));
	PUT16B(msgp, HADDR);                /* info type, hardware addr */
	PUT16B(msgp, sizeof(struct ether_pa)); /* length of info */
	sysid_haddr_p = (struct ether_pa *)msgp;/* save ptr to h_addr for later */
	for(i = 0; i < sizeof(struct ether_pa); i++)
		PUT8B(msgp, NULL);              /* clear it out and move ptr */
	PUT16B(msgp, COMDEV);               /* info type, device */
	PUT16B(msgp, sizeof(u_char));       /* length of info */
	PUT8B(msgp, QNA);                   /* device is a qna */

	return;
}





/*
 *
 * Initialize the sysid time out structures
 *
 */

qe_init_sysidto(ifp)
struct ifnet *ifp;
{
    static int i = 0;
    int len, n = 0, c, j; 
    short tmp, unit;
    char num[6];

	if(i >= MAXQNAS)
		return(-1);
    sysid_to[i].ifp = ifp;
    unit = ifp->if_unit;
    do {        /* convert short to ascii (backwards) */
	    num[n++] = unit % 10 + '0';
    } while (( unit /= 10) > 0);
    num[n] = '\0';
    len = n + 1;
    /* put it in the right order */
    for(j = 0, n -= 1; j < n; j++, n--)
    {
	    c = num[j];
	    num[j] = num[n];
	    num[n] = c;
    }
    bcopy("qe", sysid_to[i].dev.ifr_name, 2);
    bcopy(num, &sysid_to[i].dev.ifr_name[2], len);
    (*ifp->if_ioctl)(ifp, SIOCRPHYSADDR, (caddr_t)&sysid_to[i].dev);
    tmp = *(short *)&sysid_to[i].dev.current_pa[4];
    sysid_to[i].tr =  sysid_to[i].to = (((tmp/4) + 150000) /500) + 1200;
	i++;
	return(NULL);
}
