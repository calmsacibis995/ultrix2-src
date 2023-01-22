#ifndef lint
static	char	*sccsid = "@(#)tty.c	1.4	(ULTRIX)	12/10/84";
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
 * tty.c
 *
 *	sccsid[] = "@(#)tty.c	2.7 (Berkeley) 8/11/83";
 *
 *	17-Feb-84	mah. Added comments and copyright.
 *
 */

/*
 * Mail -- a mail program
 *
 * Generally useful tty stuff.
 */

#include "rcv.h"

static	int	c_erase;		/* Current erase char */
static	int	c_kill;			/* Current kill char */
static	int	hadcont;		/* Saw continue signal */
static	jmp_buf	rewrite;		/* Place to go when continued */
#ifndef TIOCSTI
static	int	ttyset;			/* We must now do erase/kill */
#endif

/*
 * Read all relevant header fields.
 */

grabh(hp, gflags)
	struct header *hp;
{
	struct sgttyb ttybuf;
	int ttycont(), signull();
#ifndef TIOCSTI
	int (*savesigs[2])();
#endif
	int (*savecont)();
	register int s;
	int errs;

# ifdef VMUNIX
	savecont = sigset(SIGCONT, signull);
# endif VMUNIX
	errs = 0;			/*initialize errs and ttyset*/
#ifndef TIOCSTI
	ttyset = 0;
#endif
	if (gtty(fileno(stdin), &ttybuf) < 0) {
					/*if unable to get stdin status, give
					 *err msg and return.
					 */
		perror("gtty");
		return(-1);
	}
	c_erase = ttybuf.sg_erase;	/*save current erase + kill setting*/
	c_kill = ttybuf.sg_kill;
#ifndef TIOCSTI
	ttybuf.sg_erase = 0;		/*clear erase + kill settings in ttybuf
					 *only.
					 */
	ttybuf.sg_kill = 0;
	for (s = SIGINT; s <= SIGQUIT; s++)
		if ((savesigs[s-SIGINT] = sigset(s, SIG_IGN)) == SIG_DFL)
			sigset(s, SIG_DFL);
#endif
	if (gflags & GTO) {		/*To: */
#ifndef TIOCSTI
		if (!ttyset && hp->h_to != NOSTR)
					/*if we don't have to do erase/kill and
					 *to: doesn't = null string, then
					 *indicate we do have to do erase/kill
					 *and clear erase/kill for real.
					 */
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_to = readtty("To: ", hp->h_to);	/*readtty rets ptr to
							 *str inputted by user.
							 */
		if (hp->h_to != NOSTR)	/*if no string ++ seq for optimization*/
			hp->h_seq++;
	}
	if (gflags & GSUBJECT) {	/*Subject:*/
#ifndef TIOCSTI
		if (!ttyset && hp->h_subject != NOSTR)
					/*ditto as for To:*/
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_subject = readtty("Subject: ", hp->h_subject);
		if (hp->h_subject != NOSTR)
			hp->h_seq++;
	}
	if (gflags & GCC) {		/*Cc:*/
#ifndef TIOCSTI
		if (!ttyset && hp->h_cc != NOSTR)
					/*ditto as for To:*/
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_cc = readtty("Cc: ", hp->h_cc);
		if (hp->h_cc != NOSTR)
			hp->h_seq++;
	}
	if (gflags & GBCC) {		/*Bcc:*/
#ifndef TIOCSTI
		if (!ttyset && hp->h_bcc != NOSTR)
					/*ditto as for To:*/
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_bcc = readtty("Bcc: ", hp->h_bcc);
		if (hp->h_bcc != NOSTR)
			hp->h_seq++;
	}
# ifdef VMUNIX
	sigset(SIGCONT, savecont);
# endif VMUNIX
#ifndef TIOCSTI
	ttybuf.sg_erase = c_erase;	/*set erase/kill back to original
					 *settings.
					 */
	ttybuf.sg_kill = c_kill;
	if (ttyset)			/*reset stdin*/
		stty(fileno(stdin), &ttybuf);
	for (s = SIGINT; s <= SIGQUIT; s++)
		sigset(s, savesigs[s-SIGINT]);
#endif
	return(errs);			/*currently will always return 0*/
}

/*
 * Read up a header from standard input.
 * The source string has the preliminary contents to
 * be read.
 *
 */

char *
readtty(pr, src)
	char pr[], src[];
{
	char ch, canonb[BUFSIZ];
	int c, signull();
	register char *cp, *cp2;

	fputs(pr, stdout);
	fflush(stdout);
	if (src != NOSTR && strlen(src) > BUFSIZ - 2) {
		printf("too long to edit\n");
		return(src);
	}
#ifndef TIOCSTI
	if (src != NOSTR)		/*write src or "" out to stdout*/
		cp = copy(src, canonb);
	else
		cp = copy("", canonb);
	fputs(canonb, stdout);
	fflush(stdout);
#else
	cp = src == NOSTR ? "" : src;
	while (c = *cp++) {
		if (c == c_erase || c == c_kill) {
			ch = '\\';
			ioctl(0, TIOCSTI, &ch);
		}
		ch = c;
		ioctl(0, TIOCSTI, &ch);
	}
	cp = canonb;
	*cp = 0;
#endif
	cp2 = cp;
	while (cp2 < canonb + BUFSIZ)	/*fill out rest of array with 0's*/
		*cp2++ = 0;
	cp2 = cp;
	if (setjmp(rewrite))		/*setup reentry pt for "continue after
					 *stop" signals.
					 */
		goto redo;
# ifdef VMUNIX
	sigset(SIGCONT, ttycont);	/*set routine to handle "continue after
					 *stop" signals.
					 */
# endif VMUNIX
	clearerr(stdin);		/*reset err indication for stdin*/
	while (cp2 < canonb + BUFSIZ) {	/*get chars from stdin and place into
					 *array.
					 */
		c = getc(stdin);
		if (c == EOF || c == '\n')
			break;
		*cp2++ = c;
	}
	*cp2 = 0;
# ifdef VMUNIX
	sigset(SIGCONT, signull);
# endif VMUNIX
	if (c == EOF && ferror(stdin) && hadcont) {
					/*if EOF detected and there's been an
					 *err reading from stdin and a continue
					 *signal has been seen.
					 */
redo:
		hadcont = 0;		/*clear cont signal flag*/
					/*cp = either NOSTR if nothing in array
					 *else cp = array.
					 */
		cp = strlen(canonb) > 0 ? canonb : NOSTR;
		clearerr(stdin);	/*reset err indication for stdin*/
		return(readtty(pr, cp));
	}
#ifndef TIOCSTI
	if (cp == NOSTR || *cp == '\0')
		return(src);
	cp2 = cp;
	if (!ttyset)
					/*return a ptr to either the array or
					 *NOSTR depending on lenght of array.
					 */
		return(strlen(canonb) > 0 ? savestr(canonb) : NOSTR);
	while (*cp != '\0') {
		c = *cp++;
		if (c == c_erase) {	/*do manual erase,ie correct array str*/
			if (cp2 == canonb)
				continue;
			if (cp2[-1] == '\\') {
				cp2[-1] = c;
				continue;
			}
			cp2--;
			continue;
		}
		if (c == c_kill) {	/*do manual kill,ie correct array str*/
			if (cp2 == canonb)
				continue;
			if (cp2[-1] == '\\') {
				cp2[-1] = c;
				continue;
			}
			cp2 = canonb;
			continue;
		}
		*cp2++ = c;
	}
	*cp2 = '\0';
#endif
	if (equal("", canonb))		/*if array="" return ptr to NOSTR*/
		return(NOSTR);
	return(savestr(canonb));	/*return a ptr to array*/
}

# ifdef VMUNIX
/*
 * Receipt continuation.
 */
ttycont(s)
{

	hadcont++;
	longjmp(rewrite, 1);
}
# endif VMUNIX

/*
 * Null routine to satisfy
 * silly system bug that denies us holding SIGCONT
 */
signull(s)
{}
