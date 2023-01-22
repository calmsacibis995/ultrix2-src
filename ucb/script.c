#ifndef lint
static	char	*sccsid = "@(#)script.c	1.5	(ULTRIX)	3/11/86";
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

/*
 * script - 
 *
 * 	EDIT HISTORY:
 *	
 *	11-Mar-1986	Marc Teitelbaum
 *		Fix script to function properly in CBREAK mode.  
 *		Original (4.2bsd) script ran in RAW mode.  This breaks 
 *		on some terminals which need quick flow control response 
 *		(and causes too much output to screen after ^S anyway).  
 *		According to SCCS someone changed script to run in CBREAK 
 *		mode because they wanted parity generated on output.  This
 *		broke script because it is not setup to handle keyboard
 *		signals (nor is it supposed to be). While i'm not sure what 
 *		the issue is in regards to parity - it is certainly the case 
 *		that script should run in CBREAK mode for reasons stated above. 
 *
 */
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sgtty.h>
#include <sys/time.h>

char	*getenv();
char	*ctime();
char	*shell;
FILE	*fscript;
int	master;
int	slave;
int	child;
char	*fname = "typescript";
int	finish();

/* Original tty state */
struct	sgttyb b;	/* sgttyb */
struct	tchars tc;	/* tchars */
struct	ltchars lc;	/* lchars */
int	lb;		/* local mode word */
int	l;		/* line discipline */

/* New tty state for reader/writer  - we fill in startc/stopc later */
struct	tchars	notc  = { -1, -1, -1, -1, -1, -1 };
struct	ltchars	noltc = { -1, -1, -1, -1, -1, -1 };

char	*line = "/dev/ptyXX";
int	aflg;

main(argc, argv)
	int argc;
	char *argv[];
{
	int f;

	shell = getenv("SHELL");
	if (shell == 0)
		shell = "/bin/sh";
	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		switch (argv[0][1]) {

		case 'a':
			aflg++;
			break;

		default:
			fprintf(stderr,
			    "usage: script [ -a ] [ typescript ]\n");
			exit(1);
		}
		argc--, argv++;
	}
	if (argc > 0)
		fname = argv[0];
	if ((fscript = fopen(fname, aflg ? "a" : "w")) == NULL) {
		perror(fname);
		fail();
	}
	getmaster();
	printf("Script started, file is %s\n", fname);
	fixtty();

	(void) signal(SIGCHLD, finish);
	child = fork();
	if (child < 0) {
		perror("fork");
		fail();
	}
	if (child == 0) {
		f = fork();
		if (f < 0) {
			perror("fork");
			fail();
		}
		if (f)
			dooutput();
		else
			doshell();
	}
	doinput();
}

doinput()
{
	char ibuf[BUFSIZ];
	int cc;

	(void) fclose(fscript);
	while ((cc = read(0, ibuf, BUFSIZ)) > 0)
		(void) write(master, ibuf, cc);
	done();
}

#include <sys/wait.h>

finish()
{
	union wait status;

	if (wait3(&status, WNOHANG, 0) != child)
		return;
	done();
}

dooutput()
{
	time_t tvec;
	char obuf[BUFSIZ];
	int cc;

	(void) close(0);
	tvec = time((time_t *)0);
	fprintf(fscript, "Script started on %s", ctime(&tvec));
	for (;;) {
		cc = read(master, obuf, sizeof (obuf));
		if (cc <= 0)
			break;
		(void) write(1, obuf, cc);
		(void) fwrite(obuf, 1, cc, fscript);
	}
	tvec = time((time_t *)0);
	fprintf(fscript,"\nscript done on %s", ctime(&tvec));
	(void) fclose(fscript);
	(void) close(master);
	exit(0);
}

doshell()
{
	int t;

	t = open("/dev/tty", 2);
	if (t >= 0) {
		ioctl(t, TIOCNOTTY, (char *)0);
		(void) close(t);
	}
	getslave();
	(void) close(master);
	(void) fclose(fscript);
	dup2(slave, 0);
	dup2(slave, 1);
	dup2(slave, 2);
	(void) close(slave);
	execl(shell, "sh", "-i", 0);
	perror(shell);
	fail();
}

fixtty()
{
	struct sgttyb sbuf;

	sbuf = b;
	sbuf.sg_flags |= CBREAK;
	sbuf.sg_flags &= ~ECHO;
	ioctl(0, TIOCSETP, (char *)&sbuf);
	ioctl(0, TIOCSETC, (char *)&notc);
	ioctl(0, TIOCSLTC, (char *)&noltc);
}

fail()
{

	(void) kill(0, SIGTERM);
	done();
}

done()
{

	ioctl(0, TIOCSETP, (char *)&b);
	ioctl(0, TIOCSETC, (char *)&tc);
	ioctl(0, TIOCSLTC, (char *)&lc);
	printf("Script done, file is %s\n", fname);
	exit(0);
}

getmaster()
{
	char c;
	struct stat stb;
	int i;

	for (c = 'p'; c <= 's'; c++) {
		line[strlen("/dev/pty")] = c;
		line[strlen("/dev/ptyp")] = '0';
		if (stat(line, &stb) < 0)
			break;
		for (i = 0; i < 16; i++) {
			line[strlen("/dev/ptyp")] = "0123456789abcdef"[i];
			master = open(line, 2);
			if (master >= 0) {
				/* save state of original tty settings */
				ioctl(0, TIOCGETP, (char *)&b);
				ioctl(0, TIOCGETC, (char *)&tc);
				ioctl(0, TIOCGETD, (char *)&l);
				ioctl(0, TIOCGLTC, (char *)&lc);
				ioctl(0, TIOCLGET, (char *)&lb);
				/* propagate startc/stopc to new settings */
				notc.t_startc = tc.t_startc;
				notc.t_stopc = tc.t_stopc;
				return;
			}
		}
	}
	fprintf(stderr, "Out of pty's\n");
	fail();
}

getslave()
{

	line[strlen("/dev/")] = 't';
	slave = open(line, 2);
	if (slave < 0) {
		perror(line);
		fail();
	}
	ioctl(slave, TIOCSETP, (char *)&b);
	ioctl(slave, TIOCSETC, (char *)&tc);
	ioctl(slave, TIOCSLTC, (char *)&lc);
	ioctl(slave, TIOCLSET, (char *)&lb);
	ioctl(slave, TIOCSETD, (char *)&l);
}
