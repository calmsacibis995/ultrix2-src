#ifndef lint
static	char	*sccsid = "@(#)ip_output.c	1.7	(ULTRIX)	1/29/87";
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
 *	Larry Cohen - 01/28/87
 *		Add ip ouput control routine and ip options processing
 *			routine
 *									*
 *	Chet Juszczak - 03/12/86					*
 *		Add new packet fragmentation code for NFS		*
 *									*
 *	Larry Cohen  -	09/16/85					*
 * 		Add 43bsd alpha tape changes for subnet routing		*
 *									*
 ************************************************************************/

/*
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include "../h/param.h"
#include "../h/mbuf.h"
#include "../h/errno.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../h/socketvar.h"

#include "../net/if.h"
#include "../net/route.h"

#include "../netinet/in.h"
#include "../netinet/in_systm.h"
#include "../netinet/in_var.h"
#include "../netinet/in_pcb.h"
#include "../netinet/ip.h"
#include "../netinet/ip_var.h"

#ifdef vax
#include "../vax/mtpr.h"
#endif

ip_output(m, opt, ro, flags)
	struct mbuf *m;
	struct mbuf *opt;
	struct route *ro;
	int flags;
{
	register struct ip *ip = mtod(m, struct ip *);
	register struct ifnet *ifp;
	int len, hlen = sizeof (struct ip), off, error = 0;
	struct route iproute;
	struct sockaddr_in *dst;

	if (opt)				/* XXX */
		(void) m_free(opt);		/* XXX */
	/*
	 * Fill in IP header.
	 */
	if ((flags & IP_FORWARDING) == 0) {
		ip->ip_v = IPVERSION;
		ip->ip_off &= IP_DF;
		ip->ip_id = htons(ip_id++);
/*
 * CJ - SUN has the following line before this if statement.
 */
		ip->ip_hl = hlen >> 2;
	}

	/*
	 * Route packet.
	 */
	if (ro == 0) {
		ro = &iproute;
		bzero((caddr_t)ro, sizeof (*ro));
	}
	dst = (struct sockaddr_in *)&ro->ro_dst;
	if (ro->ro_rt == 0) {
		dst->sin_family = AF_INET;
		dst->sin_addr = ip->ip_dst;
		/*
		 * If routing to interface only,
		 * short circuit routing lookup.
		 */
		if (flags & IP_ROUTETOIF) {
			struct in_ifaddr *ia;
			ia = in_iaonnetof(in_netof(ip->ip_dst));
			if (ia == 0) {
				error = ENETUNREACH;
				goto bad;
			}
			ifp = ia->ia_ifp;
			goto gotif;
		}
		rtalloc(ro);
	} else if ((ro->ro_rt->rt_flags & RTF_UP) == 0) {
		/*
		 * The old route has gone away; try for a new one.
		 */
		rtfree(ro->ro_rt);
		ro->ro_rt = NULL;
		rtalloc(ro);
	}
	if (ro->ro_rt == 0 || (ifp = ro->ro_rt->rt_ifp) == 0) {
		error = ENETUNREACH;
		goto bad;
	}
	ro->ro_rt->rt_use++;
	if (ro->ro_rt->rt_flags & (RTF_GATEWAY|RTF_HOST))
		dst = (struct sockaddr_in *)&ro->ro_rt->rt_gateway;
gotif:
#ifndef notdef
	/*
	 * If source address not specified yet, use address
	 * of outgoing interface.
	 */
	if (ip->ip_src.s_addr == INADDR_ANY) {
		register struct in_ifaddr *ia;

		for (ia = in_ifaddr; ia; ia = ia->ia_next)
			if (ia->ia_ifp == ifp) {
				ip->ip_src = IA_SIN(ia)->sin_addr;
				break;
			}
	}
#endif
	/*
	 * Look for broadcast address and
	 * and verify user is allowed to send
	 * such a packet.
	 */
	if (in_broadcast(dst->sin_addr)) {
		if ((ifp->if_flags & IFF_BROADCAST) == 0) {
			error = EADDRNOTAVAIL;
			goto bad;
		}
		if ((flags & IP_ALLOWBROADCAST) == 0) {
			error = EACCES;
			goto bad;
		}
		/* don't allow broadcast messages to be fragmented */
		if (ip->ip_len > ifp->if_mtu) {
			error = EMSGSIZE;
			goto bad;
		}
	}

	/*
	 * If small enough for interface, can just send directly.
	 */
	if (ip->ip_len <= ifp->if_mtu) {
		ip->ip_len = htons((u_short)ip->ip_len);
		ip->ip_off = htons((u_short)ip->ip_off);
		ip->ip_sum = 0;
		ip->ip_sum = in_cksum(m, hlen);
		error = (*ifp->if_output)(ifp, m, (struct sockaddr *)dst);
		goto done;
	}

	/*
	 * Too large for interface; fragment if possible.
	 * Must be able to put at least 8 bytes per fragment.
	 */
	if (ip->ip_off & IP_DF) {
		error = EMSGSIZE;
		goto bad;
	}
	len = (ifp->if_mtu - hlen) &~ 7;
	if (len < 8) {
		error = EMSGSIZE;
		goto bad;
	}

	/*
	 * CJ:
	 * Call to new packet fragmentation routine.
	 * Added for NFS.
	 */
	if (hlen == sizeof (struct ip) &&
	    ip_frag2(m, ip, len, &error, ifp, dst))
		goto done;

	/*
	 * Discard IP header from logical mbuf for m_copy's sake.
	 * Loop through length of segment, make a copy of each
	 * part and output.
	 */
	m->m_len -= sizeof (struct ip);
	m->m_off += sizeof (struct ip);
	for (off = 0; off < ip->ip_len-hlen; off += len) {
		struct mbuf *mh = m_get(M_DONTWAIT, MT_HEADER);
		struct ip *mhip;

		if (mh == 0) {
			error = ENOBUFS;
			goto bad;
		}
		mh->m_off = MMAXOFF - hlen;
		mhip = mtod(mh, struct ip *);
		*mhip = *ip;
		if (hlen > sizeof (struct ip)) {
			int olen = ip_optcopy(ip, mhip, off);
			mh->m_len = sizeof (struct ip) + olen;
		} else
			mh->m_len = sizeof (struct ip);
		mhip->ip_off = off >> 3;

		/*
		 * If the packet we're fragmenting has more fragments from
  		 * other systems, propagate the MORE_FRAGMENTS flag.
		 */
		if(ip->ip_off & IP_MF) mhip->ip_off |= IP_MF;

		if (off + len >= ip->ip_len-hlen)
			len = mhip->ip_len = ip->ip_len - hlen - off;
		else {
			mhip->ip_len = len;
			mhip->ip_off |= IP_MF;
		}
		mhip->ip_len += sizeof (struct ip);
		mhip->ip_len = htons((u_short)mhip->ip_len);
		mh->m_next = m_copy(m, off, len);
		if (mh->m_next == 0) {
			(void) m_free(mh);
			error = ENOBUFS;	/* ??? */
			goto bad;
		}
		mhip->ip_off = htons((u_short)mhip->ip_off);
		mhip->ip_sum = 0;
		mhip->ip_sum = in_cksum(mh, hlen);
		if (error = (*ifp->if_output)(ifp, mh, (struct sockaddr *)dst))
			break;
	}
bad:
	m_freem(m);
done:
	if (ro == &iproute && (flags & IP_ROUTETOIF) == 0 && ro->ro_rt)
		RTFREE(ro->ro_rt);
	return (error);
}

/*
 * Copy options from ip to jp.
 * If off is 0 all options are copied
 * otherwise copy selectively.
 */
ip_optcopy(ip, jp, off)
	struct ip *ip, *jp;
	int off;
{
	register u_char *cp, *dp;
	int opt, optlen, cnt;

	cp = (u_char *)(ip + 1);
	dp = (u_char *)(jp + 1);
	cnt = (ip->ip_hl << 2) - sizeof (struct ip);
	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[0];
		if (opt == IPOPT_EOL)
			break;
		if (opt == IPOPT_NOP)
			optlen = 1;
		else
			optlen = cp[1];
		if (optlen > cnt)			/* XXX */
			optlen = cnt;			/* XXX */
		if (off == 0 || IPOPT_COPIED(opt)) {
			bcopy((caddr_t)cp, (caddr_t)dp, (unsigned)optlen);
			dp += optlen;
		}
	}
	for (optlen = dp - (u_char *)(jp+1); optlen & 0x3; optlen++)
		*dp++ = IPOPT_EOL;
	return (optlen);
}



/*
 * IP socket option processing.
 */
ip_ctloutput(op, so, level, optname, m)
	int op;
	struct socket *so;
	int level, optname;
	struct mbuf **m;
{
	int error = 0;
	struct inpcb *inp = sotoinpcb(so);

	if (level != IPPROTO_IP)
		error = EINVAL;
	else switch (op) {

	case PRCO_SETOPT:
		switch (optname) {
		case IP_OPTIONS:
			return (ip_pcbopts(&inp->inp_options, *m));

		default:
			error = EINVAL;
			break;
		}
		break;

	case PRCO_GETOPT:
		switch (optname) {
		case IP_OPTIONS:
			*m = m_get(M_WAIT, MT_SOOPTS);
			if (inp->inp_options) {
				(*m)->m_off = inp->inp_options->m_off;
				(*m)->m_len = inp->inp_options->m_len;
				bcopy(mtod(inp->inp_options, caddr_t),
				    mtod(*m, caddr_t), (unsigned)(*m)->m_len);
			} else
				(*m)->m_len = 0;
			break;
		default:
			error = EINVAL;
			break;
		}
		break;
	}
	if (op == PRCO_SETOPT)
		(void)m_free(*m);
	return (error);
}

/*
 * Set up IP options in pcb for insertion in output packets.
 * Store in mbuf with pointer in pcbopt, adding pseudo-option
 * with destination address if source routed.
 */
ip_pcbopts(pcbopt, m)
	struct mbuf **pcbopt;
	register struct mbuf *m;
{
	register cnt, optlen;
	register u_char *cp;
	u_char opt;

	/* turn off any old options */
	if (*pcbopt)
		(void)m_free(*pcbopt);
	*pcbopt = 0;
	if (m == (struct mbuf *)0 || m->m_len == 0) {
		/*
		 * Only turning off any previous options.
		 */
		if (m)
			(void)m_free(m);
		return (0);
	}

#ifndef	vax
	if (m->m_len % sizeof(long))
		goto bad;
#endif
	/*
	 * IP first-hop destination address will be stored before
	 * actual options; move other options back
	 * and clear it when none present.
	 */
#if	MAX_IPOPTLEN >= MMAXOFF - MMINOFF
	if (m->m_off + m->m_len + sizeof(struct in_addr) > MAX_IPOPTLEN)
		goto bad;
#else
	if (m->m_off + m->m_len + sizeof(struct in_addr) > MMAXOFF)
		goto bad;
#endif
	cnt = m->m_len;
	m->m_len += sizeof(struct in_addr);
	cp = mtod(m, u_char *) + sizeof(struct in_addr);
	ovbcopy(mtod(m, caddr_t), (caddr_t)cp, (unsigned)cnt);
	bzero(mtod(m, caddr_t), sizeof(struct in_addr));

	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[IPOPT_OPTVAL];
		if (opt == IPOPT_EOL)
			break;
		if (opt == IPOPT_NOP)
			optlen = 1;
		else {
			optlen = cp[IPOPT_OLEN];
			if (optlen <= IPOPT_OLEN || optlen > cnt)
				goto bad;
		}
		switch (opt) {

		default:
			break;

		case IPOPT_LSRR:
		case IPOPT_SSRR:
			/*
			 * user process specifies route as:
			 *	->A->B->C->D
			 * D must be our final destination (but we can't
			 * check that since we may not have connected yet).
			 * A is first hop destination, which doesn't appear in
			 * actual IP option, but is stored before the options.
			 */
			if (optlen < IPOPT_MINOFF - 1 + sizeof(struct in_addr))
				goto bad;
			m->m_len -= sizeof(struct in_addr);
			cnt -= sizeof(struct in_addr);
			optlen -= sizeof(struct in_addr);
			cp[IPOPT_OLEN] = optlen;
			/*
			 * Move first hop before start of options.
			 */
			bcopy((caddr_t)&cp[IPOPT_OFFSET+1], mtod(m, caddr_t),
			    sizeof(struct in_addr));
			/*
			 * Then copy rest of options back
			 * to close up the deleted entry.
			 */
			ovbcopy((caddr_t)(&cp[IPOPT_OFFSET+1] +
			    sizeof(struct in_addr)),
			    (caddr_t)&cp[IPOPT_OFFSET+1],
			    (unsigned)cnt + sizeof(struct in_addr));
			break;
		}
	}
	*pcbopt = m;
	return (0);

bad:
	(void)m_free(m);
	return (EINVAL);
}


/*
 * Attempt to fragment type 2 mbuf chain.
 * Works only if each mbuf is smaller than a packet.
 * This saves copying all the data.
 */
ip_frag2(m, ip, maxpacketlen, errorp, ifp, dst)
	register struct mbuf *m;
	register struct ip *ip;
	register int maxpacketlen;
	int *errorp;
	struct ifnet *ifp;
	struct sockaddr *dst;
{
	struct mbuf *mm;
	struct mbuf *lastm;
	register struct mbuf *mh;
	register int fraglen, fragoff, pktlen, n;
	struct ip *nextip;

	/*
	 * Check whether we can do it.
	 */
	mm = m;
	n = 0;
	while (m) {
		if (m->m_type == 2) {
			n++;
		}
		if (m->m_len + sizeof (struct ip) > maxpacketlen) {
			return (0);
		}
		m = m->m_next;
	}
	if (n == 0) {	/* higher level does type 1 chain better */
		return (0);
	}
	m = mm;
	fragoff = 0;
	while (m) {
		pktlen = 0;
		mm = m;
		/*
		 * Gather up all the mbufs that will fit in a frag.
		 */
		while (m && pktlen + m->m_len <= maxpacketlen) {
			pktlen += m->m_len;
			lastm = m;
			m = m->m_next;
		}
		fraglen = pktlen - sizeof (struct ip);
		lastm->m_next = 0;
		if (m) {
			/*
			 * There are more frags, so we prepend
			 * a copy of the ip hdr to the rest
			 * of the chain.
			 */
			MGET(mh, M_DONTWAIT, MT_HEADER);
			if (mh == 0) {
				*errorp = ENOBUFS;
				break;
			}
			mh->m_off = MMAXOFF - sizeof (struct ip) - 8;
			nextip = mtod(mh, struct ip *);
			/* copy the ip header */
			*nextip = *ip;
			mh->m_len = sizeof (struct ip);
			mh->m_next = m;
			m = mh;
			if (n = (fraglen & 7)) {
				/*
				 * IP fragments must be a multiple of
				 * 8 bytes long so we must play games.
				 */
				bcopy(mtod(lastm, caddr_t) + lastm->m_len
					- n, (caddr_t) (nextip + 1), n);
				lastm->m_len -= n;
				mh->m_len += n;
				pktlen -= n;
				fraglen -= n;
			}
			ip->ip_off = htons((u_short) ((fragoff >> 3) | IP_MF));
		} else {
			ip->ip_off = htons((u_short) (fragoff >> 3));
		}
		/*
		 * Fix up the ip header for the mm chain and send it off.
		 */
		if (ip->ip_len < pktlen) {
			ip->ip_len = htons((u_short) ip->ip_len);
			if (m) {
				m_freem(m);
				m = 0;
			}
		} else {
			ip->ip_len = htons((u_short) pktlen);
			if (m) {
				nextip->ip_len -= fraglen;
			}
		}
		ip->ip_sum = 0;
		ip->ip_sum = in_cksum(mm, sizeof (struct ip));
		if (*errorp = (*ifp->if_output)(ifp, mm, dst)) {
			break;
		}
		ip = nextip;
		fragoff += fraglen;
	}
	return (1);
}
