#ifndef lint
static	char	*sccsid = "@(#)main.c	1.5	(ULTRIX)	12/10/84";
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
 * main.c
 *
 *	sccsid[] = "@(#)main.c	2.12 (Berkeley) 8/11/83";
 *
 *	17-Feb-84	mah. Added comments and copyright.
 *
 */

#include "rcv.h"
#include <sys/stat.h>

/*
 * Mail -- a mail program
 *
 * Startup -- interface with user.
 */

jmp_buf	hdrjmp;

/*
 * Find out who the user is, copy his mail file (if exists) into
 * /tmp/Rxxxxx and set up the message pointers.  Then, print out the
 * message headers and read user commands.
 *
 * Command line syntax:
 *	Mail [ -i ] [ -r address ] [ -h number ] [ -f [ name ] ]
 * or:
 *	Mail [ -i ] [ -r address ] [ -h number ] people ...
 */

main(argc, argv)
	char **argv;
{
	register char *ef;
	register int i, argp;
	int mustsend, uflag, hdrstop(), (*prevint)(), f;
	FILE *ibuf, *ftat;
	struct sgttyb tbuf;

#ifdef signal
	Siginit();
#endif

	/*
	 * Set up a reasonable environment.  We clobber the last
	 * element of argument list for compatibility with version 6,
	 * figure out whether we are being run interactively, set up
	 * all the temporary files, buffer standard output, and so forth.
	 */
	files[0].filep = stdin;
	files[0].flags = LIB_OPEN | KEEP_OPEN;
	files[1].filep = stdout;
	files[1].flags = LIB_OPEN | KEEP_OPEN;
	files[2].filep = stderr;
	files[2].flags = LIB_OPEN | KEEP_OPEN;
	uflag = 0;
	argv[argc] = (char *) -1;
#ifdef	GETHOST
	inithost();			/*get name of current host & put it
					 *in netmach table.
					 */
#endif	GETHOST
	mypid = getpid();		/*get current process id*/
	intty = isatty(0);		/*is stdin & stdout to a terminal(1)
					 *or not(0)?
					 */
	outtty = isatty(1);
	if (outtty) {			/*if terminal then get baud rate*/
		gtty(1, &tbuf);
		baud = tbuf.sg_ospeed;
	}
	else				/*else set to default(9600)*/
		baud = B9600;
	image = -1;

	/*
	 * Now, determine how we are being used.
	 * We successively pick off instances of -r, -h, -f, and -i.
	 * If called as "rmail" we note this fact for letter sending.
	 * If there is anything left, it is the base of the list
	 * of users to mail to.  Argp will be set to point to the
	 * first of these users.
	 */

	ef = NOSTR;			/*null string pointer*/
	argp = -1;			/*initialize argp & mustsend*/
	mustsend = 0;
	if (argc > 0 && **argv == 'r')	/*see if called as "rmail"; set
					 *rmail flag if true.
					 */
		rmail++;
	for (i = 1; i < argc; i++) {	/*strip off options*/

		/*
		 * If current argument is not a flag, then the
		 * rest of the arguments must be recipients.
		 */

		if (*argv[i] != '-') {	/*if no options then must be sending;
					 *set argp to indicate such.
					 */
			argp = i;
			break;
		}
		switch (argv[i][1]) {
		case 'r':		/*undocumented option*/
			/*
			 * Next argument is address to be sent along
			 * to the mailer.
			 */
			if (i >= argc - 1) {
				fprintf(stderr, "Address required after -r\n");
				exit(1);
			}
			mustsend++;
			rflag = argv[i+1];
			i++;
			break;

		case 'T':		/*undocumented option*/
			/*
			 * Next argument is temp file to write which
			 * articles have been read/deleted for netnews.
			 */
			if (i >= argc - 1) {
				fprintf(stderr, "Name required after -T\n");
				exit(1);
			}		/*create the temp file*/
			Tflag = argv[i+1];
			if ((f = creat(Tflag, 0600)) < 0) {
				perror(Tflag);
				exit(1);
			}
			close(f);	/*close the temp file*/
			i++;
			break;

		case 'u':
			/*
			 * Next argument is person to pretend to be.
			 */
			uflag++;
			if (i >= argc - 1) {
				fprintf(stderr, "Missing user name for -u\n");
				exit(1);
			}		/*set myname=name specified, ie.
					 *pretend to be the someone
					 */
			strcpy(myname, argv[i+1]);
			i++;
			break;

		case 'i':
			/*
			 * User wants to ignore interrupts.
			 * Set the variable "ignore"
			 */
					/*assign null string to ignore*/
			assign("ignore", "");
			break;

		case 'd':
			debug++;	/*debugging switch*/
			break;

		case 'h':
			/*
			 * Specified sequence number for network.
			 * This is the number of "hops" made so
			 * far (count of times message has been
			 * forwarded) to help avoid infinite mail loops.
			 */
			if (i >= argc - 1) {
				fprintf(stderr, "Number required for -h\n");
				exit(1);
			}
			mustsend++;
			hflag = atoi(argv[i+1]);
			if (hflag == 0) {
				fprintf(stderr, "-h needs non-zero number\n");
				exit(1);
			}
			i++;
			break;

		case 's':
			/*
			 * Give a subject field for sending from
			 * non terminal
			 */
			if (i >= argc - 1) {
				fprintf(stderr, "Subject req'd for -s\n");
				exit(1);
			}
			mustsend++;
			sflag = argv[i+1];
			i++;
			break;

		case 'f':
			/*
			 * User is specifying file to "edit" with Mail,
			 * as opposed to reading system mailbox.
			 * If no argument is given after -f, we read his
			 * mbox file in his home directory.
			 */
			if (i >= argc - 1)	/*set ef to mbox or to file
						 *specified.
						 */
				ef = mbox;
			else
				ef = argv[i + 1];
			i++;
			break;

		case 'n':
			/*
			 * User doesn't want to source /usr/lib/Mail.rc
			 */
			nosrc++;
			break;

		case 'N':
			/*
			 * Avoid initial header printing.
			 */
			noheader++;
			break;

		case 'v':
			/*
			 * Send mailer verbose flag
			 */
			assign("verbose", "");	/*assign null string to
						 *verbose.
						 */
			break;

		default:
			fprintf(stderr, "Unknown flag: %s\n", argv[i]);
			exit(1);
		}
	}

	/*
	 * Check for inconsistent arguments.
	 */

	if (ef != NOSTR && argp != -1) {	/*can't specify mailbox to read
						 *if sending mail
						 */
		fprintf(stderr, "Cannot give -f and people to send to.\n");
		exit(1);
	}
	if (mustsend && argp == -1) {		/*must indicate addressee if
						 *sending mail
						 */
		fprintf(stderr, "The flags you gave make no sense since you're not sending mail.\n");
		exit(1);
	}
	tinit();		/*give names to all the tmp files needed*/
	input = stdin;
	rcvmode = argp == -1;
	if (!nosrc)
		load(MASTER);	/*interperts the commands in /usr/lib/Mail.rc
				 *unless no sourcing was specified.
				 */
	load(mailrc);		/*interperts the commands in the user's
				 *~user/.mailrc.  notice this is done no matter
				 *whether no sourcing is specified.
				 */
	if (argp != -1) {
		mail(&argv[argp]);	/*send mail to indicated addressees
					 *and exit.
					 */

		/*
		 * why wait?
		 */

		exit(senderr);
	}

	/*
	 * Ok, we are reading mail.
	 * Decide whether we are editing a mailbox or reading
	 * the system mailbox, and open up the right stuff.
	 */

	if (ef != NOSTR) {	/*if ef is not the null string, then we are
				 *either looking at mbox or a specified file,
				 *therefore we must be editting
				 */
		char *ename;

		edit++;
		ename = expand(ef);	/*expand the file name*/
		if (ename != ef) {
			ef = (char *) calloc(1, strlen(ename) + 1);
			strcpy(ef, ename);
		}
		editfile = ef;
		strcpy(mailname, ef);
	}
	if (setfile(mailname, edit) < 0) {
		if (edit)	/*error reading mailfile.  If own mailfile
				 *state error.  Otherwise was not our own
				 *but /usr/spool/mail/user, so assumes no mail
				 *for that person.
				 */
			perror(mailname);
		else
			fprintf(stderr, "No mail for %s\n", myname);
		exit(1);
	}
	if (!edit && !noheader && value("noheader") == NOSTR) {
		if (setjmp(hdrjmp) == 0) {
			if ((prevint = sigset(SIGINT, SIG_IGN)) != SIG_IGN)
				sigset(SIGINT, hdrstop);
			announce(!0);	/*print out
					 * Mail version x date. ? for help.
					 */
			fflush(stdout);
			sigset(SIGINT, prevint);
		}
	}
	if (edit)
		newfileinfo();		/*print out msg info in mailfile*/
	if (!edit && msgCount == 0) {	/*if not editting and no msgs say so
					 *and exit.
					 */
		printf("No mail\n");
		fflush(stdout);
		exit(0);
	}
	commands();			/*interpert user commands*/
	if (!edit) {
		sigset(SIGHUP, SIG_IGN);
		sigset(SIGINT, SIG_IGN);
		sigset(SIGQUIT, SIG_IGN);
		quit();			/*msg clean up, ie untouched msgs go
					 *back to /usr/spool/mail/user or
					 *touched msgs go to ~user/mbox, etc.
					 */
	}
	exit(0);
}

/*
 * Interrupt printing of the headers.
 */
hdrstop()
{

	clrbuf(stdout);
	printf("\nInterrupt\n");
	fflush(stdout);
	longjmp(hdrjmp, 1);
}
