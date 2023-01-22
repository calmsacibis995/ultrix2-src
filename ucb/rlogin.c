#ifndef lint
static	char	*sccsid = "@(#)rlogin.c	1.6  (ULTRIX)        2/12/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984,1985 by			*
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
 *
 *	06-Feb-87	lp
 * 	Bugfixes  + Sigwinch + Changes from 43bsd
 *
 *	9-5-85  lp
 *	Changed order of reader/writer calls when using select io
 *	arbitration. Seems to be a syncronization problem with the 
 *	rlogind when the other order. This problem showed up as 
 *	a hangup if too many interrupts are sent when recieving
 * 	lots of output. The change in order makes it act more like
 *	the original version & minimizes the problem (seems to make
 *	it go away completely.
 *
 *	8-29-85 lp
 *	Changed how kill gets done when ^Z and ^Y get done. It is
 *	not sufficient to kill just getpid() when ^Z is done as
 *	if you're running from a shell the shell won't be stopped.
 *	What gets done now is a ^Z == kill(0, SIGTSTP) and a ^Y
 *	is the same thing with the reader process ignoring SIGTSTP.
 *
 *	Revised 2/17/85 by lp@decvax.
 *	Use select to eliminate reader process. Reader process will
 *	now only awaken on ~^Y command (ie a reader subprocess will
 *	be made while main program is suspended). This process will
 *	die when the main program returns (or exits).
 *	This change can be incorporated into rlogin by compiling with
 *	with no option (-DNOSELECT gets 2 process version).
 */

/*
#ifndef lint
static char sccsid[] = "@(#)rlogin.c    4.15 (Berkeley) 83/07/02";
#endif
*/

/*
 * rlogin - remote login
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/file.h>

#include <netinet/in.h>

#include <stdio.h>
#include <sgtty.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <netdb.h>

#ifndef TIOCPKT_WINDOW
#define TIOCPKT_WINDOW 0x80
#endif  TIOCPKT_WINDOW

char	*index(), *rindex(), *malloc(), *getenv();
struct	passwd *getpwuid();
char	*name;
int	rem;
char	cmdchar = '~';
int	eight;
char	*speeds[] =
    { "0", "50", "75", "110", "134", "150", "200", "300",
      "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400" };
char	term[64] = "network";
extern	int errno;
int	lostpeer();
int 	dosigwinch = 0;

struct winsize winsize;
int sigwinch();

main(argc, argv)
	int argc;
	char **argv;
{
	char *host, *cp;
	struct sgttyb ttyb;
	struct passwd *pwd;
	struct servent *sp;
	int uid, options = 0;

	host = rindex(argv[0], '/');
	if (host)
		host++;
	else
		host = argv[0];
	argv++, --argc;
	if (!strcmp(host, "rlogin"))
		host = *argv++, --argc;
another:
	if (argc > 0 && !strcmp(*argv, "-d")) {
		argv++, argc--;
		options |= SO_DEBUG;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-l")) {
		argv++, argc--;
		if (argc == 0)
			goto usage;
		name = *argv++; argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-e", 2)) {
		cmdchar = argv[0][2];
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-8")) {
		eight = 1;
		argv++, argc--;
		goto another;
	}
	if (host == 0)
		goto usage;
	if (argc > 0)
		goto usage;
	pwd = getpwuid(getuid());
	if (pwd == 0) {
		fprintf(stderr, "Who are you?\n");
		exit(1);
	}
	sp = getservbyname("login", "tcp");
	if (sp == 0) {
		fprintf(stderr, "rlogin: login/tcp: unknown service\n");
		exit(2);
	}
	cp = getenv("TERM");
	if (cp)
		strcpy(term, cp);
	if (ioctl(0, TIOCGETP, &ttyb)==0) {
		strcat(term, "/");
		strcat(term, speeds[ttyb.sg_ospeed]);
	}
	(void) ioctl(0, TIOCGWINSZ, &winsize);
	signal(SIGPIPE, lostpeer);
	rem = rcmd(&host, sp->s_port, pwd->pw_name,
	    name ? name : pwd->pw_name, term, 0);
	if (rem < 0)
		exit(1);
	if (options & SO_DEBUG &&
	    setsockopt(rem, SOL_SOCKET, SO_DEBUG, 0, 0) < 0)
		perror("rlogin: setsockopt (SO_DEBUG)");
	uid = getuid();
	if (setuid(uid) < 0) {
		perror("rlogin: setuid");
		exit(1);
	}
	doit();
	/*NOTREACHED*/
usage:
	fprintf(stderr,
	    "usage: rlogin host [ -ex ] [ -l username ] [ -8 ]\n");
	exit(1);
}

#define CRLF "\r\n"

int	child;
int	catchild();

int	defflags, tabflag;
char	deferase, defkill;
struct	tchars deftc;
struct	ltchars defltc;
struct	tchars notc =	{ -1, -1, -1, -1, -1, -1 };
struct	ltchars noltc = { -1, -1, -1, -1, -1, -1 };

#ifndef NOSELECT
#include <sys/time.h>
	int running = 1;
	int readonly = 0;
#endif

doit()
{
	int exit();
	struct sgttyb sb;
#ifndef NOSELECT
	int *readfd, *writefd=0, *exfd=0, maski, masko, mask, found;
	int oob();
	struct timeval *time=0;
	int osigmask, on=1;
	maski = (1<<0);
	masko = (1<<rem);
#endif

	ioctl(0, TIOCGETP, (char *)&sb);
	defflags = sb.sg_flags;
	tabflag = defflags & TBDELAY;
	defflags &= ECHO | CRMOD;
	deferase = sb.sg_erase;
	defkill = sb.sg_kill;
	ioctl(0, TIOCGETC, (char *)&deftc);
	notc.t_startc = deftc.t_startc;
	notc.t_stopc = deftc.t_stopc;
	ioctl(0, TIOCGLTC, (char *)&defltc);
	signal(SIGINT, exit);
	signal(SIGHUP, exit);
	signal(SIGQUIT, exit);
#ifdef NOSELECT
	child = fork();
	if (child == -1) {
		perror("rlogin: fork");
		done();
	}
	signal(SIGINT, SIG_IGN);
	mode(1);
	if (child == 0) {
		reader();
		sleep(1);
		prf("\007Connection closed.");
		exit(3);
	}
	signal(SIGCHLD, catchild);
	writer();
#else
	signal(SIGINT, SIG_IGN);
	signal(SIGURG, oob); 
	mode(1);
	{ int pid = -getpid();
	  ioctl(rem, SIOCSPGRP, (char *)&pid); }
	while (running) {
		mask = maski|masko;
		if (readonly)
			mask = masko;
		readfd = &mask;
		found = select(32, readfd, writefd, exfd, time);
		if(found == -1) {     /* Ignore if select is interrupted */
			errno = 0;
			continue;
		}
		if (*readfd&masko) {
			(void) reader();
			if (!running) {
		/*		sleep(1); */
				prf("Connection closed.");
				mode(0);
				exit(3);
			}
		}
		if (!readonly&&(*readfd&maski)) {
			osigmask = sigblock((1<<(SIGURG-1))); /* block sigurg */
			if (!writer())
				break;
			osigmask = sigsetmask(osigmask); /* unblock sigurg */
		}
	}
#endif
	prf("Closed connection.");
	done();
}

done()
{

	mode(0);
#ifdef NOSELECT
	if (child > 0 && kill(child, SIGKILL) >= 0)
		wait((int *)0);
#endif
	exit(0);
}

catchild()
{
	union wait status;
	int pid;

again:
	pid = wait3(&status, WNOHANG|WUNTRACED, 0);
	if (pid == 0)
		return;
	/*
	 * if the child (reader) dies, just quit
	 */
	if (pid < 0 || pid == child && !WIFSTOPPED(status))
		done();
	goto again;
}

/*
 * writer: write to remote: 0 -> line.
 * ~.	terminate
 * ~^Z	suspend rlogin process.
 * ~^Y	suspend rlogin process, but leave reader alone.
 */
#ifndef NOSELECT
#define MAXLOOP 8	/* Simulate an 8 character typeahead else select might eat your system */
       int local = 0;
       int bol = 1;
#endif

writer()
{
#ifdef NOSELECT
	char c;
	register n;
#else
	register char c;
	register n;
	int inchars;
	int numloop = 0;

	if(ioctl(0, FIONREAD, (caddr_t) &inchars) == -1)
		return(1);
	if(inchars <= 0)
		return(1);
	inchars = (inchars > MAXLOOP ? MAXLOOP : inchars);
#endif

top:
#ifdef NOSELECT
	for (;;) {
		int local;
#else
reread:
#endif

		n = read(0, &c, 1);
		if (n == 0)
#ifdef NOSELECT
			break;
#else
			return (0);
#endif
		if (n < 0)
#ifdef NOSELECT
			if (errno == EINTR)
				continue;
			else
				break;
#else
			if (errno == EWOULDBLOCK)
				return(1);
			else
				return(0);
#endif

		if (eight == 0)
			c &= 0177;
		/*
		 * If we're at the beginning of the line
		 * and recognize a command character, then
		 * we echo locally.  Otherwise, characters
		 * are echo'd remotely.  If the command
		 * character is doubled, this acts as a
		 * force and local echo is suppressed.
		 */
		if(bol) {
			bol = 0;
			if(c == cmdchar) {
				bol = 0;
				local = 1;
#ifdef NOSELECT
				continue;
#else
				return(1);
#endif
			}
		} else if(local) {
			local = 0;
			if (c == '.' || c == deftc.t_eofc) {
				echo(c);
#ifdef NOSELECT
				break;
#else
				return(0);
#endif
			}
			if (c == defltc.t_suspc ||
				    c == defltc.t_dsuspc) {
				    	bol = 1;
					echo(c);
#ifdef NOSELECT
					mode(0);
					signal(SIGCHLD, SIG_IGN);
					kill(c == defltc.t_suspc ?
					  0 : getpid(), SIGTSTP);
					signal(SIGCHLD, catchild);
					mode(1);
					goto top;
#else
					if(c == defltc.t_dsuspc) {
						child = fork();
						if (child == -1)
							perror("rlogin: can't fork reader");
						if (child == 0) {
						      readonly = 1;
						      signal(SIGTSTP, SIG_IGN);
						      return(1);
						}
					}
					mode(0);
	     /*
		kill 0 in case we're running from a shell.
		Note that if we're doing ^Y case the main
		process is now (should be by now) ignoring
		SIGTSTP so that the readonly process is not
		suspended by the following kill.
	      */

	     /* suspend all */		kill((c == defltc.t_suspc ? 0 : getpid()), SIGTSTP);
					if(c == defltc.t_dsuspc) {
		/* Kill reader */		kill(child, SIGKILL);
						(void) wait(0);
					}
					readonly = 0;
					mode(1);
					sigwinch();
					return (1);
#endif

				}
			if(c != cmdchar)
				write(rem, &cmdchar, 1);
			}
			if (write(rem, &c, 1) <= 0) {
				prf("line gone");
#ifdef NOSELECT
				return;
#else
				return (0);
#endif
			}
		bol = c == defkill || c == deftc.t_eofc ||
		    c == '\r' || c == '\n' || c == defltc.t_suspc ||
		    c == deftc.t_intrc;
#ifdef NOSELECT
	}
#else
	/* If there are more input chars go get them */
	if(--inchars > 0)
		goto top;
#endif
	return (1);
}

echo(c)
register char c;
{
	char buf[8];
	register char *p = buf;
	
	c &= 0177;
	*p++ = cmdchar;
	if (c < ' ') {
		*p++ = '^';
		*p++ = c + '@';
	} else if(c == 0177) {
		*p++ = '^';
		*p++ = '?';
	} else
		*p++ = c;
	*p++ = '\r';
	*p++ = '\n';
	write(1, buf, p - buf);
}
	
oob()
{
	int in = FREAD,out = FWRITE, atmark;
	char waste[BUFSIZ], mark;
	struct sgttyb sb;

	recv(rem, &mark, 1, MSG_OOB);

	if(mark & TIOCPKT_WINDOW) {
		if(dosigwinch == 0) {
			sendwindow();
			signal(SIGWINCH, sigwinch);
		}
		dosigwinch = 1;
	}
	if(!eight && (mark & TIOCPKT_FLUSHREAD))
		ioctl(0, TIOCFLUSH, (char *)&in); 

	if (!eight && (mark & TIOCPKT_NOSTOP)) {
		ioctl(0, TIOCGETP, (char *)&sb);
		sb.sg_flags &= ~CBREAK;
		sb.sg_flags |= RAW;
		ioctl(0, TIOCSETN, (char *)&sb); 
		notc.t_stopc = -1;
		notc.t_startc = -1;
		ioctl(0, TIOCSETC, (char *)&notc);
	}
	if (!eight && (mark & TIOCPKT_DOSTOP)) {
		ioctl(0, TIOCGETP, (char *)&sb);
		sb.sg_flags &= ~RAW;
		sb.sg_flags |= CBREAK;
		ioctl(0, TIOCSETN, (char *)&sb); 
		notc.t_stopc = deftc.t_stopc;
		notc.t_startc = deftc.t_startc;
		ioctl(0, TIOCSETC, (char *)&notc);
	}

	if(mark & TIOCPKT_FLUSHWRITE) {
	ioctl(1, TIOCFLUSH, (char *)&out);
	for (;;) {
		int n;
		if (ioctl(rem, SIOCATMARK, &atmark) < 0) {
			perror("ioctl");
			break;
		}
		if (atmark)
			break;
		n = read(rem, waste, sizeof (waste));
		if(n <= 0)
			break;
	}
	}
}

/*
 * reader: read from remote: line -> 1
 */
reader()
{
	char rb[BUFSIZ];
	register int cnt;

#ifdef NOSELECT
	signal(SIGURG, oob);
	{ int pid = -getpid();
	  ioctl(rem, SIOCSPGRP, (char *)&pid); }
	for (;;) {
#endif
		cnt = read(rem, rb, sizeof (rb));
		if (cnt == 0)
#ifdef NOSELECT
			break;
#else
			return (running = 0);
#endif
		if (cnt < 0) {
#ifdef NOSELECT
			if (errno == EINTR)
				continue;
			break;
#else
			return(running = 0);
#endif
		}
		write(1, rb, cnt);
#ifdef NOSELECT
	}
#else
	return (running = 1);
#endif
}

mode(f)
{
	struct tchars *tc;
	struct ltchars *ltc;
	struct sgttyb sb;

	ioctl(0, TIOCGETP, (char *)&sb);
	switch (f) {

	case 0:
		sb.sg_flags &= ~(CBREAK|RAW|TBDELAY);
		sb.sg_flags |= defflags|tabflag;
		tc = &deftc;
		ltc = &defltc;
		sb.sg_kill = defkill;
		sb.sg_erase = deferase;
		break;

	case 1:
		sb.sg_flags |= (eight ? RAW : CBREAK);
		sb.sg_flags &= ~defflags;
		/* preserve tab delays, but turn off XTABS */
		if ((sb.sg_flags & TBDELAY) == XTABS)
			sb.sg_flags &= ~TBDELAY;
		tc = &notc;
		ltc = &noltc;
		sb.sg_kill = sb.sg_erase = -1;
		break;

	default:
		return;
	}
	ioctl(0, TIOCSLTC, (char *)ltc);
	ioctl(0, TIOCSETC, (char *)tc);
	ioctl(0, TIOCSETN, (char *)&sb);
}

/*VARARGS*/
prf(f, a1, a2, a3)
	char *f;
{
	fprintf(stderr, f, a1, a2, a3);
	fprintf(stderr, CRLF);
}

lostpeer()
{
	signal(SIGPIPE, SIG_IGN);
	prf("\007Connection closed.");
	done();
}

sigwinch()
{
	struct winsize ws;

	if (dosigwinch && ioctl(0, TIOCGWINSZ, &ws) == 0 &&
		bcmp(&ws, &winsize, sizeof(ws))) {
		winsize = ws;
		sendwindow();
	}
}

sendwindow()
{
	char obuf[4 + sizeof (struct winsize)];
	register struct winsize *wp = (struct winsize *)(obuf+4);

	/* The magic sequence */
	obuf[0] = 0377; obuf[1] = 0377; obuf[2] = 's'; obuf[3] = 's';
	
	wp->ws_row = htons(winsize.ws_row);
	wp->ws_col = htons(winsize.ws_col);
	wp->ws_xpixel = htons(winsize.ws_xpixel);
	wp->ws_ypixel = htons(winsize.ws_ypixel);
	(void) write(rem, obuf, sizeof(obuf));
}
