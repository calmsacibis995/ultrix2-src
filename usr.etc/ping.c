#ifndef lint
static char *sccsid = "@(#)ping.c	1.2	ULTRIX	10/3/86";
#endif lint

/****************************************************************
 *								*
 *  Licensed to Digital Equipment Corporation, Maynard, MA	*
 *		Copyright 1985 Sun Microsystems, Inc.		*
 *			All rights reserved.			*
 *								*
 ****************************************************************/

/* 
 * ping host [timeout]
 *
 *	attempts to see if machine is alive by pinging it for
 *      timeout seconds (default is 20)
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <sys/wait.h>
#include <ctype.h>

char *adrtostr(), *host;
int noanswer(), die();
int pid;
#define DEFTIMEOUT 20
#define MAXALARM 2147483647	/* max arg to alarm() */

#if 0
#define PACKETSIZE 16		/* 8 bytes for icmp + 8 for struct timeval */
#endif

#define PACKETSIZE 56		/* old kernel bug needs this */

main(argc, argv)
	char *argv[];
{
	char buf[PACKETSIZE];
	struct icmp *icp = (struct icmp *)buf;
	int s, fromlen, size, packetsize, timeout, adr;
	struct timeval time;
	struct hostent *hp;
	struct sockaddr_in to, from;
	union wait status;

	struct protoent *proto;
	
	packetsize = PACKETSIZE;
	if (argc < 2){
	    	fprintf(stderr, "usage: ping host [timeout]\n");
		exit(1);
	}
	host = argv[1];
	if (isdigit(host[0]))
		adr = inet_addr(host);
	else {
		if ((hp = gethostbyname(host)) == NULL) {
			fprintf(stderr, "can't find host %s\n", host);
			exit(1);
		}
		adr = *((int *)hp->h_addr);
	}

	if (argc == 3) {
		timeout = atoi(argv[2]);
		if (timeout < 0 | timeout > MAXALARM) {
			fprintf(stderr, "invalid timeout\n");
			exit(1);
		}
	}
	else
		timeout = DEFTIMEOUT;
#if 0
	if ((s = socket(AF_INET, SOCK_RAW, 0)) < 0) {
		perror("ping: socket");
		exit(1);
	}
#endif
/* Code lifted from our pinger..
*/
	if ((proto = getprotobyname("icmp")) == NULL) {
		fprintf(stderr, "icmp: unknown protocol\n");
		exit(10);
	}
	if ((s = socket(AF_INET, SOCK_RAW, proto->p_proto)) < 0) {
		perror("ping: socket");
		exit(5);
	}
/* ECode lifted
*/
	to.sin_family = AF_INET;
	to.sin_port = 0;
	to.sin_addr.s_addr = adr;
	gettimeofday(&time, 0);
	bcopy(&time, buf + 8, sizeof(struct timeval));
	icp->icmp_type = ICMP_ECHO;
	icp->icmp_code = 0;
	icp->icmp_cksum = 0;
	icp->icmp_id = 1;
	icp->icmp_seq = 1;
	icp->icmp_cksum = in_cksum(icp, packetsize);

	if ((pid = fork()) < 0) {
	    perror("ping: fork");
	    exit(1);
	}
	if (pid != 0) {		/* parent */
		signal(SIGINT, die);
		while (1) {
			if (sendto(s, icp, packetsize, 0, &to, sizeof(to))
			    != packetsize) {
				perror("ping: sendto");
				kill (pid, SIGKILL);
				exit(1);
			}
			sleep(1);
			if (wait3(&status, WNOHANG, 0) == pid)
				if (status.w_termsig == 0)
					exit(status.w_retcode);
				else
					exit(-1);
		}
	}

	if (pid == 0) {		/* child */
		alarm(timeout);
		signal(SIGALRM, noanswer);
		while (1) {
			fromlen = sizeof(from);
			if ((size = recvfrom(s, buf, sizeof(buf), 0,
			    &from, &fromlen)) < 0) {
				perror("ping: recvfrom");
				continue;
			}
			if (size != packetsize) {
				continue;
			}
			if (icp->icmp_type != ICMP_ECHOREPLY) {
fprintf(stderr,"icp->icmp_type  (%d)  !=  ICMP_ECHOREPLY (%d)\n",icp->icmp_type,ICMP_ECHOREPLY);
				continue;
			}
			if (bcmp(buf + 8, &time, sizeof(struct timeval)) != 0){
				continue;
			}
			printf("%s is alive\n", adrtostr(from.sin_addr));
			exit(0);
		}
	}
}

char *
adrtostr(adr)
int adr;
{
	struct hostent *hp;
	char buf[100];		/* hope this is long enough */
	
	hp = gethostbyaddr(&adr, sizeof(adr), AF_INET);
	if (hp == NULL) {
	    	sprintf(buf, "0x%x", adr);
		return buf;
	}
	else
		return hp->h_name;
}

in_cksum(addr, len)
	u_short *addr;
	int len;
{
	register u_short *ptr;
	register int sum;
	u_short *lastptr;

	sum = 0;
	ptr = (u_short *)addr;
	lastptr = ptr + (len/2);
	for (; ptr < lastptr; ptr++) {
		sum += *ptr;
		if (sum & 0x10000) {
			sum &= 0xffff;
			sum++;
		}
	}
	return (~sum & 0xffff);
}

noanswer()
{
	printf("no answer from %s\n", host);
	exit(1);
}

die()
{
	kill (pid, SIGKILL);
	exit(1);
}
