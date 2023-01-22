#ifndef lint
static	char	*sccsid = "@(#)rshd.c	1.4	(ULTRIX)	2/12/87";
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
 *	2/29/87 -- Larry Cohen
 *		- add close of file descriptor so that net connections
 *			for backgrounded commands will close.
 *	9/18/85 -- Larry Cohen
 *		change setsockopt commands to comform to 43bsd syntax
 *
 *	4/5/85 -- jrs
 *		Revise to allow inetd to perform front end functions,
 *		following the Berkeley model.
 *
 *	Based on 4.2BSD labeled:
 *		rshd.c	4.21	84/09/13
 *
 *-----------------------------------------------------------------------
 */

#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <netdb.h>
#include <syslog.h>

int	errno;
struct	passwd *getpwnam();
char	*index(), *rindex(), *sprintf();
/* VARARGS 1 */
int	error();
/*
 * remote shell server:
 *	remuser\0
 *	locuser\0
 *	command\0
 *	data
 */
main(argc, argv)
	int argc;
	char **argv;
{
	int on, fromlen;
	struct sockaddr_in from;
	struct linger linger;

	fromlen = sizeof(from);
	if (getpeername(0, &from, &fromlen) < 0) {
		openlog(argv[0], LOG_PID);
		syslog(LOG_ERR, "getpeername: %m");
		closelog();
		exit(1);
	}
	on = 1;
	if (setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)) < 0) {
		openlog(argv[0], LOG_PID);
		syslog(LOG_WARNING, "setsockopt(SO_KEEPALIVE): %m");
		closelog();
	}
	linger.l_onoff = on;
	linger.l_linger = 60;
	if (setsockopt(0, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger)) < 0) {
		openlog(argv[0], LOG_PID);
		syslog(LOG_WARNING, "setsockopt(SO_LINGER): %m");
		closelog();
	}
	(void) dup2(0, 3);
	(void) close(0);
	doit(3, &from);
}

char	username[20] = "USER=";
char	homedir[64] = "HOME=";
char	shell[64] = "SHELL=";
char	*envinit[] =
	    {homedir, shell, "PATH=:/usr/ucb:/bin:/usr/bin", username, 0};
char	**environ;

doit(f, fromp)
	int f;
	struct sockaddr_in *fromp;
{
	char cmdbuf[NCARGS+1], *cp;
	char locuser[16], remuser[16];
	struct passwd *pwd;
	int s;
	struct hostent *hp;
	short port;
	int pv[2], pid, ready, readfrom, cc;
	char buf[BUFSIZ], sig;
	int one = 1;

	(void) signal(SIGINT, SIG_DFL);
	(void) signal(SIGQUIT, SIG_DFL);
	(void) signal(SIGTERM, SIG_DFL);
#ifdef DEBUG
	{ int t = open("/dev/tty", 2);
	  if (t >= 0) {
		ioctl(t, TIOCNOTTY, (char *)0);
		(void) close(t);
	  }
	}
#endif
	fromp->sin_port = ntohs((u_short)fromp->sin_port);
	if (fromp->sin_family != AF_INET ||
	    fromp->sin_port >= IPPORT_RESERVED) {
		openlog("rshd", LOG_PID);
		syslog(LOG_ERR, "malformed from address");
		closelog();
		exit(1);
	}
	(void) alarm(60);
	port = 0;
	for (;;) {
		char c;
		if (read(f, &c, 1) != 1) {
			openlog("rshd", LOG_PID);
			syslog(LOG_ERR, "read: %m");
			closelog();
			(void) shutdown(f, 1+1);
			exit(1);
		}
		if (c == 0)
			break;
		port = port * 10 + c - '0';
	}
	(void) alarm(0);
	(void) dup2(f, 0);
	(void) dup2(f, 1);
	(void) dup2(f, 2);
	if (port != 0) {
		int lport = IPPORT_RESERVED - 1;
		s = rresvport(&lport);
		if (s < 0) {
			openlog("rshd", LOG_PID);
			syslog(LOG_ERR, "can't get stderr port");
			closelog();
			exit(1);
		}
		if (port >= IPPORT_RESERVED) {
			openlog("rshd", LOG_PID);
			syslog(LOG_ERR, "2nd port not reserved");
			closelog();
			exit(1);
		}
		fromp->sin_port = htons((u_short)port);
		if (connect(s, fromp, sizeof (*fromp)) < 0) {
			openlog("rshd", LOG_PID);
			syslog(LOG_ERR, "connect: %m");
			closelog();
			exit(1);
		}
	}
	hp = gethostbyaddr(&fromp->sin_addr, sizeof (struct in_addr),
		fromp->sin_family);
	if (hp == 0) {
		error("Host name for your address unknown\n");
		exit(1);
	}
	getstr(remuser, sizeof(remuser), "remuser");
	getstr(locuser, sizeof(locuser), "locuser");
	getstr(cmdbuf, sizeof(cmdbuf), "command");
	(void) setpwent();
	pwd = getpwnam(locuser);
	if (pwd == NULL) {
		error("Login incorrect.\n");
		exit(1);
	}
	(void) endpwent();
	if (chdir(pwd->pw_dir) < 0) {
		error("No remote directory.\n");
		exit(1);
	}
	if (ruserok(hp->h_name, pwd->pw_uid == 0, remuser, locuser) < 0) {
		error("Permission denied.\n");
		exit(1);
	}
	(void) write(2, "\0", 1);
	if (port) {
		if (pipe(pv) < 0) {
			error("Can't make pipe.\n");
			exit(1);
		}
		pid = fork();
		if (pid == -1)  {
			error("Try again.\n");
			exit(1);
		}
		if (pid) {
			(void) close(0); (void) close(1); (void) close(2);
			(void) close(f); (void) close(pv[1]);
			readfrom = (1<<s) | (1<<pv[0]);
			ioctl(pv[1], FIONBIO, (char *)&one);
			/* should set s nbio! */
			do {
				ready = readfrom;
				if (select(16, &ready, 0, 0, 0) < 0)
					break;
				if (ready & (1<<s)) {
					if (read(s, &sig, 1) <= 0)
						readfrom &= ~(1<<s);
					else
						killpg(pid, sig);
				}
				if (ready & (1<<pv[0])) {
					errno = 0;
					cc = read(pv[0], buf, sizeof (buf));
					if (cc <= 0) {
						(void) shutdown(s, 1+1);
						readfrom &= ~(1<<pv[0]);
					} else
						(void) write(s, buf, cc);
				}
			} while (readfrom);
			exit(0);
		}
		setpgrp(0, getpid());
		(void) close(s); (void) close(pv[0]);
		(void) dup2(pv[1], 2);
		(void) close(pv[1]);
	}
	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = "/bin/sh";
	(void) close(f);
	initgroups(pwd->pw_name, pwd->pw_gid);
	(void) setgid(pwd->pw_gid);
	(void) setuid(pwd->pw_uid);
	environ = envinit;
	strncat(homedir, pwd->pw_dir, sizeof(homedir)-6);
	strncat(shell, pwd->pw_shell, sizeof(shell)-7);
	(void) strncat(username, pwd->pw_name, sizeof(username)-6);
	cp = rindex(pwd->pw_shell, '/');
	if (cp)
		cp++;
	else
		cp = pwd->pw_shell;
	execl(pwd->pw_shell, cp, "-c", cmdbuf, 0);
	perror(pwd->pw_shell);
	exit(1);
protofail:
	error("rsh: protocol failure detected by remote\n");
	exit(1);
}

/* VARARGS 1 */
error(fmt)
	char *fmt;
{
	char buf[BUFSIZ];

	buf[0] = 1;
	(void) sprintf(buf+1, fmt);
	(void) write(2, buf, strlen(buf));
}

getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		*buf++ = c;
		if (--cnt == 0) {
			error("%s too long\n", err);
			exit(1);
		}
	} while (c != 0);
}
