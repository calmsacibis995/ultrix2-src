#ifndef lint
static char *sccsid = "@(#)printjob.c	1.10	ULTRIX	10/31/86";
#endif

/*
 * printjob -- print jobs in the queue.
 *
 *	NOTE: the lock file is used to pass information to lpq and lprm.
 *	it does not need to be removed because file locks are dynamic.
 */

#include "lp.h"

#define DORETURN	0		/* absorb fork error */
#define DOABORT		1		/* abort if dofork fails */

static char	title[80];		/* ``pr'' title */
static FILE	*cfp;			/* control file */
static int	pfd;			/* printer file descriptor */
static int	ofd;			/* output filter file descriptor */
static int	lfd;			/* lock file descriptor */
static int	pid;			/* pid of lpd process */
static int	prchild;		/* id of pr process */
static int	child;			/* id of any filters */
static int	ofilter;		/* id of output filter, if any */
static int	tof;			/* true if at top of form */
static int	count;			/* Number of files actually printed */
static int	remote;			/* true if sending files to remote */
static int	inid;			/* initialization file descriptor */

static char	fromhost[32];		/* user's host machine */
static char	logname[32];		/* user's login name */
static char	jobname[32];		/* job or file name */
static char	ininame[32];		/* initialization file name */
static char	class[32];		/* classification field */
static char	width[10] = "-w";	/* page width in characters */
static char	length[10] = "-l";	/* page length in lines */
static char	pxwidth[10] = "-x";	/* page width in pixels */
static char	pxlength[10] = "-y";	/* page length in pixels */
static char	indent[10] = "-i0";	/* indentation size in characters */
long pidoff;
#define SAFE_SYSTEM "IFS= ;PATH=/usr/ucb:/bin:/usr/bin:/usr/lib;export PATH IFS ;"
char cmdbuf[1024];

printjob(begin)
int	begin;
{
	struct stat stb;
	register struct queue *q, **qp;
	struct queue **queue;
	register int i, nitems;
	extern int onintr();

	init();					/* set up capabilities */
	(void) write(1, "", 1);			/* ack that daemon is started */
	(void) close(1);			/* set up log file */
	(void) close(2);
	if((begin==1) && (RM==NULL))
		{
		inid = open(ininame,O_RDWR|O_CREAT,0660);	/* create a init file */
		write(inid, "init file\n", 10);
		(void) close(inid);
		}
	if (open(LF, O_WRONLY|O_APPEND) < 0)
		(void) open("/dev/null", O_WRONLY);
	dup(1);
	pid = getpid();				/* for use with lprm */
	setpgrp(0, pid);
	signal(SIGHUP, onintr);
	signal(SIGINT, onintr);
	signal(SIGQUIT, onintr);
	signal(SIGTERM, onintr);

	/*
	 * uses short form file names
	 */
	if (chdir(SD) < 0) {
		log("cannot chdir to %s", SD);
		exit(1);
	}
	if (stat(LO, &stb) == 0 && (stb.st_mode & 0100))
		exit(0);		/* printing disabled */
	lfd = open(LO, O_WRONLY|O_CREAT, 0644);
	if (lfd < 0) {
		log("cannot create %s", LO);
		exit(1);
	}
	if (flock(lfd, LOCK_EX|LOCK_NB) < 0) {
		if (errno == EWOULDBLOCK)	/* active deamon present */
			exit(0);
		log("cannot lock %s", LO);
		exit(1);
	}
	ftruncate(lfd, 0);
	/*
	 * write process id for others to know
	 */
	sprintf(line, "%u\n", pid);
	pidoff = i = strlen(line);
	if (write(lfd, line, i) != i) {
		log("cannot write daemon pid");
		exit(1);
	}
	/*
	 * search the spool directory for work and sort by queue order.
	 */
	if ((nitems = getq(&queue)) < 0) {
		log("can't scan spool directory %s", SD);
		exit(1);
	}
	if (nitems == 0)		/* no work to do */
		exit(0);
	if (stb.st_mode & 01) {		/* reset queue flag */
		if (fchmod(lfd, stb.st_mode & 0776) < 0)
			log("cannot chmod %s", LO);
	}
	openpr();		/* open printer or remote */
again:
	/*
	 * we found something to do now do it --
	 *    write the name of the current control file into the lock file
	 *    so the spool queue program can tell what we're working on
	 */
	for (qp = queue; nitems--; free((char *) q)) {
		q = *qp++;
		if (stat(q->q_name, &stb) < 0)
			continue;
	restart:
		(void) lseek(lfd, pidoff, 0);
		(void) sprintf(line, "%s\n", q->q_name);
		i = strlen(line);
		if (write(lfd, line, i) != i)
			log("can't write (%d) control file name %s", errno, LO);
		if (!remote)
			i = printit(q->q_name);
		else
			i = sendit(q->q_name);
		/*
		 * Check to see if we are supposed to stop printing or
		 * if we are to rebuild the queue.
		 */
		if (fstat(lfd, &stb) == 0) {
			if (stb.st_mode & 0100)
				goto done;
			if (stb.st_mode & 01) {
				for (free((char *) q); nitems--; free((char *) q))
					q = *qp++;
				if (fchmod(lfd, stb.st_mode & 0776) < 0)
					log("cannot chmod %s", LO);
				break;
			}
		}
		if (i == 0)		/* file ok and printed */
			count++;
		else if (i > 0) {	/* try reprinting the job */
			log("restarting");
			if (ofilter > 0) {
				kill(ofilter, SIGCONT);	/* to be sure */
				(void) close(ofd);
				while ((i = wait(0)) > 0 && i != ofilter)
					;
				ofilter = 0;
			}
			if (flock(pfd, LOCK_UN) < 0)
				log("cannot unlock (%d) file %s ", errno, LP);
			(void) close(pfd);	/* close printer */
			(void) lseek(lfd, pidoff, 0);
			if (write(lfd, "\n", 1) != 1)
				log("can't write (%d) control file name %s", errno, LO);
			openpr();		/* try to reopen printer */
			goto restart;
		}
	}
	free((char *) queue);
	/*
	 * search the spool directory for more work.
	 */
	if ((nitems = getq(&queue)) < 0) {
		log("can't scan spool directory %s", SD);
		exit(1);
	}
	if (nitems == 0) {		/* no more work to do */
	done:
		if (count > 0) {	/* Files actually printed */
			if (!SF && !tof)
				(void) write(ofd, FF, strlen(FF));
			if (TR != NULL)		/* output trailer */
				(void) write(ofd, TR, strlen(TR));
		}
		exit(0);
	}
	goto again;
}

char	fonts[4][50];	/* fonts for troff */

static char ifonts[4][18] = {
	"/usr/lib/vfont/R",
	"/usr/lib/vfont/I",
	"/usr/lib/vfont/B",
	"/usr/lib/vfont/S"
};

/*
 * The remaining part is the reading of the control file (cf)
 * and performing the various actions.
 * Returns 0 if everthing was OK, 1 if we should try to reprint the job and
 * -1 if a non-recoverable error occured.
 */
static
printit(file)
	char *file;
{
	register int i;
	int bombed = 0;

	/*
	 * open control file
	 */
	if ((cfp = fopen(file, "r")) == NULL) {
		log("open failure (%d) of control file %s", errno, file);
		return(0);
	}
	/*
	 * Reset troff fonts.
	 */
	for (i = 0; i < 4; i++)
		strcpy(fonts[i], ifonts[i]);

	/*
	 *      read the control file for work to do
	 *
	 *      file format -- first character in the line is a command
	 *      rest of the line is the argument.
	 *      valid commands are:
	 *
	 *		J -- "job name" on banner page
	 *		C -- "class name" on banner page
	 *              L -- "literal" user's name to print on banner
	 *		T -- "title" for pr
	 *		H -- "host name" of machine where lpr was done
	 *              P -- "person" user's login name
	 *              I -- "indent" amount to indent output
	 *              f -- "file name" name of text file to print
	 *		l -- "file name" text file with control chars
	 *		p -- "file name" text file to print with pr(1)
	 *		t -- "file name" troff(1) file to print
	 *		n -- "file name" ditroff(1) file to print
	 *		d -- "file name" dvi file to print
	 *		g -- "file name" plot(1G) file to print
	 *		v -- "file name" plain raster file to print
	 *		c -- "file name" cifplot file to print
	 *		x -- "file name" file to pass-thru to printer
	 *		1 -- "R font file" for troff
	 *		2 -- "I font file" for troff
	 *		3 -- "B font file" for troff
	 *		4 -- "S font file" for troff
	 *		N -- "name" of file (used by lpq)
	 *              U -- "unlink" name of file to remove
	 *                    (after we print it. (Pass 2 only)).
	 *		M -- "mail" to user when done printing
	 *
	 *      getline reads a line and expands tabs to blanks
	 */
	sprintf(&width[2], "%d", PW);
	sprintf(&length[2], "%d", PL);
	sprintf(&pxwidth[2], "%d", PX);
	sprintf(&pxlength[2], "%d", PY);

	/* pass 1 */
/*				*/
	while (getline(cfp))
		switch (line[0]) {
		case 'H':
			strcpy(fromhost, line+1);
			if (class[0] == '\0')
				strcpy(class, line+1);
			continue;

		case 'P':
			strcpy(logname, line+1);
			if (RS) {			/* restricted */
				if (getpwnam(logname) == (struct passwd *)0) {
					bombed = 2;
					sendmail(bombed);
					goto pass2;
				}
			}
			continue;

		case 'J':
			if (line[1] != '\0')
				strcpy(jobname, line+1);
			else
				strcpy(jobname, " ");
			continue;

		case 'C':
			if (line[1] != '\0')
				strcpy(class, line+1);
			else if (class[0] == '\0')
				gethostname(class, sizeof (class));
			continue;

		case 'T':	/* header title for pr */
			strcpy(title, line+1);
			continue;

		case 'L':	/* identification line */
			if (!SH)
				banner(line+1, jobname);
			continue;

		case '1':	/* troff fonts */
		case '2':
		case '3':
		case '4':
			if (line[1] != '\0')
				strcpy(fonts[line[0]-'1'], line+1);
			continue;
		case 'z':
		case 'Z':	/* page length */
			strcpy(length+2, line+1);
			continue;
		case 'w':
		case 'W':	/* page width */
			strcpy(width+2, line+1);
			continue;

		case 'I':	/* indent amount */
			strcpy(indent+2, line+1);
			continue;

		default:	/* some file to print */
			if ((i = print(line[0], line+1)) > 0) {
				(void) fclose(cfp);
				return(1);
			} else if (i < 0)
				bombed = 1;
			title[0] = '\0';
			continue;

		case 'N':
		case 'U':
		case 'M':
			continue;
		}

	/* pass 2 */

pass2:
	fseek(cfp, 0L, 0);
	while (getline(cfp))
		switch (line[0]) {
		case 'M':
			if (bombed != 2)		/* already sent if 2 */
				sendmail(bombed);
			continue;

		case 'U':
			(void) unlink(line+1);
		}
	/*
	 * clean-up incase another control file exists
	 */
	(void) fclose(cfp);
	(void) unlink(file);
	return(bombed ? -1 : 0);
}

/*
 * Print a file.
 * Set up the chain [ PR [ | {IF, OF} ] ] or {IF, RF, TF, NF, DF, CF, VF}.
 * Return -1 if a non-recoverable error occured, 1 if a recoverable error and
 * 0 if all is well.
 * Note: all filters take stdin as the file, stdout as the printer,
 * stderr as the log file, and must not ignore SIGINT.
 */
static
print(format, file)
	int format;
	char *file;
{
	register int i, n, fi, fo;
	register char *prog;
	char *av[15], buf[BUFSIZ];
	int pid, p[2], stopped = 0;
	union wait status;
	struct	stat	Inode;
	int	nmbr;
	char	*cp;
	char	*save_aft;

	if ((fi = open(file, O_RDONLY)) < 0) {
		log("open failure (%d) of control file %s", errno, file);
		return(-1);
	}
	if (!SF && !tof) {		/* start on a fresh page */
		(void) write(ofd, FF, strlen(FF));
		tof = 1;
	}
	if (IF == NULL && (format == 'f' || format == 'l')) {
		tof = 0;
		while ((n = read(fi, buf, BUFSIZ)) > 0)
			if (write(ofd, buf, n) != n) {
				(void) close(fi);
				return(1);
			}
		(void) close(fi);
		return(0);
	}
	switch (format) {
	case 'p':	/* print file using 'pr' */
		if (PP != NULL) {	/* use special print filter */
			prog = PR;
			av[0] = "pr";
			av[1] = width;
			av[2] = length;
			av[3] = "-h";
			av[4] = *title ? title : " ";
			av[5] = 0;
			n=6;
			break;
			}
		if (IF == NULL) {	/* use output filter */
			prog = PR;
			av[0] = "pr";
			av[1] = width;
			av[2] = length;
			av[3] = "-h";
			av[4] = *title ? title : " ";
			av[5] = 0;
			fo = ofd;
			goto start;
		}
		pipe(p);
		if ((prchild = dofork(DORETURN)) == 0) {	/* child */
			dup2(fi, 0);		/* file is stdin */
			dup2(p[1], 1);		/* pipe is stdout */
			for (i = 3; i < NOFILE; i++)
				(void) close(i);
			/* Check if prog is a shell command
	 	 	 */
			if (stat(prog, &Inode) < 0) {
				/* Make sure to insert arguments before pipe
		 	 	 * and save stuff after, or at end if no pipe
		 	 	 */
				if ((cp = index(prog, '|')) != 0) {
					save_aft = (char *)malloc (strlen(cp) + 1);
					if (!save_aft) {
						log("no memory, exiting ...");
						exit(2);
					}
					strcpy(save_aft, cp);
					*cp++ = '\0';
					cp = (char *)malloc (strlen(prog) + 1);
						if (!cp) {
							log("no memory, exiting ...");
							exit(2);
						}
					strcpy(cp, prog);
					free((char *)prog);
					prog = cp;
				}
				else
					save_aft[0] = '\0';
				/* Move all n-2 arguments into prog 
	 		 	 */
				for (nmbr = 1; nmbr < n-1; nmbr++) {
					prog = (char *)realloc (prog, strlen(prog) + strlen(av[nmbr]) + 2);
					if (!prog) {
						log("no memory, exiting ...");
						exit(2);
					}
					strcat(prog, av[nmbr]);
					strcat(prog, " ");
				}
				if (save_aft[0] != '\0') {
					prog = (char *)realloc (prog, strlen(prog) + strlen(save_aft) + 1);
					if (!prog) {
						log("no memory, exiting ...");
						exit(2);
					}
					strcat(prog, save_aft);
					free((char *)save_aft);
				}
				(void) sprintf(cmdbuf, "(%s%s)", SAFE_SYSTEM, prog);
	    			if ((nmbr = system(cmdbuf)) != 0) {
					log("system call failure on print filter %s", cmdbuf);
					exit(2);
	    			}
				exit(0); /* must exit like exec */
			}
			else {
				execl(PR, "pr", width, length, "-h", *title ? title : " ", 0);
				log("cannot execl %s", PR);
				exit(2);
			}
		}
		(void) close(p[1]);		/* close output side */
		(void) close(fi);
		if (prchild < 0) {
			prchild = 0;
			(void) close(p[0]);
			return(-1);
		}
		fi = p[0];			/* use pipe for input */
	case 'f':	/* print plain text file */
		prog = IF;
		av[1] = width;
		av[2] = length;
		av[3] = indent;
		n = 4;
		break;
	case 'l':	/* like 'f' but pass control characters */
		prog = IF;
		av[1] = "-c";
		av[2] = width;
		av[3] = length;
		av[4] = indent;
		n = 5;
		break;
	case 'r':	/* print a fortran text file */
		prog = RF;
		av[1] = width;
		av[2] = length;
		n = 3;
		break;
	case 'n':	/* print ditroff output */
		prog = NF;
		av[1] = "-c";
		av[2] = width;
		av[3] = length;
		av[4] = indent;
		n = 5;
		break;
	case 't':	/* print troff output */
	case 'd':	/* print tex output */
		(void) unlink(".railmag");
		if ((fo = creat(".railmag", FILMOD)) < 0) {
			log("cannot create .railmag");
			(void) unlink(".railmag");
		} else {
			for (n = 0; n < 4; n++) {
				if (fonts[n][0] != '/')
					(void) write(fo, "/usr/lib/vfont/", 15);
				(void) write(fo, fonts[n], strlen(fonts[n]));
				(void) write(fo, "\n", 1);
			}
			(void) close(fo);
		}
		prog = (format == 't') ? TF : (format == 'n') ? NF : DF;
		av[1] = pxwidth;
		av[2] = pxlength;
		n = 3;
		break;
	case 'c':	/* print cifplot output */
		prog = CF;
		av[1] = pxwidth;
		av[2] = pxlength;
		n = 3;
		break;
	case 'g':	/* print plot(1G) output */
		prog = GF;
		av[1] = pxwidth;
		av[2] = pxlength;
		n = 3;
		break;
	case 'v':	/* print raster output */
		prog = VF;
		av[1] = pxwidth;
		av[2] = pxlength;
		n = 3;
		break;
	case 'x':	/* print via pass-thru filter */
		prog = XF;
		n = 1;
		break;
	default:
		(void) close(fi);
		log("illegal format character '%c'", format);
		return(-1);
	}
	if ((av[0] = rindex(prog, '/')) != NULL)
		av[0]++;
	else
		av[0] = prog;
	av[n++] = "-n";
	av[n++] = logname;
	av[n++] = "-h";
	av[n++] = fromhost;
	av[n++] = AF;
	av[n] = 0;
	fo = pfd;
	if (ofilter > 0) {		/* kill output filter */
		write(ofd, "\031\1", 2);
		while ((pid = wait3(&status, WUNTRACED, 0)) > 0 && pid != ofilter)
			;
		if (status.w_stopval < 0) {
			(void) close(fi);
			log("output filter died (%d)", status.w_retcode);
			return(1);
		}
		stopped++;
	}
start:
	if ((child = dofork(DORETURN)) == 0) {	/* child */
		dup2(fi, 0);
		dup2(fo, 1);
		for (i = 3; i < NOFILE; i++)
			(void) close(i);
		/* Check if prog is a shell command
	 	 */
		if (stat(prog, &Inode) < 0) {
			/* Make sure to insert arguments before pipe
	 		 * and save stuff after, or at end if no pipe
	 		 */
			if ((cp = index(prog, '|')) != 0) {
				save_aft = (char *)malloc (strlen(cp) + 1);
				if (!save_aft) {
					log("no memory, exiting ...");
					exit(2);
				}
				strcpy(save_aft, cp);
				*cp++ = '\0';
				cp = (char *)malloc (strlen(prog) + 1);
					if (!cp) {
						log("no memory, exiting ...");
						exit(2);
					}
				strcpy(cp, prog);
				free((char *)prog);
				prog = cp;
			}
			else
				save_aft[0] = '\0';
			/* Move all n-2 arguments into prog 
	 		 */
			for (nmbr = 1; nmbr < n-1; nmbr++) {
				prog = (char *)realloc (prog, strlen(prog) + strlen(av[nmbr]) + 2);
				if (!prog) {
					log("no memory, exiting ...");
					exit(2);
				}
				strcat(prog, av[nmbr]);
				strcat(prog, " ");
			}
			if (save_aft[0] != '\0') {
				prog = (char *)realloc (prog, strlen(prog) + strlen(save_aft) + 1);
				if (!prog) {
					log("no memory, exiting ...");
					exit(2);
				}
				strcat(prog, save_aft);
				free((char *)save_aft);
			}
			(void) sprintf(cmdbuf, "(%s%s)", SAFE_SYSTEM, prog);
	    		if ((nmbr = system(cmdbuf)) != 0) {
				log("system call failure on %s", cmdbuf);
				exit(2);
	    		}
			exit(0); /* must exit like exec */
		}
		else {
			execv(prog, av);
			log("cannot execv %s", prog);
			exit(2);
		}
	}
	(void) close(fi);
	if (child < 0)
		status.w_retcode = 100;
	else
		while ((pid = wait(&status)) > 0 && pid != child)
			;
	child = 0;
	prchild = 0;
	if (stopped) { 		/* restart output filter */
		if (ofilter > 0) {
			kill(ofilter, SIGCONT);	/* to be sure */
			(void) close(ofd);
			while ((i = wait(0)) > 0 && i != ofilter)
				;
			ofilter = 0;
		}
		if (flock(pfd, LOCK_UN) < 0)
			log("cannot unlock (%d) file %s ", errno, LP);
		(void) close(pfd);	/* close printer */
		(void) lseek(lfd, pidoff, 0);
		if (write(lfd, "\n", 1) != 1)
			log("can't write (%d) control file name %s", errno, LO);
		openpr();
	}
	tof = 0;
	if (!WIFEXITED(status) || status.w_retcode > 1) {
		log("Daemon Filter '%c' Malfunction (%d)", format, status.w_retcode);
		return(-1);
	} else if (status.w_retcode == 1)
		return(1);
	tof = 1;
	return(0);
}

/*
 * Send the daemon control file (cf) and any data files.
 * Return -1 if a non-recoverable error occured, 1 if a recoverable error and
 * 0 if all is well.
 */
static
sendit(file)
	char *file;
{
	register int linelen, err = 0;
	char last[132];

	/*
	 * open control file
	 */
	if ((cfp = fopen(file, "r")) == NULL) {
		log("open failure (%d) of control file %s", errno, file);
		return(0);
	}
	/*
	 *      read the control file for work to do
	 *
	 *      file format -- first character in the line is a command
	 *      rest of the line is the argument.
	 *      commands of interest are:
	 *
	 *            a-z -- "file name" name of file to print
	 *              U -- "unlink" name of file to remove
	 *                    (after we print it. (Pass 2 only)).
	 */

	/*
	 * pass 1
	 */
	while (getline(cfp)) {
	again:
		if (line[0] >= 'a' && line[0] <= 'z') {
			strcpy(last, line);
			while (linelen = getline(cfp))
				if (strcmp(last, line))
					break;
			if ((err = sendfile('\3', last+1)) > 0) {
				(void) fclose(cfp);
				return(1);
			} else if (err)
				break;
			if (linelen)
				goto again;
			break;
		}
	}
	if (!err && sendfile('\2', file) > 0) {
		(void) fclose(cfp);
		return(1);
	}
	/*
	 * pass 2
	 */
	fseek(cfp, 0L, 0);
	while (getline(cfp))
		if (line[0] == 'U')
			(void) unlink(line+1);
	/*
	 * clean-up incase another control file exists
	 */
	(void) fclose(cfp);
	(void) unlink(file);
	return(0);
}

/*
 * Send a data file to the remote machine and spool it.
 * Return positive if we should try resending.
 */
static
sendfile(type, file)
	char type, *file;
{
	register int f, i, amt;
	struct stat stb;
	char buf[BUFSIZ];
	int sizerr;

	if ((f = open(file, O_RDONLY)) < 0 || fstat(f, &stb) < 0) {
		log("open failure (%d) of control file %s", errno, file);
		return(-1);
	}
	(void) sprintf(buf, "%c%d %s\n", type, stb.st_size, file);
	amt = strlen(buf);
	if (write(pfd, buf, amt) != amt) {
		(void) close(f);
		return(1);
	}
	if (noresponse()) {
		(void) close(f);
		return(1);
	}
	sizerr = 0;
	for (i = 0; i < stb.st_size; i += BUFSIZ) {
		amt = BUFSIZ;
		if (i + amt > stb.st_size)
			amt = stb.st_size - i;
		if (sizerr == 0 && read(f, buf, amt) != amt)
			sizerr = 1;
		if (write(pfd, buf, amt) != amt) {
			(void) close(f);
			return(1);
		}
	}
	(void) close(f);
	if (sizerr) {
		log("%s: changed size", file);
		(void) write(pfd, "\1", 1);  /* tell recvjob to ignore this file */
		return(-1);
	}
	if (write(pfd, "", 1) != 1)
		return(1);
	if (noresponse())
		return(1);
	return(0);
}

/*
 * Check to make sure there have been no errors and that both programs
 * are in sync with eachother.
 * Return non-zero if the connection was lost.
 */
static
noresponse()
{
	char resp;

	if (read(pfd, &resp, 1) != 1 || resp != '\0') {
		log("lost connection or error in recvjob");
		return(1);
	}
	return(0);
}

/*
 * Banner printing stuff
 */
static
banner(name1, name2)
	char *name1, *name2;
{
	time_t tvec;
	extern char *ctime();

	time(&tvec);
	if (!SF && !tof)
		(void) write(ofd, FF, strlen(FF));
	if (SB) {	/* short banner only */
		if (class[0]) {
			(void) write(ofd, class, strlen(class));
			(void) write(ofd, ":", 1);
		}
		(void) write(ofd, name1, strlen(name1));
		(void) write(ofd, "  Job: ", 7);
		(void) write(ofd, name2, strlen(name2));
		(void) write(ofd, "  Date: ", 8);
		(void) write(ofd, ctime(&tvec), 24);
		(void) write(ofd, "\n", 1);
	} else {	/* normal banner */
		(void) write(ofd, "\n\n\n", 3);
		scan_out(ofd, name1, '\0');
		(void) write(ofd, "\n\n", 2);
		scan_out(ofd, name2, '\0');
		if (class[0]) {
			(void) write(ofd,"\n\n\n",3);
			scan_out(ofd, class, '\0');
		}
		(void) write(ofd, "\n\n\n\n\t\t\t\t\tJob:  ", 15);
		(void) write(ofd, name2, strlen(name2));
		(void) write(ofd, "\n\t\t\t\t\tDate: ", 12);
		(void) write(ofd, ctime(&tvec), 24);
		(void) write(ofd, "\n", 1);
	}
	if (!SF)
		(void) write(ofd, FF, strlen(FF));
	tof = 1;
}

static char *
scnline(key, p, c)
	register char key, *p;
	char c;
{
	register scnwidth;

	for (scnwidth = WIDTH; --scnwidth;) {
		key <<= 1;
		*p++ = key & 0200 ? c : BACKGND;
	}
	return (p);
}

#define TRC(q)	(((q)-' ')&0177)

static
scan_out(scfd, scsp, dlm)
	int scfd;
	char *scsp, dlm;
{
	register char *strp;
	register nchrs, j;
	char outbuf[LINELEN+1], *sp, c, cc;
	int d, scnhgt;
	extern char scnkey[][HEIGHT];	/* in lpdchar.c */

	for (scnhgt = 0; scnhgt++ < HEIGHT+DROP; ) {
		strp = &outbuf[0];
		sp = scsp;
		for (nchrs = 0; ; ) {
			d = dropit(c = TRC(cc = *sp++));
			if ((!d && scnhgt > HEIGHT) || (scnhgt <= DROP && d))
				for (j = WIDTH; --j;)
					*strp++ = BACKGND;
			else
				strp = scnline(scnkey[c][scnhgt-1-d], strp, cc);
			if (*sp == dlm || *sp == '\0' || nchrs++ >= PW/(WIDTH+1)-1)
				break;
			*strp++ = BACKGND;
			*strp++ = BACKGND;
		}
		while (*--strp == BACKGND && strp >= outbuf)
			;
		strp++;
		*strp++ = '\n';	
		(void) write(scfd, outbuf, strp-outbuf);
	}
}

static
dropit(c)
	char c;
{
	switch(c) {

	case TRC('_'):
	case TRC(';'):
	case TRC(','):
	case TRC('g'):
	case TRC('j'):
	case TRC('p'):
	case TRC('q'):
	case TRC('y'):
		return (DROP);

	default:
		return (0);
	}
}

/*
 * sendmail ---
 *   tell people about job completion
 */
static
sendmail(bombed)
	int bombed;
{
	static int p[2];
	register int i;
	int stat;
	register char *cp;
	char buf[100];

	pipe(p);
	if ((stat = dofork(DORETURN)) == 0) {		/* child */
		dup2(p[0], 0);
		for (i = 3; i < NOFILE; i++)
			(void) close(i);
		if ((cp = rindex(MAIL, '/')) != NULL)
			cp++;
		else
			cp = MAIL;
		sprintf(buf, "%s@%s", line+1, fromhost);
		execl(MAIL, cp, buf, 0);
		exit(0);
	} else if (stat > 0) {				/* parent */
		dup2(p[1], 1);
		printf("To: %s@%s\n", line+1, fromhost);
		printf("Subject: printer job\n\n");
		printf("Your printer job ");
		if (*jobname)
			printf("(%s) ", jobname);
		switch (bombed) {
		case 0:
			printf("\ncompleted successfully\n");
			break;
		default:
		case 1:
			printf("\ncould not be printed\n");
			break;
		case 2:
			printf("\ncould not be printed without an account on %s\n", host);
			break;
		}
		fflush(stdout);
		(void) close(1);
	}
	(void) close(p[0]);
	(void) close(p[1]);
	wait(&stat);
}

/*
 * dofork - fork with retries on failure
 */
static
dofork(action)
	int action;
{
	register int i, pid;

	for (i = 0; i < 20; i++) {
		if ((pid = fork()) < 0) {
			sleep((unsigned)(i*i));
			continue;
		}
		/*
		 * Child should run as daemon instead of root
		 */
		if (pid == 0)
			setuid(DU);
		return(pid);
	}
	log("can't fork");

	switch (action) {
	case DORETURN:
		return (-1);
	default:
		log("bad action (%d) to dofork", action);
		/*FALL THRU*/
	case DOABORT:
		exit(1);
	}
	/*NOTREACHED*/
}

/*
 * Cleanup child processes when a signal is caught.
 */
static
onintr()
{
	kill(0, SIGINT);
	if (ofilter > 0)
		kill(ofilter, SIGCONT);
	while (wait(0) > 0)
		;
	exit(0);
}

static
init()
{
	int status;
/*	reset parameters	*/
sprintf(width,"-w");	/* page width in characters */
sprintf(length,"-l");	/* page length in lines */
sprintf(pxwidth,"-x");	/* page width in pixels */
sprintf(pxlength,"-y");	/* page length in pixels */
sprintf(indent,"-i0");	/* indentation size in characters */
/*				*/
	if ((status = pgetent(line, printer)) < 0)
		fatal("can't open printer description file");
	else if (status == 0)
		fatal("unknown printer");
	if ((LP = pgetstr("lp", &bp)) == NULL)
		LP = DEFDEVLP;
	if ((RP = pgetstr("rp", &bp)) == NULL)
		RP = DEFLP;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
	if ((ST = pgetstr("st", &bp)) == NULL)
		ST = DEFSTAT;
	if ((LF = pgetstr("lf", &bp)) == NULL)
		LF = DEFLOGF;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((DU = pgetnum("du")) < 0)
		DU = DEFUID;
	if ((FF = pgetstr("ff", &bp)) == NULL)
		FF = DEFFF;
	if ((PW = pgetnum("pw")) < 0)
		PW = DEFWIDTH;
	sprintf(&width[2], "%d", PW);
	if ((PL = pgetnum("pl")) < 0)
		PL = DEFLENGTH;
	sprintf(&length[2], "%d", PL);
	if ((PX = pgetnum("px")) < 0)
		PX = 0;
	sprintf(&pxwidth[2], "%d", PX);
	if ((PY = pgetnum("py")) < 0)
		PY = 0;
	sprintf(&pxlength[2], "%d", PY);
	sprintf(ininame, "%s/%s",SD,DEFINIT);	/* setup init file name */
	RM = pgetstr("rm", &bp);
	TS = pgetstr("ts", &bp);
	OP = pgetstr("op", &bp);
	OS = pgetstr("os", &bp);
	AF = pgetstr("af", &bp);
	OF = pgetstr("of", &bp);
	IF = pgetstr("if", &bp);
	PP = pgetstr("pp", &bp); /* substitute print filter to replace pr */
	RF = pgetstr("rf", &bp);
	TF = pgetstr("tf", &bp);
	NF = pgetstr("nf", &bp);
	DF = pgetstr("df", &bp);
	GF = pgetstr("gf", &bp);
	VF = pgetstr("vf", &bp);
	CF = pgetstr("cf", &bp);
	TR = pgetstr("tr", &bp);
	XF = pgetstr("xf", &bp);
	RS = pgetflag("rs");
	SF = pgetflag("sf");
	SH = pgetflag("sh");
	SB = pgetflag("sb");
	RW = pgetflag("rw");
	BR = pgetnum("br");
	if ((FC = pgetnum("fc")) < 0)
		FC = 0;
	if ((FS = pgetnum("fs")) < 0)
		FS = 0;
	if ((XC = pgetnum("xc")) < 0)
		XC = 0;
	if ((XS = pgetnum("xs")) < 0)
		XS = 0;
	tof = !pgetflag("fo");
}

/*
 * Acquire line printer or remote connection.
 */
static
openpr()
{
	register int i, n;
	int p[2];
	char *cp;
	struct	stat	Inode;
	int	nmbr;
	char	*save_aft;

	if (*TS) {
	    status("waiting for %s to connect", printer);
	    pfd = lat_conn(LP,TS,OP,OS);
 
	    if (pfd < 0)
	    {
		log("cannot open %s",LP);
		exit(1);
	    }
	    setty(pfd);
	    status("%s is ready and printing",printer);
	}
	else
	if (*LP) {
		for (i = 1; ; i = i < 32 ? i << 1 : i) {
			pfd = open(LP, RW ? O_RDWR | O_CREAT : O_WRONLY | O_CREAT, 0644);
			if (pfd >= 0)
				break;
			if (errno == ENOENT) {
				log("cannot open %s", LP);
				exit(1);
			}
			if (i == 1)
				status("waiting for %s to become ready (offline ?)", printer);
			sleep(i);
		}
		if ((i = flock(pfd, LOCK_EX | LOCK_NB)) < 0) {
	 		if (errno == EWOULDBLOCK) {
	 			status("waiting for lock on %s", LP);
	 			i = flock(pfd, LOCK_EX);
	 		}
	 		if (i < 0) {
	 			log("broken lock on %s", LP);
	 			exit(1);
	 		}
	 	}
		if (isatty(pfd))
			setty();
		status("%s is ready and printing", printer);
	} else if (RM != NULL) {
		for (i = 1; ; i = i < 512 ? i << 1 : i) {
			pfd = getport(RM);
			if (pfd >= 0) {
				(void) sprintf(line, "\2%s\n", RP);
				n = strlen(line);
				if (write(pfd, line, n) != n)
					break;
				if (noresponse()) {
					if (flock(pfd, LOCK_UN) < 0) {
						log("cannot unlock %s", LP);
						exit(1);
					}
					(void) close(pfd);
				}
				else
					break;
			}
			if (i == 1)
				status("waiting for %s to come up", RM);
			sleep(i);
		}
		status("sending to %s", RM);
		remote = 1;
	} else {
		log("no line printer device or remote machine name");
		exit(1);
	}
	/*
	 * Start up an output filter, if needed.
	 */
	if (OF) {
		pipe(p);
		if ((ofilter = dofork(DOABORT)) == 0) {	/* child */
			dup2(p[0], 0);		/* pipe is std in */
			dup2(pfd, 1);		/* printer is std out */
			for (i = 3; i < NOFILE; i++)
				(void) close(i);
			/* Check if OF is a shell command
	 	 	 */
			if (stat(OF, &Inode) < 0) {
				/* Make sure to insert arguments before pipe
		 	 	 * and save stuff after, or at end if no pipe
		 	 	 */
				if ((cp = index(OF, '|')) != 0) {
					save_aft = (char *)malloc (strlen(cp) + 1);
					if (!save_aft) {
						log("no memory, exiting ...");
						exit(2);
					}
					strcpy(save_aft, cp);
					*cp++ = '\0';
					cp = (char *)malloc (strlen(OF) + 1);
						if (!cp) {
							log("no memory, exiting ...");
							exit(2);
						}
					strcpy(cp, OF);
					free((char *)OF);
					OF = cp;
				}
				else
					save_aft[0] = '\0';
				/* Move arguments into OF 
		 	 	 */

				OF = (char *)realloc (OF, strlen(OF) + strlen(width) + strlen(length) + 3);
				if (!OF) {
					log("no memory, exiting ...");
					exit(2);
				}
				strcat(OF, width);
				strcat(OF, " ");
				strcat(OF, length);
				strcat(OF, " ");
				if (save_aft[0] != '\0') {
					OF = (char *)realloc (OF, strlen(OF) + strlen(save_aft) + 1);
					if (!OF) {
						log("no memory, exiting ...");
						exit(2);
					}
					strcat(OF, save_aft);
					free((char *)save_aft);
				}
				(void) sprintf(cmdbuf, "(%s%s)", SAFE_SYSTEM, OF);
	    			if ((nmbr = system(cmdbuf)) != 0) {
					log("system call failure on output filter %s", cmdbuf);
					exit(2);
	    			}
				exit(0); /* must exit like exec */
			}
			else {
				if ((cp = rindex(OF, '/')) == NULL)
					cp = OF;
				else
					cp++;
				execl(OF, cp, width, length, 0);
				log("can't execl output filter %s", OF);
				exit(1);
			}
		}
		(void) close(p[0]);		/* close input side */
		ofd = p[1];			/* use pipe for output */
	}
	else {
		ofd = pfd;
		ofilter = 0;
	}
}

struct bauds {
	int	baud;
	int	speed;
} bauds[] = {
	50,	B50,
	75,	B75,
	110,	B110,
	134,	B134,
	150,	B150,
	200,	B200,
	300,	B300,
	600,	B600,
	1200,	B1200,
	1800,	B1800,
	2400,	B2400,
	4800,	B4800,
	9600,	B9600,
	19200,	EXTA,
	38400,	EXTB,
	0,	0
};

/*
 * setup tty lines.
 */
static
setty()
{
	struct sgttyb ttybuf;
	register struct bauds *bp;

	if (ioctl(pfd, TIOCGETP, (char *)&ttybuf) < 0) {
		log("cannot get tty parameters");
		exit(1);
	}
	if (BR > 0) {
		for (bp = bauds; bp->baud; bp++)
			if (BR == bp->baud)
				break;
		if (!bp->baud) {
			log("illegal baud rate %d", BR);
			exit(1);
		}
		ttybuf.sg_ispeed = ttybuf.sg_ospeed = bp->speed;
	}
	ttybuf.sg_flags &= ~FC;
	ttybuf.sg_flags |= FS;
	if (ioctl(pfd, TIOCSETP, (char *)&ttybuf) < 0) {
		log("cannot set tty parameters");
		exit(1);
	}
	if (XC) {
		if (ioctl(pfd, TIOCLBIC, &XC) < 0) {
			log("cannot set local tty parameters");
			exit(1);
		}
	}
	if (XS) {
		if (ioctl(pfd, TIOCLBIS, &XS) < 0) {
			log("cannot set local tty parameters");
			exit(1);
		}
	}
}

/*VARARGS1*/
static
status(msg, a1, a2, a3)
	char *msg;
{
	register int fd;
	char buf[BUFSIZ];

	umask(0);
	fd = open(ST, O_WRONLY|O_CREAT, 0664);
	if (fd < 0 || flock(fd, LOCK_EX) < 0)
		fatal("cannot create status file");
	ftruncate(fd, 0);
	sprintf(buf, msg, a1, a2, a3);
	strcat(buf, "\n");
	(void) write(fd, buf, strlen(buf));
	(void) close(fd);
}
