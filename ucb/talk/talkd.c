#ifndef lint
static	char sccsid[] = "@(#)talkd.c	1.2	(ULTRIX)	6/11/85";
#endif

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

/*-----------------------------------------------------------------------
 *	Modification History
 *
 *	4/5/85 -- jrs
 *		Revise to allow inetd to perform front end functions,
 *		following the Berkeley model.
 *
 *	Based on 4.2BSD labeled:
 *		talkd.c	1.4	84/12/20
 *
 *-----------------------------------------------------------------------
 */

/*
 * The top level of the daemon, the format is heavily borrowed
 * from rwhod.c. Basically: find out who and where you are; 
 * disconnect all descriptors and ttys, and then endless
 * loop on waiting for and processing requests
 */
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "ctl.h"

CTL_MSG		request;
CTL_RESPONSE	response;

int	sockt;
int	debug = 0;
FILE	*debugout;
int	timeout();
long	lastmsgtime;

char	hostname[32];

#define TIMEOUT 30
#define MAXIDLE 120

main(argc, argv)
	int argc;
	char *argv[];
{
	struct sockaddr_in from;
	int fromlen, cc;
	
	if (debug)
		debugout = (FILE *)fopen ("/usr/tmp/talkd.msgs", "w");

	if (getuid()) {
		fprintf(stderr, "Talkd : not super user\n");
		exit(1);
	}
	(void) gethostname(hostname, sizeof (hostname));
	(void) chdir("/dev");
	(void) signal(SIGALRM, timeout);
	(void) alarm(TIMEOUT);
	for (;;) {
		extern int errno;

		fromlen = sizeof(from);
		cc = recvfrom(0, (char *)&request, sizeof (request), 0,
		    &from, &fromlen);
		if (cc != sizeof(request)) {
			if (cc < 0 && errno != EINTR)
			perror("recvfrom");
			continue;
		}
		lastmsgtime = time(0);
		swapmsg(&request);
		if (debug) print_request(&request);
		process_request(&request, &response);
		/* can block here, is this what I want? */
		cc = sendto(sockt, (char *) &response,
		    sizeof (response), 0, &request.ctl_addr,
		    sizeof (request.ctl_addr));
		if (cc != sizeof(response))
			perror("sendto");
	}
}

timeout()
{

	if (time(0) - lastmsgtime >= MAXIDLE)
		exit(0);
	(void) alarm(TIMEOUT);
}

/*  
 * heuristic to detect if need to swap bytes
 */

swapmsg(req)
	CTL_MSG *req;
{
	if (req->ctl_addr.sin_family == ntohs(AF_INET)) {
		req->id_num = ntohl(req->id_num);
		req->pid = ntohl(req->pid);
		req->addr.sin_family = ntohs(req->addr.sin_family);
		req->ctl_addr.sin_family =
			ntohs(req->ctl_addr.sin_family);
	}
}

extern int sys_nerr;
extern char *sys_errlist[];

print_error(string)
char *string;
{
    FILE *cons;
    char *err_dev = "/dev/console";
    char *sys;
    int val, pid;

    if (debug) err_dev = "/dev/tty";

    sys = "Unknown error";

    if(errno < sys_nerr) {
	sys = sys_errlist[errno];
    }

	/* don't ever open tty's directly, let a child do it */
    if ((pid = fork()) == 0) {
	cons = fopen(err_dev, "a");
	if (cons != NULL) {
	    fprintf(cons, "Talkd : %s : %s(%d)\n\r", string, sys,
		    errno);
	    (void) fclose(cons);
	}
	exit(0);
    } else {
	    /* wait for the child process to return */
	do {
	    val = wait(0);
	    if (val == -1) {
		if (errno == EINTR) {
		    continue;
		} else if (errno == ECHILD) {
		    break;
		}
	    }
	} while (val != pid);
    }
}
