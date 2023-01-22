
#ifndef lint
static	char	*sccsid = "@(#)man.c	1.6		1/8/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1983 by				*
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
#ifndef lint
/*	static char sccsid[] = "@(#)man.c	1.10 (Berkeley) 9/19/83"; */
#endif

#include <stdio.h>
#include <ctype.h>
#include <sgtty.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
/*
 * man
 * link also to apropos and whatis
 * This version uses more for underlining and paging.  now it uses page.(7/84)
 * Uses manpage macros in tmac.an.nopage to output to terminals without
 *  page numbers and their associated breaks (1/86)
 */
#define	MANDIR	"/usr/man"		/* this is deeply embedded */
#define	NROFFCAT "nroff -h -man.nopage"	/* for nroffing to cat file */
#define	NROFF	"nroff -man.nopage"	/* for nroffing to tty */
#define	MORE	"page -s"		/* paging filter */
#define	CAT	"cat -s"		/* for '-' opt (no more) */

#define TROFFCMD \
"troff -t -man.nopage /usr/lib/tmac/tmac.vcat %s | /usr/lib/rvsort |/usr/ucb/vpr -t"

/*  for troff:
#define	TROFFCMD "troff -man.nopage %s"
*/

char *allsect[] = {
    "1", "n", "l", "6", "8", "2", "3", "4", "5", "7", "p", "o", ""
};				/* order to look through sections */

char *sect1[] = {
    "1", "n", "l", "o", ""
};				/* sections to look at if 1 is specified */

char *subsec1[] = {
    "m", "c", "g", "p", "r", "v", "nfs", "yp", ""
};				/* subsections to try in section 1 */

char *subsec2[] = {
    "v", "nfs", "yp", ""
};

char *subsec3[] = {
    "j", "x", "m", "s", "n", "v", "c", "f", "nfs", "yp", ""
};

char *subsec4[] = {
    "p", "f", "v", "s", "n", ""
};

char *subsec8[] = {
    "v", "c", "nfs", "yp", ""
};

char *subsec0[] = {
    ""
};

int	nomore;
int	cflag;
char	*manpath = "/usr/man";
char	*strcpy();
char	*strcat();
char	*trim(), *section, *subsec;
int	remove();
int	troffit;
int	killtmp;

#define	eq(a,b)	(strcmp(a,b) == 0)

main(argc, argv)
	int argc;
	char *argv[];
{

	if (signal(SIGINT, SIG_IGN) == SIG_DFL) {
		signal(SIGINT, remove);
		signal(SIGQUIT, remove);
		signal(SIGTERM, remove);
	}
	umask(0);
	if (strcmp(argv[0], "apropos") == 0) {
		apropos(argc-1, argv+1);
		exit(0);
	}
	if (strcmp(argv[0], "whatis") == 0) {
		whatis(argc-1, argv+1);
		exit(0);
	}
	if (argc <= 1) {
		fprintf(stderr, "Usage: man [ section ] name ...\n");
		exit(1);
	}
	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		switch(argv[0][1]) {

		case 0:
			nomore++;
			break;

		case 't':
			troffit++;
			break;

		case 'k':
			apropos(argc-1, argv+1);
			exit(0);

		case 'f':
			whatis(argc-1, argv+1);
			exit(0);

		case 'P':
			argc--, argv++;
			manpath = *argv;
			break;
		}
		argc--, argv++;
	}
	if (chdir(manpath) < 0) {
		fprintf(stderr, "Can't chdir to %s.\n", manpath);
		exit(1);
	}
	if (troffit == 0 && nomore == 0 && !isatty(1))
		nomore++;
	section = "";
	do {
		if (eq(argv[0], "local")) {
			section = "l";
			goto sectin;
		} else if (eq(argv[0], "new")) {
			section = "n";
			goto sectin;
		} else if (eq(argv[0], "old")) {
			section = "o";
			goto sectin;
		} else if (eq(argv[0], "public")) {
			section = "p";
			goto sectin;
		} else if (argv[0][0] >= '0' && argv[0][0] <= '9'){
			section = argv[0];
			subsec = &argv[0][1];
sectin:
			argc--, argv++;
			if (argc == 0) {
				fprintf(stderr, "But what do you want from section %s?\n", argv[-1]);
				exit(1);
			}
			continue;
		}
		manual(section, argv[0]);
		argc--, argv++;
	} while (argc > 0);
	exit(0);
}

manual(sec, name)
	char *sec;
	char *name;
{
	char *section = sec;
	char work[100], work2[100], cmdbuf[150];
#ifdef	ultrix
	char	cwork[100];
	int	docatonly = 0;
	struct	stat	cstbuf;
#endif ultrix
	int ss;
	struct stat stbuf, stbuf2;
	int last;
	char **sp;

	sp = allsect;
#ifdef	ultrix
	strcpy(cwork,"cat");
#endif ultrix
	strcpy(work, "manx/");
	strcat(work, name);
	strcat(work, ".");
	last = strlen(work);
	if (*section == '1') {
		sp = sect1;
		section = "";
	}
	if (*section == '\0') {
		ss = 0;
		for (section = *sp++; *section; section = *sp, sp++) {
			work[3] = *section;
			work[last] = '\0';
			strcat(work, section);
			if (stat(work, &stbuf) >= 0)
				break;
#ifdef	ultrix
			strcpy(&cwork[3],&work[3]);
			if (stat(cwork, &cstbuf) >= 0) {
				docatonly = 1;
				break;
			}
#endif ultrix
			if (work[last] >= '1' && work[last] <= '8') {
				char **cp;
search:
				switch (work[last]) {
				case '1':
				case 'n':
				case 'l':
				case 'o': cp = subsec1; break;
				case '2':
				case '5': cp = subsec2; break;
				case '3': cp = subsec3; break;
				case '4': cp = subsec4; break;
				case '8': cp = subsec8; break;
				default: cp = subsec0;
				}
				while (**cp) {
				        work[last] = *section;
				        work[last+1] = '\0';
					strcat(work, *cp);
					cp++;
					if (stat(work, &stbuf) >= 0) {
						ss = work[last+1];
						goto found;
					}
#ifdef	ultrix
					strcpy(&cwork[3],&work[3]);
					if (stat(cwork, &cstbuf) >= 0) {
						ss = cwork[last+1];
						docatonly = 1;
						goto found;
					}
#endif ultrix
				}
				if (ss = 0)
					work[last+1] = 0;
			}
		}
		if (*section == '\0') {
			if (*sec == '\0')
				printf("No manual entry for %s.\n", name);
			else
				printf("No entry for %s in section %s of the manual.\n", name, sec);
			return;
		}
	} else {
		work[3] = *section;
		strcat(work, section);
#ifdef	ultrix
		strcpy(&cwork[3],&work[3]);
#endif
		if (stat(work, &stbuf) < 0) {
#ifdef	ultrix
			if(stat(cwork,&cstbuf) >= 0){
				docatonly = 1;
				goto found;
			}
#endif
			if ((*section >= '1' && *section <= '8') && *subsec == '\0') {
				*sp = "\0";
				goto search;
			}
			printf("No entry for %s in section %c", name, section[0]);
			if (*subsec)
				printf("%s", subsec);
			printf(" of the manual.\n");
			return;
		}
	}
found:
	if (troffit)
		troff(work);
	else {
		FILE *it;
		char abuf[BUFSIZ];

		if (!nomore) {
#ifdef	ultrix
		   if(!docatonly){
#endif	ultrix
			if ((it = fopen(work, "r")) == NULL) {
				perror(work);
				exit(1);
			}
			if (fgets(abuf, BUFSIZ-1, it) &&
			   abuf[0] == '.' && abuf[1] == 's' &&
			   abuf[2] == 'o' && abuf[3] == ' ') {
				register char *cp = abuf+strlen(".so ");
				char *dp;

				while (*cp && *cp != '\n')
					cp++;
				*cp = 0;
				while (cp > abuf && *--cp != '/')
					;
				dp = ".so man";
				if (cp != abuf+strlen(dp)+1) { /* bogus */
tohard:
					nomore = 1;
					strcpy(work, abuf+4);
					goto hardway;
				}
				for (cp = abuf; *cp == *dp && *cp; cp++, dp++)
					;
				if (*dp)
					goto tohard;
				strcpy(work, cp-3);
			}
			fclose(it);
#ifdef	ultrix
		    }
#endif
			strcpy(work2, "cat");
			strcpy(work2+3, work+3);
			work2[4] = 0;
	/* see if the cat? dir exist if so nroff the file into it if
	 * isn't already there 
	 */
#ifdef ultrix
			if (!docatonly && (stat(work2, &stbuf2) < 0))
#else ultrix
			if (stat(work2, &stbuf2) < 0)
#endif ultrix
				goto hardway;
			strcpy(work2+3, work+3);
#ifdef	ultrix
			if (!docatonly && (stat(work2, &stbuf2) < 0 || stbuf2.st_mtime < stbuf.st_mtime)) {
#else	ultrix
			if (stat(work2, &stbuf2) < 0 || stbuf2.st_mtime < stbuf.st_mtime) {
#endif ultrix
				printf("Reformatting page.  Wait...");
				fflush(stdout);
				unlink(work2);
				sprintf(cmdbuf,
			"%s %s > /tmp/man%d; trap '' 1 15; mv /tmp/man%d %s",
				    NROFFCAT, work, getpid(), getpid(), work2);
				if (system(cmdbuf)) {
					printf(" aborted (sorry)\n");
					remove();
					/*NOTREACHED*/
				}
				printf(" done\n");
			}
			strcpy(work, work2);
		}
hardway:
		nroff(work);
	}
}

nroff(cp)
	char *cp;
{
	char cmd[BUFSIZ];

	if (cp[0] == 'c')
		sprintf(cmd, "%s %s", nomore? CAT : MORE, cp);
	else
		sprintf(cmd, nomore? "%s %s" : "%s %s|%s", NROFF, cp, MORE);
	system(cmd);
}

troff(cp)
	char *cp;
{
	char cmdbuf[BUFSIZ];

	sprintf(cmdbuf, TROFFCMD, cp);
	system(cmdbuf);
}

any(c, sp)
	register int c;
	register char *sp;
{
	register int d;

	while (d = *sp++)
		if (c == d)
			return (1);
	return (0);
}

remove()
{
	char name[15];

	sprintf(name, "/tmp/man%d", getpid());
	unlink(name);
	exit(1);
}

apropos(argc, argv)
	int argc;
	char **argv;
{
	char buf[BUFSIZ];
	char *gotit;
	register char **vp;

	if (argc == 0) {
		fprintf(stderr, "apropos what?\n");
		exit(1);
	}
	if (freopen("/usr/lib/whatis", "r", stdin) == NULL) {
		perror("/usr/lib/whatis");
		exit (1);
	}
	gotit = (char *) calloc(1, blklen(argv));
	while (fgets(buf, sizeof buf, stdin) != NULL)
		for (vp = argv; *vp; vp++)
			if (match(buf, *vp)) {
				printf("%s", buf);
				gotit[vp - argv] = 1;
				for (vp++; *vp; vp++)
					if (match(buf, *vp))
						gotit[vp - argv] = 1;
				break;
			}
	for (vp = argv; *vp; vp++)
		if (gotit[vp - argv] == 0)
			printf("%s: nothing appropriate\n", *vp);
}

match(buf, str)
	char *buf, *str;
{
	register char *bp, *cp;

	bp = buf;
	for (;;) {
		if (*bp == 0)
			return (0);
		if (amatch(bp, str))
			return (1);
		bp++;
	}
}

amatch(cp, dp)
	register char *cp, *dp;
{

	while (*cp && *dp && lmatch(*cp, *dp))
		cp++, dp++;
	if (*dp == 0)
		return (1);
	return (0);
}

lmatch(c, d)
	char c, d;
{

	if (c == d)
		return (1);
	if (!isalpha(c) || !isalpha(d))
		return (0);
	if (islower(c))
		c = toupper(c);
	if (islower(d))
		d = toupper(d);
	return (c == d);
}

blklen(ip)
	register int *ip;
{
	register int i = 0;

	while (*ip++)
		i++;
	return (i);
}

whatis(argc, argv)
	int argc;
	char **argv;
{
	register char **avp;

	if (argc == 0) {
		fprintf(stderr, "whatis what?\n");
		exit(1);
	}
	if (freopen("/usr/lib/whatis", "r", stdin) == NULL) {
		perror("/usr/lib/whatis");
		exit (1);
	}
	for (avp = argv; *avp; avp++)
		*avp = trim(*avp);
	whatisit(argv);
	exit(0);
}

whatisit(argv)
	char **argv;
{
	char buf[BUFSIZ];
	register char *gotit;
	register char **vp;

	gotit = (char *)calloc(1, blklen(argv));
	while (fgets(buf, sizeof buf, stdin) != NULL)
		for (vp = argv; *vp; vp++)
			if (wmatch(buf, *vp)) {
				printf("%s", buf);
				gotit[vp - argv] = 1;
				for (vp++; *vp; vp++)
					if (wmatch(buf, *vp))
						gotit[vp - argv] = 1;
				break;
			}
	for (vp = argv; *vp; vp++)
		if (gotit[vp - argv] == 0)
			printf("%s: not found\n", *vp);
}

wmatch(buf, str)
	char *buf, *str;
{
	register char *bp, *cp;

	bp = buf;
again:
	cp = str;
	while (*bp && *cp && lmatch(*bp, *cp))
		bp++, cp++;
	if (*cp == 0 && (*bp == '(' || *bp == ',' || *bp == '\t' || *bp == ' '))
		return (1);
	while (isalpha(*bp) || isdigit(*bp))
		bp++;
	if (*bp != ',')
		return (0);
	bp++;
	while (isspace(*bp))
		bp++;
	goto again;
}

char *
trim(cp)
	register char *cp;
{
	register char *dp;

	for (dp = cp; *dp; dp++)
		if (*dp == '/')
			cp = dp + 1;
	if (cp[0] != '.') {
		if (cp + 3 <= dp && dp[-2] == '.' && any(dp[-1], "cosa12345678npP"))
			dp[-2] = 0;
		if (cp + 4 <= dp && dp[-3] == '.' && any(dp[-2], "13") && isalpha(dp[-1]))
			dp[-3] = 0;
	}
	return (cp);
}

system(s)
char *s;
{
	int status, pid, w;

	if ((pid = vfork()) == 0) {
		execl("/bin/sh", "sh", "-c", s, 0);
		_exit(127);
	}
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1)
		status = -1;
	return (status);
}
