#ifndef lint
static	char	*sccsid = "@(#)input.c	1.3	(ULTRIX)	9/23/85";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/************************************************************************
 *			Modification History				*
 *									*
 *	Larry Cohen  -	09/16/85					*
 * 		Update to 43bsd alpha tape for subnet routing, and	*
 *			handle point to point links correctly		*
 *									*
 ************************************************************************/


/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
static char sccsid[] = "@(#)input.c	5.1 (Berkeley) 6/4/85";
*/

/*
 * Routing Table Management Daemon
 */
#include "defs.h"

/*
 * Process a newly received packet.
 */
rip_input(from, size)
	struct sockaddr *from;
	int size;
{
	register struct rt_entry *rt;
	register struct netinfo *n;
	register struct interface *ifp;
	struct interface *if_ifwithdstaddr();
	int newsize;
	register struct afswitch *afp;

	ifp = 0;
	TRACE_INPUT(ifp, from, size);
	if (from->sa_family >= AF_MAX)
		return;
	afp = &afswitch[from->sa_family];
	switch (msg->rip_cmd) {

	case RIPCMD_REQUEST:
		newsize = 0;
		size -= 4 * sizeof (char);
		n = msg->rip_nets;
		while (size > 0) {
			if (size < sizeof (struct netinfo))
				break;
			size -= sizeof (struct netinfo);

			if (msg->rip_vers > 0) {
				n->rip_dst.sa_family =
					ntohs(n->rip_dst.sa_family);
				n->rip_metric = ntohl(n->rip_metric);
			}
			/* 
			 * A single entry with sa_family == AF_UNSPEC and
			 * metric ``infinity'' means ``all routes''.
			 */
			if (n->rip_dst.sa_family == AF_UNSPEC &&
			    n->rip_metric == HOPCNT_INFINITY && size == 0) {
				supply(from, 0, ifp);
				return;
			}
			rt = rtlookup(&n->rip_dst);
			n->rip_metric = rt == 0 ? HOPCNT_INFINITY :
				min(rt->rt_metric+1, HOPCNT_INFINITY);
			if (msg->rip_vers > 0) {
				n->rip_dst.sa_family =
					htons(n->rip_dst.sa_family);
				n->rip_metric = htonl(n->rip_metric);
			}
			n++, newsize += sizeof (struct netinfo);
		}
		if (newsize > 0) {
			msg->rip_cmd = RIPCMD_RESPONSE;
			newsize += sizeof (int);
			(*afp->af_output)(s, 0, from, newsize);
		}
		return;

	case RIPCMD_TRACEON:
	case RIPCMD_TRACEOFF:
		/* verify message came from a privileged port */
		if ((*afp->af_portcheck)(from) == 0)
			return;
		packet[size] = '\0';
		if (msg->rip_cmd == RIPCMD_TRACEON)
			traceon(msg->rip_tracefile);
		else
			traceoff();
		return;

	case RIPCMD_RESPONSE:
		/* verify message came from a router */
		if ((*afp->af_portmatch)(from) == 0)
			return;
		(*afp->af_canon)(from);
		/* are we talking to ourselves? */
		ifp = if_ifwithaddr(from);
		if (ifp) {
			rt = rtfind(from);
			if (rt == 0 || (rt->rt_state & RTS_INTERFACE) == 0)
				addrouteforif(ifp);
			else
				rt->rt_timer = 0;
			return;
		}
		/*
		 * Update timer for interface on which the packet arrived.
		 * If from other end of a point-to-point link that isn't
		 * in the routing tables, (re-)add the route.
		 */
		if ((rt = rtfind(from)) && (rt->rt_state & RTS_INTERFACE))
			rt->rt_timer = 0;
		else if (ifp = if_ifwithdstaddr(from))
			addrouteforif(ifp);
		size -= 4 * sizeof (char);
		n = msg->rip_nets;
		for (; size > 0; size -= sizeof (struct netinfo), n++) {
			if (size < sizeof (struct netinfo))
				break;
			if (msg->rip_vers > 0) {
				n->rip_dst.sa_family =
					ntohs(n->rip_dst.sa_family);
				n->rip_metric = ntohl(n->rip_metric);
			}
			if ((unsigned) n->rip_metric > HOPCNT_INFINITY)
				continue;
			if (n->rip_dst.sa_family >= AF_MAX)
				continue;
			afp = &afswitch[n->rip_dst.sa_family];
			if (((*afp->af_checkhost)(&n->rip_dst)) == 0)
				continue;
			rt = rtlookup(&n->rip_dst);
			if (rt == 0) {
				if (n->rip_metric < HOPCNT_INFINITY)
				    rtadd(&n->rip_dst, from, n->rip_metric, 0);
				continue;
			}

			/*
			 * Update if from gateway and different,
			 * shorter, or getting stale and equivalent.
			 */
			if (equal(from, &rt->rt_router)) {
				if (n->rip_metric == HOPCNT_INFINITY) {
					rtdelete(rt);
					continue;
				}
				if (n->rip_metric != rt->rt_metric)
					rtchange(rt, from, n->rip_metric);
				rt->rt_timer = 0;
			} else if ((unsigned) (n->rip_metric) < rt->rt_metric ||
			    (rt->rt_timer > (EXPIRE_TIME/2) &&
			    rt->rt_metric == n->rip_metric)) {
				rtchange(rt, from, n->rip_metric);
				rt->rt_timer = 0;
			}
		}
		return;
	}
}
