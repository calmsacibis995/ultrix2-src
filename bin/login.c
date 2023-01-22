#ifndef lint
static	char	*sccsid = "@(#)login.c	1.20	ULTRIX	3/18/87";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
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
 *	$Source: /u1/usr/src/bin/RCS/login.c,v $
 *	$Author: jg $
 *	$Locker:  $
 *	$Log:	login.c,v $
 *
 * 1.19 3/12/87		lp
 * Three times has to make it right! Finally fixed the decnet/telnet
 * clash.
 *
 * 1.18 3/3/87		lp
 * Fixed the problem 1.15 problem again (as 1.17 broke decnet).
 *
 * 1.17 2/12/87 	lp
 * Fixed a silly problem introduced in rev 1.15 (which broke telnet logins).
 * 
 * Rev 1.16 1/14/86	lp
 * Increased ahosts to 128 (from 32) as hostnames of the form
 * "washington.berekley.edu username" might be longer than 32 and
 * hence might not match.
 *
 * Revision 1.15 86/12/4	Tim Burke
 * A change to the dodecnetlogin routine to set proxy to -1 so that a prompt
 * for password will appear.
 *
 * Revision 1.14 86/8/8		Tim Burke
 * Inserted changes proposed by Peter Harbo of decnet.  These changes are
 * mainly in the dodecnetlogin routine to return -1 if username, but no
 * password has been received.
 *
 * Revision 1.13 86/6/24	Tim
 * Changed so that invalid rlogins and dlogins are handled similarly.
 * Inserted copyright notice.
 *
 * Revision 1.12 86/3/11	Robin
 * Made change to count logins in the kernel.
 *
 * Revision 1.11 86/2/5    10:05:00 robin
 * fixed problem that stopped erase from working if a second try
 * at logging in was made.
 *
 * Revision 1.8  85/10/22  10:30:00  was
 * add decnet support for remote login
 *
 * Revision 1.7  84/10/29  17:24:33  jg
 * fix problem with -p option.  Wasn't allowing user name, so getty
 * wasn't happy.
 * 
 * Revision 1.6  84/10/25  14:12:23  jg
 * Undid utmp changes.  Added -p flag to preserve environment passed
 * from getty.
 * 
 * Revision 1.5  84/10/01  09:28:38  jg
 * fix allocation bug that caused login to fail.
 * 
 * Revision 1.4  84/09/09  15:50:19  jg
 * fix bug introduced by environment change; was not doing rlogin right.
 * 
 * Revision 1.3  84/09/09  14:51:31  jg
 * fixed login not to destroy the environment set up by whomever is calling it.
 * 
 * Revision 1.2  84/09/01  11:39:08  jg
 * changes required by utmp.h changes.
 * 
 * Revision 1.1  84/04/20  01:07:13  root
 * Initial revision
 * 
 */

/*
static char *rcsid_login_c = "$Header: login.c,v 1.7 84/10/29 17:24:33 jg Exp $";
static	char *sccsid = "@(#)login.c	4.34 (Berkeley) 84/05/07";
*/

/*
 * login [ name ]
 * login -r hostname (for rlogind)
 * login -h hostname (for telnetd, etc.)
 */

#include <sys/param.h>
#include <sys/quota.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>

#include <sgtty.h>
#include <utmp.h>
#include <signal.h>
#include <pwd.h>
#include <stdio.h>
#include <lastlog.h>
#include <errno.h>
#include <ttyent.h>
#include <syslog.h>

#define	SCMPN(a, b)	strncmp(a, b, sizeof(a))
#define	SCPYN(a, b)	strncpy(a, b, sizeof(a))

#define NMAX	sizeof(utmp.ut_name)

#define	FALSE	0
#define	TRUE	-1

char	nolog[] =	"/etc/nologin";
char	qlog[]  =	".hushlogin";
char	maildir[30] =	"/usr/spool/mail/";
char	lastlog[] =	"/usr/adm/lastlog";
struct	passwd nouser = {"", "nope", -1, -1, -1, "", "", "", "" };
struct	sgttyb ttyb;
struct	utmp utmp;
char	minusnam[16] = "-";
char	*envinit[] =
	{ 0 };		/* now set by setenv calls */
/*
 * This bounds the time given to login.  We initialize it here
 * so it can be patched on machines where it's too small.
 */
int	timeout = 240;

char	term[64];

struct	passwd *pwd;
char	*strcat(), *rindex(), *index();
int	timedout();
char	*ttyname();
char	*crypt();
char	*getpass();
char	*stypeof();
extern	char **environ;
extern	int errno;

struct	tchars tc = {
	CINTR, CQUIT, CSTART, CSTOP, CEOT, CBRK
};
struct	ltchars ltc = {
	CSUSP, CDSUSP, CRPRNT, CFLUSH, CWERASE, CLNEXT
};

int	rflag, dflag;
char	rusername[NMAX+1], lusername[NMAX+1];
char	rpassword[NMAX+1];
char	name[NMAX+1];
char	*rhost;

main(argc, argv)
	char *argv[];
{
	register char *namep;
	int pflag = 0;		/* preserve environment from getty */
	int t, f, c;
	int invalid, quietlog;
	int lcmask;
	FILE *nlfd;
	char *ttyn, *tty;
	int ldisc = 0, zero = 0;

	signal(SIGALRM, timedout);
	alarm(timeout);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	setpriority(PRIO_PROCESS, 0, 0);
	quota(Q_SETUID, 0, 0, 0);
	/*
	 * -p is used by getty to tell login not to destroy the environment
	 * -r is used by rlogind to cause the autologin protocol;
	 * -h is used by other servers to pass the name of the
	 * remote host to login so that it may be placed in utmp and wtmp
	 */
	if (argc > 1) {
		if (strcmp(argv[1], "-r") == 0) {
			rflag = doremotelogin(argv[2]);
			SCPYN(utmp.ut_host, argv[2]);
			argc = 0;
		}
		if (strcmp(argv[1], "-h") == 0 && getuid() == 0) {
			dflag = dodecnetlogin();
			SCPYN(utmp.ut_host, argv[2]);
			argc = 0;
		}
		if (strcmp(argv[1], "-p") == 0) {
			argv++;
			pflag = 1;
			argc -= 1;
		}
	}
	ioctl(0, TIOCLSET, &zero);
	lcmask = (LPRTERA || LCRTBS || LCRTERA);
	ioctl(0, TIOCLSET, &lcmask);
	ioctl(0, TIOCNXCL, 0);
	ioctl(0, FIONBIO, &zero);
	ioctl(0, FIOASYNC, &zero);
	ioctl(0, TIOCGETP, &ttyb);
	/*
	 * If talking to an rlogin process,
	 * propagate the terminal type and
	 * baud rate across the network.
	 */
	if (rflag)
		doremoteterm(term, &ttyb);
	ioctl(0, TIOCSLTC, &ltc);
	ioctl(0, TIOCSETC, &tc);
	ioctl(0, TIOCSETP, &ttyb);
	for (t = getdtablesize(); t > 3; t--)
		close(t);
	ttyn = ttyname(0);
	if (ttyn == (char *)0)
		ttyn = "/dev/tty??";
	tty = rindex(ttyn, '/');
	if (tty == NULL)
		tty = ttyn;
	else
		tty++;
	openlog("login", 0);
	t = 0;
	do {
		ldisc = 0;
		ioctl(0, TIOCSETD, &ldisc);
		invalid = FALSE;
		SCPYN(utmp.ut_name, "");
		/*
		 * Name specified, take it.
		 */
		if (argc > 1) {
			SCPYN(utmp.ut_name, argv[1]);
			argc = 0;
		}
		/*
		 * If remote login take given name,
		 * otherwise prompt user for something.
		 */
		if (rflag || dflag) {
			SCPYN(utmp.ut_name, lusername);
			/* autologin failed, prompt for passwd */
			if (rflag == -1)
				rflag = 0;
			if (dflag == -1)
				dflag = 0;
		} else
			getloginname(&utmp);
		if (!strcmp(pwd->pw_shell, "/bin/csh")) {
			ldisc = NTTYDISC;
			ioctl(0, TIOCSETD, &ldisc);
		}
		/*
		 * If no remote login authentication and
		 * a password exists for this user, prompt
		 * for one and verify it.
		 */
		if ((!rflag && !dflag) && *pwd->pw_passwd != '\0') {
			char *pp;

			setpriority(PRIO_PROCESS, 0, -4);
			pp = getpass("Password:");
			namep = crypt(pp, pwd->pw_passwd);
			setpriority(PRIO_PROCESS, 0, 0);
			if (strcmp(namep, pwd->pw_passwd))
				invalid = TRUE;
		}
		/*
		 * If user not super-user, check for logins disabled.
		 */
		if (pwd->pw_uid != 0 && (nlfd = fopen(nolog, "r")) > 0) {
			while ((c = getc(nlfd)) != EOF)
				putchar(c);
			fflush(stdout);
			sleep(5);
			exit(0);
		}
		/*
		 * If valid so far and root is logging in,
		 * see if root logins on this terminal are permitted.
		 */
		if (!invalid && pwd->pw_uid == 0 && !rootterm(tty)) {
			syslog(LOG_INFO, "ROOT LOGIN REFUSED %s", tty);
			invalid = TRUE;
		}
		if (invalid) {
			printf("Login incorrect\n");
			if (++t >= 5) {
				syslog(LOG_INFO,
				    "REPEATED LOGIN FAILURES %s, %s",
					tty, utmp.ut_name);
				ioctl(0, TIOCHPCL, (struct sgttyb *) 0);
				close(0);
				close(1);
				close(2);
				sleep(10);
				exit(1);
			}
		}
		if (*pwd->pw_shell == '\0')
			pwd->pw_shell = "/bin/sh";
		if (chdir(pwd->pw_dir) < 0 && !invalid ) {
			if (chdir("/") < 0) {
				printf("No directory!\n");
				invalid = TRUE;
			} else {
				printf("No directory! %s\n",
				   "Logging in with home=/");
				pwd->pw_dir = "/";
			}
		}
		/*
		 * Remote login invalid must have been because
		 * of a restriction of some sort, no extra chances.
		 */
		if ((rflag|dflag) && invalid)
			exit(1);
	} while (invalid);
/* committed to login turn off timeout */
	alarm(0);

	if (quota(Q_SETUID, pwd->pw_uid, 0, 0) < 0) {
		if (errno == EUSERS)
			printf("%s.\n%s.\n",
			   "Too many users logged on already",
			   "Try again later");
		else if (errno == EPROCLIM)
			printf("You have too many processes running.\n");
		else
			perror("setuid");
		sleep(5);
		exit(0);
	}
	if (pwd->pw_uid != 0) /* Always let root in */
	{
		if( quota(Q_ULIMIT,pwd->pw_uid,0,0) != 0)
		{
			printf("Too many users logged on already.\nTry again later.\n");
			sleep(5);
			exit(0);
		}
	}
	time(&utmp.ut_time);
	t = ttyslot();
	if (t > 0 && (f = open("/etc/utmp", O_WRONLY)) >= 0) {
		lseek(f, (long)(t*sizeof(utmp)), 0);
		SCPYN(utmp.ut_line, tty);
		write(f, (char *)&utmp, sizeof(utmp));
		close(f);
	}
	if ((f = open("/usr/adm/wtmp", O_WRONLY|O_APPEND)) >= 0) {
		write(f, (char *)&utmp, sizeof(utmp));
		close(f);
	}
	quietlog = access(qlog, F_OK) == 0;
	if ((f = open(lastlog, O_RDWR)) >= 0) {
		struct lastlog ll;

		lseek(f, (long)pwd->pw_uid * sizeof (struct lastlog), 0);
		if (read(f, (char *) &ll, sizeof ll) == sizeof ll &&
		    ll.ll_time != 0 && !quietlog) {
			printf("Last login: %.*s ",
			    24-5, (char *)ctime(&ll.ll_time));
			if (*ll.ll_host != '\0')
				printf("from %.*s\n",
				    sizeof (ll.ll_host), ll.ll_host);
			else
				printf("on %.*s\n",
				    sizeof (ll.ll_line), ll.ll_line);
		}
		lseek(f, (long)pwd->pw_uid * sizeof (struct lastlog), 0);
		time(&ll.ll_time);
		SCPYN(ll.ll_line, tty);
		SCPYN(ll.ll_host, utmp.ut_host);
		write(f, (char *) &ll, sizeof ll);
		close(f);
	}
	chown(ttyn, pwd->pw_uid, pwd->pw_gid);
	chmod(ttyn, 0622);
	setgid(pwd->pw_gid);
	strncpy(name, utmp.ut_name, NMAX);
	name[NMAX] = '\0';
	initgroups(name, pwd->pw_gid);
	quota(Q_DOWARN, pwd->pw_uid, (dev_t)-1, 0);
	setuid(pwd->pw_uid);
	/* destroy environment unless user has asked to preserve it */
	if (pflag == 0) environ = envinit;
	
	/* set up environment, this time without destruction */
	/* copy the environment before setenving */
	{
	int i = 0;

	char **envnew;
	while (environ [i] != NULL) i++;

	envnew = (char **) malloc (sizeof (char *) * (i + 1));
	for (; i >= 0; i--) envnew [i] = environ [i];
	environ = envnew;
	}

	setenv("HOME=",pwd->pw_dir);
	setenv("SHELL=",pwd->pw_shell);
	if (term[0] == '\0') strncpy (term,stypeof(tty), sizeof(term));
	setenv("TERM=",term);
	setenv("USER=",pwd->pw_name);
	setenv("PATH=",":/usr/ucb:/bin:/usr/bin");

	if ((namep = rindex(pwd->pw_shell, '/')) == NULL)
		namep = pwd->pw_shell;
	else
		namep++;
	strcat(minusnam, namep);
	umask(022);
	if (tty[sizeof("tty")-1] == 'd')
		syslog(LOG_NOTICE, "DIALUP %s %s", tty, pwd->pw_name);
	if (!quietlog) {		/*  */
		showmotd();
		strcat(maildir, pwd->pw_name);
		if (access(maildir, R_OK) == 0) {
			struct stat statb;
			stat(maildir, &statb);
			if (statb.st_size)
				printf("You have mail.\n");
		}
	}
	signal(SIGALRM, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGTSTP, SIG_IGN);
	closelog();
	execlp(pwd->pw_shell, minusnam, 0);
	if (pwd->pw_uid == 0)
	{
		printf("Bad shell, root will use /bin/sh\n");
		execlp("/bin/sh",minusnam, 0);
	}
	perror(pwd->pw_shell);
	printf("No shell\n");
	exit(0);
}

getloginname(up)
	register struct utmp *up;
{
	register char *namep;
	char c;

	while (up->ut_name[0] == '\0') {
		namep = up->ut_name;
		printf("login: ");
		while ((c = getchar()) != '\n') {
			if (c == ' ')
				c = '_';
			if (c == EOF)
				exit(0);
			if (namep < up->ut_name+NMAX)
				*namep++ = c;
		}
	}
	strncpy(lusername, up->ut_name, NMAX);
	lusername[NMAX] = 0;
	if ((pwd = getpwnam(lusername)) == NULL) 
		pwd = &nouser;
}

timedout()
{

	printf("Login timed out after %d seconds\n", timeout);
	exit(0);
}

int	stopmotd;
catch()
{

	signal(SIGINT, SIG_IGN);
	stopmotd++;
}

rootterm(tty)
	char *tty;
{
	register struct ttyent *t;

	if ((t = getttynam(tty)) != NULL) {
		if (t->ty_status & TTY_SECURE)
			return (1);
	}
	return (0);
}

showmotd()
{
	FILE *mf;
	register c;

	signal(SIGINT, catch);
	if ((mf = fopen("/etc/motd", "r")) != NULL) {
		while ((c = getc(mf)) != EOF && stopmotd == 0)
			putchar(c);
		fclose(mf);
	}
	signal(SIGINT, SIG_IGN);
}

#undef	UNKNOWN
#define UNKNOWN "su"

char *
stypeof(ttyid)
	char *ttyid;
{
	register struct ttyent *t;

	if (ttyid == NULL || (t = getttynam(ttyid)) == NULL)
		return (UNKNOWN);
	return (t->ty_type);
}

dodecnetlogin()
{
	char *getenv();
	char *cp;
	int proxy = 1;
	/*
	 * check environment variables are present, and defined for DECnet
	 *	if "NETWORK" != "DECnet", force login
	 *	if "ACCESS" == "DEFAULT", force login
	 *	if "USER" is not defined, or is too long, force
	 *	login
	 */
	if (((cp = getenv("NETWORK")) == 0) || (strcmp(cp, "DECnet") != 0))
		proxy = 0; /* Else we break telnet */
	/*
	 * if terminal type is defined, use it for local terminal
	 */
	if (cp = getenv("TERM")) {
		if (strcmp(cp, "none"))
			strncat(term, cp, sizeof(term)-6);
	}
	/*
	 * don't login if using default access
	 */
	if (((cp = getenv("ACCESS")) == 0) || (strcmp(cp, "DEFAULT") == 0))
		proxy = 0;
	/*
	 * if local name is too long, can't log user in
	 */
	if (((cp = getenv("USER")) == 0) || (strlen(cp) > NMAX)) {
		if(getenv("USERNAME") == NULL)
			return(0);
		else
			proxy=0;
	} else
		strcpy(lusername, cp);
	/*
	 * save the connecting host name
	 */
	if (cp = getenv("REMNODE"))
		SCPYN(utmp.ut_host, cp);
	/*
	 * Get username from environment if this is not a proxy line.
	 */
	if (*lusername == '\000') {
		if (cp = getenv("USERNAME"))
			strcpy(lusername,cp);
		proxy = -1;
	}
	setpwent();
	pwd = getpwnam(lusername);
	endpwent();
	if (pwd == NULL) {
		pwd = &nouser;
		proxy = -1;
	}
	return(proxy);
}

doremotelogin(host)
	char *host;
{
	FILE *hostf;
	int first = 1;

	getstr(rusername, sizeof (rusername), "remuser");
	getstr(lusername, sizeof (lusername), "locuser");
	getstr(term, sizeof(term), "Terminal type");
	if (getuid()) {
		pwd = &nouser;
		goto bad;
	}
	pwd = getpwnam(lusername);
	if (pwd == NULL) {
		pwd = &nouser;
		goto bad;
	}
	hostf = pwd->pw_uid ? fopen("/etc/hosts.equiv", "r") : 0;
again:
	if (hostf) {
		char ahost[128];

		while (fgets(ahost, sizeof (ahost), hostf)) {
			char *user;

			if ((user = index(ahost, '\n')) != 0)
				*user++ = '\0';
			if ((user = index(ahost, ' ')) != 0)
				*user++ = '\0';
			if (!strcmp(host, ahost) &&
			    !strcmp(rusername, user ? user : lusername)) {
				fclose(hostf);
				return (1);
			}
		}
		fclose(hostf);
	}
	if (first == 1) {
		char *rhosts = ".rhosts";
		struct stat sbuf;

		first = 0;
		if (chdir(pwd->pw_dir) < 0)
			goto again;
		if (lstat(rhosts, &sbuf) < 0)
			goto again;
		if ((sbuf.st_mode & S_IFMT) == S_IFLNK) {
			printf("login: .rhosts is a soft link.\r\n");
			goto bad;
		}
		hostf = fopen(rhosts, "r");
		fstat(fileno(hostf), &sbuf);
		if (sbuf.st_uid && sbuf.st_uid != pwd->pw_uid) {
			printf("login: Bad .rhosts ownership.\r\n");
			fclose(hostf);
			goto bad;
		}
		goto again;
	}
bad:
	return (-1);
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
		if (--cnt < 0) {
			printf("%s too long\r\n", err);
			exit(1);
		}
		*buf++ = c;
	} while (c != 0);
}

char	*speeds[] =
    { "0", "50", "75", "110", "134", "150", "200", "300",
      "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400" };
#define	NSPEEDS	(sizeof (speeds) / sizeof (speeds[0]))

doremoteterm(term, tp)
	char *term;
	struct sgttyb *tp;
{
	char *cp = index(term, '/');
	register int i;

	if (cp) {
		*cp++ = 0;
		for (i = 0; i < NSPEEDS; i++)
			if (!strcmp(speeds[i], cp)) {
				tp->sg_ispeed = tp->sg_ospeed = i;
				break;
			}
	}
	tp->sg_flags = ECHO|CRMOD|ANYP|XTABS;
}

setenv (var, value)
/*
   sets the value of var to be arg in the Unix 4.2 BSD environment env.
   Var should end with '='.
   (bindings are of the form "var=value")
   This procedure assumes the memory for the first level of environ
   was allocated using malloc.
 */
char *var, *value;
{
	extern char **environ;
	int index = 0;

	while (environ [index] != NULL)
	{
	    if (strncmp (environ [index], var, strlen (var)) == 0)
	    {
		/* found it */
		environ [index] = (char *) malloc (strlen (var) + strlen (value) + 1);
		strcpy (environ [index], var);
		strcat (environ [index], value);
		return;
	    }
	    index ++;
	}

	if ((environ = (char **) realloc (environ, sizeof (char *) * (index + 2))) == NULL)
	{
	    fprintf (stderr, "Setenv: malloc out of memory\n");
	    exit (1);
	}

	environ [index] = (char *) malloc (strlen (var) + strlen (value) + 1);
	strcpy (environ [index], var);
	strcat (environ [index], value);
	environ [++index] = NULL;
}
