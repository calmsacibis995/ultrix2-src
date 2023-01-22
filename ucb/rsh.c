#ifndef lint
static	char	*sccsid = "@(#)rsh.c	1.6  (ULTRIX)        @(#)rsh.c	1.6";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * rsh.c
 *
 *	static char sccsid[] = "rsh.c       (Berkeley)  4.8 83/06/10";
 *
 *	14-Mar-85	Added SELECT (NO) defines to allow rsh to be compiled
 *			to run as a single process (via select system
 *			call). This results in a slightly more responsive
 *			rsh especially when you interrupt it.
 *							lp@decvax
 *
 *	22-Aug-84	ma.  Changed the usage message to reflect reality.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>

#include <netinet/in.h>

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <netdb.h>

/*
 * rsh - remote shell
 */
/* VARARGS */
int	error();
char	*index(), *rindex(), *malloc(), *getpass(), *sprintf(), *strcpy();

struct	passwd *getpwuid();

int	errno;
int	options;
int	rfd2;
int	sendsig();
int	noinput = 0;

#define mask(s) (1 << ((s) - 1))

main(argc, argv0)
	int argc;
	char **argv0;
{
	int rem, pid;
	char *host, *cp, **ap, buf[BUFSIZ], *args, **argv = argv0, *user = 0;
	register int cc;
	int asrsh = 0;
	struct passwd *pwd;
	int readfrom, ready;
	int one = 1;
	struct servent *sp;
	int omask;
	int bytin=0, bytout=0;

	host = rindex(argv[0], '/');
	if (host)
		host++;
	else
		host = argv[0];
	argv++, --argc;
	if (!strcmp(host, "rsh")) {
		host = *argv++, --argc;
		asrsh = 1;
	}
another:
	if (argc > 0 && !strcmp(*argv, "-l")) {
		argv++, argc--;
		if (argc > 0)
			user = *argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-n")) {
		argv++, argc--;
		(void) close(0);
		(void) open("/dev/null", 0);
		noinput++;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-d")) {
		argv++, argc--;
		options |= SO_DEBUG;
		goto another;
	}
	/*
	 * Ignore the -e flag to allow aliases with rlogin
	 * to work
	 */
	if (argc > 0 && !strncmp(*argv, "-e", 2)) {
		argv++, argc--;
		goto another;
	}
	if (host == 0)
		goto usage;
	if (argv[0] == 0) {
		if (asrsh)
			*argv0 = "rlogin";
		execv("/usr/ucb/rlogin", argv0);
		perror("/usr/ucb/rlogin");
		exit(1);
	}
	pwd = getpwuid(getuid());
	if (pwd == 0) {
		fprintf(stderr, "who are you?\n");
		exit(1);
	}
	cc = 0;
	for (ap = argv; *ap; ap++)
		cc += strlen(*ap) + 1;
	cp = args = malloc(cc);
	for (ap = argv; *ap; ap++) {
		(void) strcpy(cp, *ap);
		while (*cp)
			cp++;
		if (ap[1])
			*cp++ = ' ';
	}
	sp = getservbyname("shell", "tcp");
	if (sp == 0) {
		fprintf(stderr, "rsh: shell/tcp: unknown service\n");
		exit(1);
	}
	rem = rcmd(&host, sp->s_port, pwd->pw_name,
	    user ? user : pwd->pw_name, args, &rfd2);
	if (rem < 0)
		exit(1);
	if (rfd2 < 0) {
		fprintf(stderr, "rsh: can't establish stderr\n");
		exit(2);
	}
	if (options & SO_DEBUG) {
		if (setsockopt(rem, SOL_SOCKET, SO_DEBUG, 0, 0) < 0)
			perror("setsockopt (stdin)");
		if (setsockopt(rfd2, SOL_SOCKET, SO_DEBUG, 0, 0) < 0)
			perror("setsockopt (stderr)");
	}
	(void) setuid(getuid());
	omask = sigblock(mask(SIGINT)|mask(SIGQUIT)|mask(SIGTERM));
	signal(SIGINT, sendsig);
	signal(SIGQUIT, sendsig);
	signal(SIGTERM, sendsig);
#ifdef NOSELECT
	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(1);
	}
	ioctl(rfd2, FIONBIO, &one);
	ioctl(rem, FIONBIO, &one);
	if (pid == 0) {
		char *bp; int rembits, wc;
		(void) close(rfd2);
	reread:
		errno = 0;
		cc = read(0, buf, sizeof buf);
		if (cc <= 0)
			goto done;
		bp = buf;
	rewrite:
		rembits = 1<<rem;
		if (select(16, 0, &rembits, 0, 0) < 0) {
			if (errno != EINTR) {
				perror("select");
				exit(1);
			}
			goto rewrite;
		}
		if ((rembits & (1<<rem)) == 0)
			goto rewrite;
		wc = write(rem, bp, cc);
		if (wc < 0) {
			if (errno == EWOULDBLOCK)
				goto rewrite;
			goto done;
		}
		cc -= wc; bp += wc;
		if (cc == 0)
			goto reread;
		goto rewrite;
	done:
		(void) shutdown(rem, 1);
		exit(0);
	}
	sigsetmask(omask);
	readfrom = (1<<rfd2) | (1<<rem);
	do {
		ready = readfrom;
		if (select(16, &ready, 0, 0, 0) < 0) {
			if (errno != EINTR) {
				perror("select");
				exit(1);
			}
			continue;
		}
		if (ready & (1<<rfd2)) {
			errno = 0;
			cc = read(rfd2, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK)
					readfrom &= ~(1<<rfd2);
			} else
				(void) write(2, buf, cc);
		}
		if (ready & (1<<rem)) {
			errno = 0;
			cc = read(rem, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK)
					readfrom &= ~(1<<rem);
			} else
				(void) write(1, buf, cc);
		}
	} while (readfrom);
	(void) kill(pid, SIGKILL);
	exit(0);
#else
	ioctl(rfd2, FIONBIO, &one);
	ioctl(rem, FIONBIO, &one);
	readfrom = (1<<rfd2) | (1<<rem);
	if( noinput == 0)
		readfrom |= (1<<0);
	omask = sigsetmask(omask);	/* unblock */
	do {
		char *bp; int rembits, wc;
		extern int interrupt;

		ready = readfrom;
		if (select(16, &ready, 0, 0, 0) < 0) {
			if (errno != EINTR) {
				perror("select");
				exit(1);
			}
			goto done;
		}
		omask = sigsetmask(omask);	/* block */
		if ((ready & (1<<0))) {
reread:
			errno = 0;
			cc = read(0, buf, sizeof buf);
			if (cc <= 0) {
				readfrom &= ~(1<<0);	
				goto cont1;
			}
			bp = buf;
rewrite:
			rembits = 1<<rem;
			if (select(16, 0, &rembits, 0, 0) < 0) {
				if (errno != EINTR) {
					perror("select");
					exit(1);
				}
				goto rewrite;
			}
			if ((rembits & (1<<rem)) == 0)
				goto rewrite;
			wc = write(rem, bp, cc);
			if (wc < 0) {
				if (errno == EWOULDBLOCK)
					goto rewrite;
				goto done;
			}
			cc -= wc; bp += wc;
			if (cc == 0)
				goto cont;
			goto rewrite;
cont1:
		shutdown(rem, 1);
		}
cont:
		omask = sigsetmask(omask);	/* unblock */

		if (ready & (1<<rfd2)) {
			errno = 0;
			cc = read(rfd2, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK) {
					readfrom &= ~(1<<rfd2);
				}
			} else
				(void) write(2, buf, cc);
		}
		if (ready & (1<<rem)) {
			errno = 0;
			cc = read(rem, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK) {
					readfrom &= ~(1<<rem);
					goto done;
				}
			} else
				(void) write(1, buf, cc);
		}
		if (interrupt)
			goto done;
	} while (readfrom);
done:
	(void) shutdown(rem, 2);
	exit(0);


#endif
usage:
	fprintf(stderr,
	    "usage: rsh host [ -l username ] [ -n ] command\n");
	exit(1);
}
int interrupt = 0;

sendsig(signo)
	int signo;
{

	(void) write(rfd2, (char *)&signo, 1);
	interrupt = 1;
}
