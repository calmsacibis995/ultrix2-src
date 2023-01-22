#ifndef lint
static	char	*sccsid = "@(#)rcp.c	1.7	(ULTRIX)	4/2/86";
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
 * rcp.c
 *
 *	static char sccsid[] = "@(#)rcp.c	4.8 83/08/12";
 *
 *     16 Dec 86	lp
 *			Fix overwrite problem/bug. Rcp will try to tell
 *			if a local to local copy is happening but if the
 *			user tricks rcp then the a file can get overriden.
 *			The fix makes this override happen without 
 *			destroying the file.
 *
 *	2 Apr 86	depp
 *			Added in name pipe check.  This utility now checks
 *			for a name pipe and after reporting that it found
 *			one, ignores it.
 *
 *	9-Mar-86	Marc Teitelbaum
 *			Fix rcp hang problem.  Rcp wasen't checking
 *			return status of writes.  Under extreme conditions
 *			the write over the socket can fail due to lack of
 *			mbufs, which throws the rcp protocol out of sync.
 *			We now check all writes over the socket.
 *
 *	12-Apr-84	mah.  Fixed remote to local copy.
 *
 *	17-Apr-84	mah.  Fix remote to remote copies.
 *
 *	24-Aug-84	ma.  Correct to prevent copying into one's self.
 *
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/file.h>

#include <netinet/in.h>

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <ctype.h>
#include <errno.h>

#include <strings.h>
#include <netdb.h>


int	rem;
char	*colon(), *index(), *rindex(), *malloc(), *strcpy(), *sprintf();
int	errs;
int	lostconn();
int	iamremote;

int	errno;
char	*sys_errlist[];
int	iamremote, targetshouldbedirectory;
int	iamrecursive;
struct	passwd *pwd;
struct	passwd *getpwuid();
int	port;

/*VARARGS*/
int	error();

#define	ga()	 	(void) remwrite(rem, "", 1)
#define MAXHNAMLEN	255
#define MAXREMWRITES	20000	/* times to retry writes over socket */

/*
 *  Define DEBUG to write error retry counts
 *  to a file (/tmp/RCP.#, where #=pid).
 */
/*
#define DEBUG
*/

#ifdef DEBUG
int dfd;
#endif DEBUG

main(argc, argv)
	int argc;
	char **argv;
{
	char *targ, *host, *src;
	char *suser, *tuser;
	int i;
	char buf[BUFSIZ], cmd[16];
	char lhost[MAXHNAMLEN];
	struct hostent *fhost;
	struct servent *sp;

	sp = getservbyname("shell", "tcp");
	if (sp == NULL) {
		fprintf(stderr, "rcp: shell/tcp: unknown service\n");
		exit(1);
	}
	port = sp->s_port;
	
	setpwent();				/*rewind passwd file*/
	pwd = getpwuid(getuid());		/*get passwd for uid running*/
	endpwent();				/*close passwd file*/
	if (pwd == 0) {
						/*no passwd found -> error*/
		fprintf(stderr, "who are you?\n");
		exit(1);
	}
	argc--, argv++;				/*only called w/ a single flag*/

#ifdef DEBUG
	initdebug();
#endif

	if (argc > 0 && !strcmp(*argv, "-r")) {	/*check for dir copy*/
		iamrecursive++;
		argc--, argv++;
	}
						/*check for internal switches*/
	if (argc > 0 && !strcmp(*argv, "-d")) {
		targetshouldbedirectory = 1;
		argc--, argv++;
	}
	if (argc > 0 && !strcmp(*argv, "-f")) {	/*iniated by other rcp trying*/
		argc--, argv++; iamremote = 1;	/*to do a remote-to-local copy*/
		(void) response();
		(void) setuid(getuid());
		source(argc, argv);
		exit(errs);
	}
	if (argc > 0 && !strcmp(*argv, "-t")) {	/*iniated by other rcp trying*/
		argc--, argv++; iamremote = 1;	/*to do a local-to-remote copy*/
		(void) setuid(getuid());
		sink(argc, argv);
		exit(errs);
	}
	rem = -1;
	if (argc > 2)
		targetshouldbedirectory = 1;
						/*get command*/
	(void) sprintf(cmd, "rcp%s%s",
	    iamrecursive ? " -r" : "", targetshouldbedirectory ? " -d" : "");
	signal(SIGPIPE, lostconn);
	targ = colon(argv[argc - 1]);
	if (targ) {				/*target check out*/
		*targ++ = 0;
		if (*targ == 0)
			targ = ".";
		tuser = rindex(argv[argc - 1], '.');
		if (tuser) {
			*tuser++ = 0;
			if (!okname(tuser))	/*check for valid name*/
				exit(1);
		} else
			tuser = pwd->pw_name;	/*name of current uid running*/
		for (i = 0; i < argc - 1; i++) {
			src = colon(argv[i]);
			if (src) {		/*source check out*/
				*src++ = 0;
				if (*src == 0)
					src = ".";
				suser = rindex(argv[i], '.');
				if (suser) {
					*suser++ = 0;
					if (!okname(suser))
						continue;
						/*set buf with rsh command for
						 *copy.
						 */
						/*remote-to-remote copy with
						 *host.name syntax.
						 */
		(void) sprintf(buf, "rsh %s -l %s -n %s %s '%s.%s:%s'",
					    argv[i], suser, cmd,
					    src, argv[argc - 1], tuser, targ);
				} else		/*remote-to-remote copy without
						 *host.name syntax in source
						 *host.
						 */
		(void) sprintf(buf, "rsh %s -n %s %s '%s.%s:%s'",
					    argv[i], cmd,
					    src, argv[argc - 1], tuser, targ);
		/*full name of remote1*/
				fhost = gethostbyname(argv[i]);
				if (fhost==0) {
					printf("unknown host: %s\n", 
						argv[i]);
					exit(-1);
				}
				strcpy(lhost,fhost->h_name);
		/*full name of remote2*/
				fhost = gethostbyname(argv[argc - 1]);
				if (fhost==0) {
					printf("unknown host: %s\n", 
						argv[argc-1]);
					exit(-1);
				}
		/*check source user*/
				if(!suser)
					suser = pwd->pw_name;
		/*
		 *Comparison order is source to target.  Checking hosts, then
		 *file (either same file name or .), and last for user.
		 */
				if ( !(strcmp(lhost,fhost->h_name)) &&
			            (!(strcmp(src,targ)) ||
				     !(strcmp(targ,"."))) &&
			     	     !(strcmp(suser,tuser))) {
					printf("rcp: Cannot copy file to itself.\n");
					exit(-1);
				}
				(void) susystem(buf);
			} else {
				if (rem == -1) {
						/*local-to-remote copy*/
		/*get local host*/
					gethostname(lhost,MAXHNAMLEN);
		/*full name of remote*/
					fhost = gethostbyname(argv[argc - 1]);
					if (fhost==0) {
						printf("unknown host: %s\n", 
							argv[argc-1]);
						exit(-1);
					}
		/*
		 *Comparison order is source to target.  Checking hosts, then
		 *file (either same file name or .), and last for user.
		 */
					if ( !(strcmp(lhost,fhost->h_name)) &&
				            (!(strcmp(argv[i],targ)) ||
					     !(strcmp(targ,"."))) &&
				     	     !(strcmp(pwd->pw_name,tuser))) {
						printf("rcp: Cannot copy file to itself.\n");
						exit(-1);
					}
					(void) sprintf(buf, "%s -t %s",
					    cmd, targ);
					host = argv[argc - 1];
					rem = rcmd(&host, port,
					    pwd->pw_name, tuser,
					    buf, 0);
					if (rem < 0)
						exit(1);
					if (response() < 0)
						exit(1);
				}
				source(1, argv+i);
			}
		}
	} else {
		if (targetshouldbedirectory)
			verifydir(argv[argc - 1]);
		for (i = 0; i < argc - 1; i++) {
			src = colon(argv[i]);
			if (src == 0) {
						/*local-to-local copy*/
				(void) sprintf(buf, "/bin/cp%s %s %s",
				    iamrecursive ? " -r" : "",
				    argv[i], argv[argc - 1]);
				(void) susystem(buf);
			} else {
				*src++ = 0;
				if (*src == 0)
					src = ".";
				suser = rindex(argv[i], '.');
				if (suser) {
					*suser++ = 0;
					if (!okname(suser))
						continue;
				} else
					suser = pwd->pw_name;
						/*remote-to-local copy*/
		/*local host*/
				gethostname(lhost,MAXHNAMLEN);
		/*full name of remote*/
				fhost = gethostbyname(argv[i]);
				if (fhost==0) {
					printf("unknown host: %s\n", 
						argv[i]);
					exit(-1);
				}
		/*
		 *Comparison order is source to target.  Checking hosts, then
		 *file (either same file name or .), and last for user.
		 */
				if ( !(strcmp(fhost->h_name,lhost)) &&
				    (!(strcmp(src,argv[argc - 1])) ||
				     !(strcmp(argv[argc - 1],"."))) &&
				     !(strcmp(suser,pwd->pw_name))) {
					printf("rcp: Cannot copy file to itself.\n");
					exit(-1);
				}
				(void) sprintf(buf, "%s -f %s", cmd, src);
				host = argv[i];
				rem = rcmd(&host, port,
				    pwd->pw_name, suser,
				    buf, 0);
				if (rem < 0)
					exit(1);
				sink(1, argv+argc-1);
				(void) close(rem);	/*close sock stream*/
				rem = -1;
			}
		}
	}
	exit(errs);
}

verifydir(cp)
	char *cp;
{
	struct stat stb;

	if (stat(cp, &stb) < 0)
		goto bad;
	if ((stb.st_mode & S_GFMT) == S_GFDIR)
		return;
	errno = ENOTDIR;
bad:
	error("rcp: %s: %s.\n", cp, sys_errlist[errno]);
	exit(1);
}

char *
colon(cp)
	char *cp;
{

	while (*cp) {
		if (*cp == ':')
			return (cp);
		if (*cp == '/')
			return (0);
		cp++;
	}
	return (0);
}

okname(cp0)
	char *cp0;
{
	register char *cp = cp0;
	register int c;

	do {
		c = *cp;
		if (c & 0200)
			goto bad;
		if (!isalpha(c) && !isdigit(c) && c != '_' && c != '-')
			goto bad;
		cp++;
	} while (*cp);
	return (1);
bad:
	fprintf(stderr, "rcp: invalid user name %s\n", cp0);
	return (0);
}

susystem(buf)
	char *buf;
{

	if (fork() == 0) {			/*if child do...*/
		(void) setuid(getuid());
		(void) system(buf);
		_exit(0);
	} else
		(void) wait((int *)0);
}

source(argc, argv)
	int argc;
	char **argv;
{
	char *last, *name;
	struct stat stb;
	char buf[BUFSIZ];
	int x, sizerr, f;
	off_t i, status;

	for (x = 0; x < argc; x++) {
		name = argv[x];
		if (((status = stat(name,&stb)) >= 0) && 
				((stb.st_mode & S_GFMT) == S_GFPORT))
			goto notreg2;
			
		if (access(name, 4) < 0 || (f = open(name, 0)) < 0) {
			error("rcp: %s: %s\n", name, sys_errlist[errno]);
			continue;
		}
		if (status < 0)
			goto notreg;

		switch (stb.st_mode&S_GFMT) {

		case S_GFREG:
			break;

		case S_GFDIR:
			if (iamrecursive) {
				(void) close(f);
				rsource(name, (int)stb.st_mode);
				continue;
			}
			/* fall into ... */
		default:
notreg:
			(void) close(f);
notreg2:
			error("rcp: %s: not a plain file\n", name);
			continue;
		}
		last = rindex(name, '/');
		if (last == 0)
			last = name;
		else
			last++;
		(void) sprintf(buf, "C%04o %D %s\n",
		    stb.st_mode&07777, stb.st_size, last);
		(void) remwrite(rem, buf, strlen(buf));
		if (response() < 0) {
			(void) close(f);
			continue;
		}
		sizerr = 0;
		for (i = 0; i < stb.st_size; i += BUFSIZ) {
			int amt = BUFSIZ;
			if (i + amt > stb.st_size)
				amt = stb.st_size - i;
			if (sizerr == 0 && read(f, buf, amt) != amt)
				sizerr = 1;		/*read from src file*/
			(void) remwrite(rem, buf, amt);	/*ship down sock strm*/
		}
		(void) close(f);
		if (sizerr == 0)
			ga();
		else
			error("rcp: %s: file changed size\n", name);
		(void) response();
	}
}

#include <sys/dir.h>

rsource(name, mode)
	char *name;
	int mode;
{
	DIR *d = opendir(name);
	char *last;
	struct direct *dp;
	char buf[BUFSIZ];
	char *bufv[1];

	if (d == 0) {
		error("%s: %s\n", name, sys_errlist[errno]);
		return;
	}
	last = rindex(name, '/');
	if (last == 0)
		last = name;
	else
		last++;
	(void) sprintf(buf, "D%04o %d %s\n", mode&07777, 0, last);
	(void) remwrite(rem, buf, strlen(buf));
	if (response() < 0) {
		closedir(d);
		return;
	}
	while (dp = readdir(d)) {
		if (dp->d_ino == 0)
			continue;
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		if (strlen(name) + 1 + strlen(dp->d_name) >= BUFSIZ - 1) {
			error("%s/%s: Name too long.\n", name, dp->d_name);
			continue;
		}
		(void) sprintf(buf, "%s/%s", name, dp->d_name);
		bufv[0] = buf;
		source(1, bufv);
	}
	closedir(d);
	(void) remwrite(rem, "E\n", 2);
	(void) response();
}

response()
{
	char resp, c, rbuf[BUFSIZ], *cp = rbuf;

	if (read(rem, &resp, 1) != 1)
		lostconn();
	switch (resp) {

	case 0:
		return (0);

	default:
		*cp++ = resp;
		/* fall into... */
	case 1:
	case 2:
		do {
			if (read(rem, &c, 1) != 1)
				lostconn();
			*cp++ = c;
		} while (cp < &rbuf[BUFSIZ] && c != '\n');
		if (iamremote == 0)
			(void) write(2, rbuf, cp - rbuf);
		errs++;
		if (resp == 1)
			return (-1);
		exit(1);
	}
	/*NOTREACHED*/
}

lostconn()
{

	if (iamremote == 0)
		fprintf(stderr, "rcp: lost connection\n");
	exit(1);
}

sink(argc, argv)
	int argc;
	char **argv;
{
	char *targ;
	char cmdbuf[BUFSIZ], nambuf[BUFSIZ], buf[BUFSIZ], *cp;
	int of, mode, wrerr, exists, first;
	off_t i, size;
	char *whopp;
	struct stat stb; int targisdir = 0;
#define	SCREWUP(str)	{ whopp = str; goto screwup; }
	int mask = umask(0);
	char *myargv[1];

	umask(mask);
	if (argc > 1) {
		error("rcp: ambiguous target\n");	/*only 1 at a time*/
		exit(1);
	}
	targ = *argv;
	if (targetshouldbedirectory)
		verifydir(targ);
	ga();
/*
 *  Send null out over SOCK STREAM - either rem has been set with rcmd() or
 *  else it is the resulting rcp generating by the rcmd() so rem = 0
 *  (hopefully).  If the later is true, stdin has been redirected to
 *  the SOCK STREAM by the rcmd().
 */
	if (stat(targ, &stb) == 0 && (stb.st_mode & S_GFMT) == S_GFDIR)
		targisdir = 1;
	for (first = 1; ; first = 0) {
		cp = cmdbuf;
		if (read(rem, cp, 1) <= 0)
			return;
		if (*cp++ == '\n')
			SCREWUP("unexpected '\\n'");
		do {
			if (read(rem, cp, 1) != 1)
				SCREWUP("lost connection");
		} while (*cp++ != '\n');
		*cp = 0;
		if (cmdbuf[0] == '\01' || cmdbuf[0] == '\02') {
			if (iamremote == 0)
				(void) write(2, cmdbuf+1, strlen(cmdbuf+1));
			if (cmdbuf[0] == '\02')
				exit(1);
			errs++;
			continue;
		}
		*--cp = 0;
		cp = cmdbuf;
		if (*cp == 'E') {
			ga();
			return;
		}
		if (*cp != 'C' && *cp != 'D') {
			/*
			 * Check for the case "rcp remote:foo\* local:bar".
			 * In this case, the line "No match." can be returned
			 * by the shell before the rcp command on the remote is
			 * executed so the ^Aerror_message convention isn't
			 * followed.
			 */
			if (first) {
				error("%s\n", cp);
				exit(1);
			}
			SCREWUP("expected control record");
		}
		cp++;
		mode = 0;
		for (; cp < cmdbuf+5; cp++) {
			if (*cp < '0' || *cp > '7')
				SCREWUP("bad mode");
			mode = (mode << 3) | (*cp - '0');
		}
		if (*cp++ != ' ')
			SCREWUP("mode not delimited");
		size = 0;
		while (*cp >= '0' && *cp <= '9')
			size = size * 10 + (*cp++ - '0');
		if (*cp++ != ' ')
			SCREWUP("size not delimited");
		if (targisdir)
			(void) sprintf(nambuf, "%s%s%s", targ,
			    *targ ? "/" : "", cp);
		else
			(void) strcpy(nambuf, targ);
		exists = stat(nambuf, &stb) == 0;
		if (exists && ((stb.st_mode & S_GFMT) == S_GFPORT)) {
						/* file is named pipe
						 */
			error("rcp: %s: Not a plain file\n",nambuf);
			continue;
		}
		if (exists && access(nambuf, 2) < 0)
						/*file exists but no access
						 *allowed
						 */
			goto bad2;
		{ char *slash = rindex(nambuf, '/'), *dir;
						/*checking for dir access*/
		  if (slash == 0) { slash = "/";
			dir = ".";
		  } else if (slash == nambuf) {	/*check for root*/
			dir = "/";
		  } else {
			*slash = 0;
			dir = nambuf;
		  }
		  if (exists == 0 && access(dir, 2) < 0)
			goto bad;
		  *slash = '/';
		  if (cmdbuf[0] == 'D') {
			if (stat(nambuf, &stb) == 0) {
				if ((stb.st_mode&S_GFMT) != S_GFDIR) {
					errno = ENOTDIR;
					goto bad;
				}
			} else if (mkdir(nambuf, mode) < 0)
				goto bad;
			myargv[0] = nambuf;
			sink(1, myargv);
			continue;
		  }
		  if ((of = open(nambuf, O_WRONLY|O_CREAT, mode)) < 0) {
	bad:
			*slash = '/';
	bad2:
			error("rcp: %s: %s\n", nambuf, sys_errlist[errno]);
			continue;
		  }
		}
		if (exists == 0) {
			(void) stat(nambuf, &stb);
			(void) chown(nambuf, pwd->pw_uid, stb.st_gid);
			(void) chmod(nambuf, mode &~ mask);
		}
		ga();
		wrerr = 0;
		for (i = 0; i < size; i += BUFSIZ) {
			int amt = BUFSIZ;
			char *cp = buf;

			if (i + amt > size)
				amt = size - i;
			do {
				int j = read(rem, cp, amt);

				if (j <= 0)
					exit(1);
				amt -= j;
				cp += j;
			} while (amt > 0);
			amt = BUFSIZ;
			if (i + amt > size)
				amt = size - i;
			if (amt != 0 && wrerr == 0 && write(of, buf, amt) != amt)
				wrerr++;
		}
		ftruncate(of, size);
		(void) close(of);
		(void) response();
		if (wrerr)
			error("rcp: %s: %s\n", cp, sys_errlist[errno]);
		else
			ga();
	}
screwup:
	error("rcp: protocol screwup: %s\n", whopp);
	exit(1);
}

/*VARARGS*/
error(fmt, a1, a2, a3, a4, a5)
	char *fmt;
	int a1, a2, a3, a4, a5;
{
	char buf[BUFSIZ], *cp = buf;

	errs++;
	*cp++ = 1;
	(void) sprintf(cp, fmt, a1, a2, a3, a4, a5);
	(void) remwrite(rem, buf, strlen(buf));
	if (iamremote == 0)
		(void) write(2, buf+1, strlen(buf+1));
}

mkdir(name, mode)
	char *name;
	int mode;
{
	char *argv[4];
	int pid, rc;

	argv[0] = "mkdir";
	argv[1] = name;
	argv[2] = 0;
	pid = fork();
	if (pid < 0) {				/*if unable to fork, error*/
		perror("cp");
		return (1);
	}
	if (pid) {				/*parent executes following*/
		while (wait(&rc) != pid)	/*wailt til child is done*/
			continue;
		if (rc == 0)
			if (chmod(name, mode) < 0) {
				perror(name);
				rc = 1;
			}
		return (rc);
	}
	(void) setuid(getuid());
	execv("/bin/mkdir", argv);
	execv("/usr/bin/mkdir", argv);
	perror("mkdir");
	_exit(1);
	/*NOTREACHED*/
}

/* 
 * remwrite - error checking write, with retry
 *
 *	Used when writing over a socket where the write
 *	may fail do to the temporary condition of no
 *	mbufs.  After a large number of retrys give up.
 */
remwrite(fd, buff, many)
	int fd;
	char *buff; 
	int many;
{
	register int num;
	register unsigned cnt = 0;

	while ((num = write(fd, buff, many)) < 0
	      && errno == ENOBUFS
	      && cnt < MAXREMWRITES) {
		cnt++;
	}
#ifdef DEBUG
	if (cnt) {
		char message[128];
		sprintf(message,"rcp: remwrite: retried %d times.\n",
			cnt);
		write(dfd, message, strlen(message));
	}
#endif DEBUG

	if (num < 0)
		lostconn();	/* exit */

	return(num);
}

#ifdef DEBUG
initdebug() {
	int pid = getpid();
	char file1[128];

	sprintf(file1,"/tmp/RCP.%d",pid);
	close(creat(file1,0644));
	if ((dfd = open(file1,1)) < 0) {
		fprintf(stderr,"Cant open debug file %s.\n",file1);
		exit(255);
	}
}
#endif DEBUG
