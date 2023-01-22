#ifndef lint
static	char	*sccsid = "@(#)if.c	1.4	(ULTRIX)	10/23/86";
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
 *      U. Sinkewicz - 10/23/86					        *
 *		Added case for AF_BSC.					*
 *
 *	23-Oct-1986 - Marc Teitelbaum
 *		Fixed the default case of an unknown address family.
 *									*
 ************************************************************************/


/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
static char sccsid[] = "@(#)if.c	5.2 (Berkeley) 6/15/85";
*/

#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#ifdef NS
#include <netns/ns.h>
#endif

#include <stdio.h>

extern	int kmem;
extern	int tflag;
extern	int nflag;
extern	char *interface;
extern	int unit;
extern	char *routename(), *netname();

/*
 * Print a description of the network interfaces.
 */
intpr(interval, ifnetaddr)
	int interval;
	off_t ifnetaddr;
{
	struct ifnet ifnet;
	union {
		struct ifaddr ifa;
		struct in_ifaddr in;
	} ifaddr;
	off_t ifaddraddr;
	char name[16];

	if (ifnetaddr == 0) {
		printf("ifnet: symbol not defined\n");
		return;
	}
	if (interval) {
		sidewaysintpr(interval, ifnetaddr);
		return;
	}
	klseek(kmem, ifnetaddr, 0);
	read(kmem, &ifnetaddr, sizeof ifnetaddr);
	printf("%-5.5s %-5.5s %-10.10s  %-12.12s %-7.7s %-5.5s %-7.7s %-5.5s",
		"Name", "Mtu", "Network", "Address", "Ipkts", "Ierrs",
		"Opkts", "Oerrs");
	printf(" %-6.6s", "Collis");
	if (tflag)
		printf(" %-6.6s", "Timer");
	putchar('\n');
	ifaddraddr = 0;
	while (ifnetaddr || ifaddraddr) {
		struct sockaddr_in *sin;
		register char *cp;
		int n;
		char *index();
		struct in_addr in, inet_makeaddr();

		if (ifaddraddr == 0) {
			klseek(kmem, ifnetaddr, 0);
			read(kmem, &ifnet, sizeof ifnet);
			klseek(kmem, (off_t)ifnet.if_name, 0);
			read(kmem, name, 16);
			name[15] = '\0';
			ifnetaddr = (off_t) ifnet.if_next;
			if (interface != 0 &&
			    (strcmp(name, interface) != 0 || unit != ifnet.if_unit))
				continue;
			cp = index(name, '\0');
			*cp++ = ifnet.if_unit + '0';
			if ((ifnet.if_flags&IFF_UP) == 0)
				*cp++ = '*';
			*cp = '\0';
			ifaddraddr = (off_t)ifnet.if_addrlist;
		}
		printf("%-5.5s %-5d ", name, ifnet.if_mtu);
		if (ifaddraddr == 0) {
			printf("%-10.10s  ", "none");
			printf("%-12.12s ", "none");
		} else {
			klseek(kmem, ifaddraddr, 0);
			read(kmem, &ifaddr, sizeof ifaddr);
			ifaddraddr = (off_t)ifaddr.ifa.ifa_next;
			switch (ifaddr.ifa.ifa_addr.sa_family) {
			case AF_UNSPEC:
				printf("%-10.10s  ", "none");
				printf("%-12.12s ", "none");
				break;
			case AF_INET:
				sin = (struct sockaddr_in *)&ifaddr.in.ia_addr;
#ifdef notdef
				/* can't use inet_makeaddr because kernel
				 * keeps nets unshifted.
				 */
				in = inet_makeaddr(ifaddr.in.ia_subnet,
					INADDR_ANY);
				printf("%-10.10s  ", netname(in));
#else
				printf("%-10.10s  ",
					netname(htonl(ifaddr.in.ia_subnet),
						ifaddr.in.ia_subnetmask));
#endif
				printf("%-12.12s ", routename(sin->sin_addr));
				break;
#ifdef NS
			case AF_NS:
				{
				struct sockaddr_ns *sns =
				(struct sockaddr_ns *)&ifaddr.in.ia_addr;
				printf("ns:%-8d ",
					ntohl(ns_netof(sns->sns_addr)));
				printf("%-12s ",ns_phost(sns));
				}
				break;
#endif
			case AF_BSC:
				printf("%-10.10s  ","2780/3780");
				printf("%-12.12s ","none");
				break;
			default: {
				u_char *start, *end;
				int len;

				printf("af%-2.2d: ",
				     ifaddr.ifa.ifa_addr.sa_family);
				start = (u_char *)ifaddr.ifa.ifa_addr.sa_data;
				len = sizeof(ifaddr.ifa.ifa_addr.sa_data);
				end = start + len - 1;
				/* zoom end back to first non null byte */
				while (*end == 0 && end >= start)
					end--;
				if (*end == 0) { /* no address */
					printf("%-5.5s ", "none");
					printf("%-12.12s ", "none");
				} else {
					len = 0; /* keep track of field width */
					while (start < end) {
						len += widthof(*start & 0xff);
						len++; /* for the "." */
						printf("%d.", *start++ & 0xff);
					}
					printf("%d ", *start);
					len += widthof(*start & 0xff);
					/* 22  - 6 used already == 18 */
					/* pad to next field */
					if (len < 18) {
						len = 18 - len;
						printf("%*.*s",len,len," ");
					}
				}
				break;
			} /* end default */

			}
		}
		printf("%-7d %-5d %-7d %-5d %-6d",
		    ifnet.if_ipackets, ifnet.if_ierrors,
		    ifnet.if_opackets, ifnet.if_oerrors,
		    ifnet.if_collisions);
		if (tflag)
			printf(" %-6d", ifnet.if_timer);
		putchar('\n');
	}
}
widthof(n) 
	int n;
{
	if (n < 10)
		return(1);
	if (n < 100)
		return(2);
	return(3);
}

#define	MAXIF	10
struct	iftot {
	char	ift_name[16];		/* interface name */
	int	ift_ip;			/* input packets */
	int	ift_ie;			/* input errors */
	int	ift_op;			/* output packets */
	int	ift_oe;			/* output errors */
	int	ift_co;			/* collisions */
} iftot[MAXIF];

/*
 * Print a running summary of interface statistics.
 * Repeat display every interval seconds, showing
 * statistics collected over that interval.  First
 * line printed at top of screen is always cumulative.
 */
sidewaysintpr(interval, off)
	int interval;
	off_t off;
{
	struct ifnet ifnet;
	off_t firstifnet;
	register struct iftot *ip, *total;
	register int line;
	struct iftot *lastif, *sum, *interesting;
	int maxtraffic;

	klseek(kmem, off, 0);
	read(kmem, &firstifnet, sizeof (off_t));
	lastif = iftot;
	sum = iftot + MAXIF - 1;
	total = sum - 1;
	interesting = iftot;
	for (off = firstifnet, ip = iftot; off;) {
		char *cp;

		klseek(kmem, off, 0);
		read(kmem, &ifnet, sizeof ifnet);
		klseek(kmem, (int)ifnet.if_name, 0);
		ip->ift_name[0] = '(';
		read(kmem, ip->ift_name + 1, 15);
		if (interface && strcmp(ip->ift_name + 1, interface) == 0 &&
		    unit == ifnet.if_unit)
			interesting = ip;
		ip->ift_name[15] = '\0';
		cp = index(ip->ift_name, '\0');
		sprintf(cp, "%d)", ifnet.if_unit);
		ip++;
		if (ip >= iftot + MAXIF - 2)
			break;
		off = (off_t) ifnet.if_next;
	}
	lastif = ip;
banner:
	printf("    input   %-6.6s    output       ", interesting->ift_name);
	if (lastif - iftot > 0)
		printf("   input  (Total)    output       ");
	for (ip = iftot; ip < iftot + MAXIF; ip++) {
		ip->ift_ip = 0;
		ip->ift_ie = 0;
		ip->ift_op = 0;
		ip->ift_oe = 0;
		ip->ift_co = 0;
	}
	putchar('\n');
	printf("%-7.7s %-5.5s %-7.7s %-5.5s %-5.5s ",
		"packets", "errs", "packets", "errs", "colls");
	if (lastif - iftot > 0)
		printf("%-7.7s %-5.5s %-7.7s %-5.5s %-5.5s ",
			"packets", "errs", "packets", "errs", "colls");
	putchar('\n');
	fflush(stdout);
	line = 0;
loop:
	sum->ift_ip = 0;
	sum->ift_ie = 0;
	sum->ift_op = 0;
	sum->ift_oe = 0;
	sum->ift_co = 0;
	for (off = firstifnet, ip = iftot; off && ip < lastif; ip++) {
		klseek(kmem, off, 0);
		read(kmem, &ifnet, sizeof ifnet);
		if (ip == interesting)
			printf("%-7d %-5d %-7d %-5d %-5d ",
				ifnet.if_ipackets - ip->ift_ip,
				ifnet.if_ierrors - ip->ift_ie,
				ifnet.if_opackets - ip->ift_op,
				ifnet.if_oerrors - ip->ift_oe,
				ifnet.if_collisions - ip->ift_co);
		ip->ift_ip = ifnet.if_ipackets;
		ip->ift_ie = ifnet.if_ierrors;
		ip->ift_op = ifnet.if_opackets;
		ip->ift_oe = ifnet.if_oerrors;
		ip->ift_co = ifnet.if_collisions;
		sum->ift_ip += ip->ift_ip;
		sum->ift_ie += ip->ift_ie;
		sum->ift_op += ip->ift_op;
		sum->ift_oe += ip->ift_oe;
		sum->ift_co += ip->ift_co;
		off = (off_t) ifnet.if_next;
	}
	if (lastif - iftot > 0)
		printf("%-7d %-5d %-7d %-5d %-5d\n",
			sum->ift_ip - total->ift_ip,
			sum->ift_ie - total->ift_ie,
			sum->ift_op - total->ift_op,
			sum->ift_oe - total->ift_oe,
			sum->ift_co - total->ift_co);
	*total = *sum;
	fflush(stdout);
	line++;
	if (interval)
		sleep(interval);
	if (line == 21)
		goto banner;
	goto loop;
	/*NOTREACHED*/
}
