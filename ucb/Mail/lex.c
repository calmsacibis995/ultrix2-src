#ifndef lint
static	char	*sccsid = "@(#)lex.c	1.4	(ULTRIX)	12/10/84";
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
 * lex.c
 *
 *	sccsid[] = "@(#)lex.c	2.14 (Berkeley) 8/11/83";
 *
 *	17-Feb-84	mah. Added comments and copyright.
 *
 */

#include "rcv.h"
/*
 * Mail -- a mail program
 *
 * Lexical processing of commands.
 */

char	*prompt = "& ";

/*
 * Set up editing on the given file name.
 * If isedit is true, we are considered to be editing the file,
 * otherwise we are reading our mail which has signficance for
 * mbox and so forth.
 */

setfile(name, isedit)
	char *name;
{
	FILE *ibuf;
	int i;
	static int shudclob;
	static char efile[128];
	extern char tempMesg[];

	if ((ibuf = fopen(name, "r")) == NULL)
		return(-1);
	files[fileno(ibuf)].filep = ibuf;
	files[fileno(ibuf)].flags = LIB_OPEN & ~KEEP_OPEN;
	/*
	 * Looks like all will be well.  We must now relinquish our
	 * hold on the current set of stuff.  Must hold signals
	 * while we are reading the new file, else we will ruin
	 * the message[] data structure.
	 */

	holdsigs();
	if (shudclob) {
		if (edit)
			edstop();
		else
			quit();
	}

	/*
	 * Copy the messages into /tmp
	 * and set pointers.
	 */

	readonly = 0;
	if ((i = open(name, 1)) < 0)
		readonly++;
	else
		close(i);
	if (shudclob) {
		fclose(itf);
		fclose(otf);
	}
	shudclob = 1;
	edit = isedit;
	strncpy(efile, name, 128);
	editfile = efile;
	if (name != mailname)
		strcpy(mailname, name);
	mailsize = fsize(ibuf);
	if ((otf = fopen(tempMesg, "w")) == NULL) {
		perror(tempMesg);
		exit(1);
	}
	files[fileno(otf)].filep = otf;
	files[fileno(otf)].flags = LIB_OPEN | KEEP_OPEN;
	if ((itf = fopen(tempMesg, "r")) == NULL) {
		perror(tempMesg);
		exit(1);
	}
	files[fileno(itf)].filep = itf;
	files[fileno(itf)].flags = LIB_OPEN | KEEP_OPEN;
	remove(tempMesg);
	setptr(ibuf);
	setmsize(msgCount);
	fclose(ibuf);
	relsesigs();
	sawcom = 0;
	return(0);
}

/*
 * Interpret user commands one by one.  If standard input is not a tty,
 * print no prompt.
 */

int	*msgvec;

commands()
{
	int eofloop, shudprompt, stop();
	register int n;
	char linebuf[LINESIZE];
	int hangup(), contin();

# ifdef VMUNIX
	sigset(SIGCONT, SIG_DFL);
# endif VMUNIX
	if (rcvmode && !sourcing) {
		if (sigset(SIGINT, SIG_IGN) != SIG_IGN)
			sigset(SIGINT, stop);
		if (sigset(SIGHUP, SIG_IGN) != SIG_IGN)
			sigset(SIGHUP, hangup);
	}
	shudprompt = intty && !sourcing;/*stdin (terminal or not) & not
					 *sourcing
					 */
	for (;;) {
		setexit();		/*setjmp(srbuf)*/

		/*
		 * Print the prompt, if needed.  Clear out
		 * string space, and flush the output.
		 */

		if (!rcvmode && !sourcing)
					/*if not receiving and not sourcing,
					 *then return.
					 */
			return;
		eofloop = 0;		/*initialize eof loop counter*/
top:
		if (shudprompt) {	/*if terminal and not sourcing put out
					 *prompt.
					 */
			printf(prompt);
			flush();
# ifdef VMUNIX
					/*set action routine for "continue
					 *after stop" signal.
					 */
			sigset(SIGCONT, contin);
# endif VMUNIX
		} else
			flush();
		sreset();		/*initial extra host hash table and
					 *reset string area = empty.
					 */

		/*
		 * Read a line of commands from the current input
		 * and handle end of file specially.
		 */

		n = 0;
		for (;;) {
			if (readline(input, &linebuf[n]) <= 0) {
				if (n != 0)
					/*break out to sigset*/
					break;
				if (loading)
					/*if loading a file return*/
					return;
				if (sourcing) {
					unstack();
					/*continue at outer for_loop*/
					goto more;
				}
				if (value("ignoreeof") != NOSTR && shudprompt) {
					/*if ignoring eof and (stdin&~sourcing)
					 *doesn't = 0, then will tell proper
					 *way to exit (up to 25 times).
					 */
					if (++eofloop < 25) {
						printf("Use \"quit\" to quit.\n");
					/*continue at prompt check*/
						goto top;
					}
				}
				if (edit)
					/*terminate the editing session*/
					edstop();
				return;	/*exit command routine*/
			}
			if ((n = strlen(linebuf)) == 0)
					/*if nothing on line, then break out to
					 *sigset
					 */
				break;
			n--;		/*don't coutn newline*/
			if (linebuf[n] != '\\')
					/*if not a continuator break*/
				break;
					/*subsitute continuator with space*/
			linebuf[n++] = ' ';
		}
# ifdef VMUNIX
				/*discard "continue after stop" signals*/
		sigset(SIGCONT, SIG_DFL);
# endif VMUNIX
					/*execute command*/
		if (execute(linebuf, 0))
			return;
more:		;
	}
}

/*
 * Execute a single command.  If the command executed
 * is "quit," then return non-zero so that the caller
 * will know to return back to main, if he cares.
 * Contxt is non-zero if called while composing mail.
 */

execute(linebuf, contxt)
	char linebuf[];
{
	char word[LINESIZE];
	char *arglist[MAXARGC];
	struct cmd *com;
	register char *cp, *cp2;
	register int c;
	int muvec[2];
	int edstop(), e;

	/*
	 * Strip the white space away from the beginning
	 * of the command, then scan out a word, which
	 * consists of anything except digits and white space.
	 *
	 * Handle ! escapes differently to get the correct
	 * lexical conventions.
	 */

	cp = linebuf;			/*cp pts to command to be processed*/
	while (any(*cp, " \t"))		/*strip all spaces and tabs*/
		cp++;
	if (*cp == '!') {
		if (sourcing) {	/*if sourcing no shell commands allowed*/
			printf("Can't \"!\" while sourcing\n");
			unstack();
			return(0);
		}
		shell(cp+1);		/*execute shell command*/
		return(0);
	}
	cp2 = word;
	while (*cp && !any(*cp, " \t0123456789$^.:/-+*'\""))
					/*while there are characters and no
					 *tabs, numbers, non-alpha characters;
					 *copy into word[].
					 */
		*cp2++ = *cp++;
	*cp2 = '\0';			/*terminate command string*/

	/*
	 * Look up the command; if not found, bitch.
	 * Normally, a blank command would map to the
	 * first command in the table; while sourcing,
	 * however, we ignore blank lines to eliminate
	 * confusion.
	 */

	if (sourcing && equal(word, ""))	/*make sure there's a command
						 *to process.
						 */
		return(0);
	com = lex(word);		/*return ptr to struct com for passed
					 *command.
					 */
	if (com == NONE) {		/*command not found in command table*/
		printf("Unknown command: \"%s\"\n", word);
		if (loading)
			return(1);
		if (sourcing)
			unstack();
		return(0);
	}

	/*
	 * See if we should execute the command -- if a conditional
	 * we always execute it, otherwise, check the state of cond.
	 */

	if ((com->c_argtype & F) == 0)	/*if a "conditinal" command then check
					 *for inconsistency in cond with
					 *rcvmond, eg. cond=receive only and not
					 *in receive mode.  Return, ie ignore
					 *command if inconsistent.
					 */
		if (cond == CRCV && !rcvmode || cond == CSEND && rcvmode)
			return(0);

	/*
	 * Special case so that quit causes a return to
	 * main, who will call the quit code directly.
	 * If we are in a source file, just unstack.
	 */

	if (com->c_func == edstop && sourcing) {
					/*if (func=quit edit session) and you
					 *are sourcing check to see if "loading"
					 *a file; if so return leaving stack as
					 *is; if not "loading pop stack back 1
					 *level and return.
					 */
		if (loading)
			return(1);
		unstack();
		return(0);
	}
	if (!edit && com->c_func == edstop) {
					/*if not editing + (func=quit session),
					 *set interrupt routine to ignore + ret.
					 */
		sigset(SIGINT, SIG_IGN);
		return(1);
	}

	/*
	 * Process the arguments to the command, depending
	 * on the type he expects.  Default to an error.
	 * If we are sourcing an interactive command, it's
	 * an error.
	 */

	if (!rcvmode && (com->c_argtype & M) == 0) {
					/*if not receiving and the argument type
					 *equals send mode, print err msg and
					 *clean up accordingly.
					 */
		printf("May not execute \"%s\" while sending\n",
		    com->c_name);
		if (loading)
			return(1);
		if (sourcing)
			unstack();
		return(0);
	}
	if (sourcing && com->c_argtype & I) {
					/*can't do interactive commands when
					 *sourcing.
					 */
		printf("May not execute \"%s\" while sourcing\n",
		    com->c_name);
		if (loading)
			return(1);
		unstack();
		return(0);
	}
	if (readonly && com->c_argtype & W) {
					/*illegal command when read only*/
		printf("May not execute \"%s\" -- message file is read only\n",
		   com->c_name);
		if (loading)
			return(1);
		if (sourcing)
			unstack();
		return(0);
	}
	if (contxt && com->c_argtype & R) {
					/*if the contxt in which execute was
					 *called = 1, ie called by collect
					 *(the ~ processor) and command is type
					 *that can't be executed by collect,
					 *then return.
					 */
		printf("Cannot recursively invoke \"%s\"\n", com->c_name);
		return(0);
	}
	e = 1;
	switch (com->c_argtype & ~(F|P|I|M|T|W|R)) {
					/*look for "list" type*/
	case MSGLIST:
		/*
		 * A message list defaulting to nearest forward
		 * legal message.
		 */
		if (msgvec == 0) {
			printf("Illegal use of \"message list\"\n");
			return(-1);
		}
		if ((c = getmsglist(cp, msgvec, com->c_msgflag)) < 0)
			break;
		if (c  == 0) {
					/*see if msg to perform command on*/
			*msgvec = first(com->c_msgflag,
				com->c_msgmask);
			msgvec[1] = NULL;
		}
		if (*msgvec == NULL) {
			printf("No applicable messages\n");
			break;
		}
		e = (*com->c_func)(msgvec);	/*ok, go do it*/
		break;

	case NDMLIST:
		/*
		 * A message list with no defaults, but no error
		 * if none exist.
		 */
		if (msgvec == 0) {
			printf("Illegal use of \"message list\"\n");
			return(-1);
		}
		if (getmsglist(cp, msgvec, com->c_msgflag) < 0)
			break;
		e = (*com->c_func)(msgvec);
		break;

	case STRLIST:
		/*
		 * Just the straight string, with
		 * leading blanks removed.
		 */
		while (any(*cp, " \t"))
			cp++;
		e = (*com->c_func)(cp);
		break;

	case RAWLIST:
		/*
		 * A vector of strings, in shell style.
		 */
		if ((c = getrawlist(cp, arglist)) < 0)
			break;			/*if # args < 0, forget it*/
		if (c < com->c_minargs) {	/*c_minargs same as c_msgflag*/
			printf("%s requires at least %d arg(s)\n",
				com->c_name, com->c_minargs);
			break;
		}
		if (c > com->c_maxargs) {	/*c_maxargs same as c_msgmask*/
			printf("%s takes no more than %d arg(s)\n",
				com->c_name, com->c_maxargs);
			break;
		}
		e = (*com->c_func)(arglist);	/*do command*/
		break;

	case NOLIST:
		/*
		 * Just the constant zero, for exiting,
		 * eg.
		 */
		e = (*com->c_func)(0);
		break;

	default:
		panic("Unknown argtype");
	}

	/*
	 * Exit the current source file on
	 * error.
	 */

	if (e && loading)
		return(1);
	if (e && sourcing)
		unstack();
	if (com->c_func == edstop)
		return(1);
	if (value("autoprint") != NOSTR && com->c_argtype & P)
		if ((dot->m_flag & MDELETED) == 0) {
			muvec[0] = dot - &message[0] + 1;
			muvec[1] = 0;
			type(muvec);
		}
	if (!sourcing && (com->c_argtype & T) == 0)
		sawcom = 1;
	return(0);
}

/*
 * When we wake up after ^Z, reprint the prompt.
 */
contin(s)
{

	printf(prompt);
	fflush(stdout);
}

/*
 * Branch here on hangup signal and simulate quit.
 */
hangup()
{

	holdsigs();
	if (edit) {
		if (setexit())
			exit(0);
		edstop();
	}
	else
		quit();
	exit(0);
}

/*
 * Set the size of the message vector used to construct argument
 * lists to message list functions.
 */
 
setmsize(sz)
{

	if (msgvec != (int *) 0)
		cfree(msgvec);
	msgvec = (int *) calloc((unsigned) (sz + 1), sizeof *msgvec);
}

/*
 * Find the correct command in the command table corresponding
 * to the passed command "word"
 */

struct cmd *
lex(word)
	char word[];
{
	register struct cmd *cp;
	extern struct cmd cmdtab[];

	for (cp = &cmdtab[0]; cp->c_name != NOSTR; cp++)
		if (isprefix(word, cp->c_name))
			return(cp);
	return(NONE);
}

/*
 * Determine if as1 is a valid prefix of as2.
 * Return true if yep.
 */

isprefix(as1, as2)
	char *as1, *as2;
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ == *s2)
		if (*s2++ == '\0')
			return(1);
	return(*--s1 == '\0');
}

/*
 * The following gets called on receipt of a rubout.  This is
 * to abort printout of a command, mainly.
 * Dispatching here when command() is inactive crashes rcv.
 * Close all open files except 0, 1, 2, and the temporary.
 * Also, unstack all source files.
 */

int	inithdr;			/* am printing startup headers */

stop(s)
{
	register int fp;

# ifndef VMUNIX
	s = SIGINT;
# endif VMUNIX
	noreset = 0;
	if (!inithdr)
		sawcom++;
	inithdr = 0;
	while (sourcing)
		unstack();
	for (fp = 0; fp < _NFILE; fp++) 
		if (!(files[fp].flags & KEEP_OPEN))
			switch(files[fp].flags) {
				case LIB_OPEN:
					fclose(files[fp].filep);
					break;
				case SYS_OPEN:
					close(files[fp].filed);
					break;
				case PIPE_OPEN:
					pclose(files[fp].filep);
					pipef = NULL;
					break;
			}
	if (image >= 0)
		image = -1;
	clrbuf(stdout);
	printf("Interrupt\n");
# ifndef VMUNIX
	signal(s, stop);
# endif
	reset(0);
}

/*
 * Announce the presence of the current Mail version,
 * give the message count, and print a header listing.
 */

char	*greeting	= "Mail version %s.  Type ? for help.\n";

announce(pr)
{
	int vec[2], mdot;
	extern char *version;

	if (pr && value("quiet") == NOSTR)
		printf(greeting, version);
	mdot = newfileinfo();
	vec[0] = mdot;
	vec[1] = 0;
	dot = &message[mdot - 1];
	if (msgCount > 0 && !noheader) {
		inithdr++;
		headers(vec);
		inithdr = 0;
	}
}

/*
 * Announce information about the file we are editing.
 * Return a likely place to set dot.
 */
newfileinfo()
{
	register struct message *mp;
	register int u, n, mdot, d, s;
	char fname[BUFSIZ], zname[BUFSIZ], *ename;

	for (mp = &message[0]; mp < &message[msgCount]; mp++)
		if (mp->m_flag & MNEW)
			break;
	if (mp >= &message[msgCount])
		for (mp = &message[0]; mp < &message[msgCount]; mp++)
			if ((mp->m_flag & MREAD) == 0)
				break;
	if (mp < &message[msgCount])
		mdot = mp - &message[0] + 1;
	else
		mdot = 1;
	s = d = 0;
	for (mp = &message[0], n = 0, u = 0; mp < &message[msgCount]; mp++) {
		if (mp->m_flag & MNEW)
			n++;
		if ((mp->m_flag & MREAD) == 0)
			u++;
		if (mp->m_flag & MDELETED)
			d++;
		if (mp->m_flag & MSAVED)
			s++;
	}
	ename = mailname;
	if (getfold(fname) >= 0) {
		strcat(fname, "/");
		if (strncmp(fname, mailname, strlen(fname)) == 0) {
			sprintf(zname, "+%s", mailname + strlen(fname));
			ename = zname;
		}
	}
	printf("\"%s\": ", ename);
	if (msgCount == 1)
		printf("1 message");
	else
		printf("%d messages", msgCount);
	if (n > 0)
		printf(" %d new", n);
	if (u-n > 0)
		printf(" %d unread", u);
	if (d > 0)
		printf(" %d deleted", d);
	if (s > 0)
		printf(" %d saved", s);
	if (readonly)
		printf(" [Read only]");
	printf("\n");
	return(mdot);
}

strace() {}

/*
 * Print the current version number.
 */

pversion(e)
{
	printf("Version %s\n", version);
	return(0);
}

/*
 * Load a file of user definitions.
 */
load(name)
	char *name;
{
	register FILE *in, *oldin;

	if ((in = fopen(name, "r")) == NULL)	/*open passed filename*/
		return;
	files[fileno(in)].filep = in;
	files[fileno(in)].flags = LIB_OPEN & ~KEEP_OPEN;
	oldin = input;				/*save stdin*/
	input = in;				/*set file as stdin*/
	loading = 1;				/*set loading flag*/
	sourcing = 1;				/*set sourcing flag*/
	commands();				/*execute stdin commands*/
	loading = 0;				/*clear flag*/
	sourcing = 0;				/*clear flag*/
	input = oldin;				/*reset stdin*/
	fclose(in);				/*close file*/
}
