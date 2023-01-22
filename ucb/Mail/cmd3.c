#ifndef lint
static	char	*sccsid = "@(#)cmd3.c	1.4	(ULTRIX)	12/10/84";
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
 * cmd3.c
 *
 *	sccsid[] = "@(#)cmd3.c	2.14 (Berkeley) 8/11/83";
 *
 *	12-Jan-84	mah.  Added function routine to execute the unalias
 *			command.
 *
 */

#include "rcv.h"
#include <sys/stat.h>

/*
 * Mail -- a mail program
 *
 * Still more user commands.
 */

/*
 * Process a shell escape by saving signals, ignoring signals,
 * and forking a sh -c
 */

shell(str)
	char *str;
{
	int (*sig[2])(), stat[1];
	register int t;
	char *Shell;
	char cmd[BUFSIZ];

	strcpy(cmd, str);
	if (bangexp(cmd) < 0)
		return(-1);
	if ((Shell = value("SHELL")) == NOSTR)
		Shell = SHELL;
	for (t = 2; t < 4; t++)
		sig[t-2] = sigset(t, SIG_IGN);
	t = vfork();
	if (t == 0) {
		sigchild();
		for (t = 2; t < 4; t++)
			if (sig[t-2] != SIG_IGN)
				sigsys(t, SIG_DFL);
		execl(Shell, Shell, "-c", cmd, 0);
		perror(Shell);
		_exit(1);
	}
	while (wait(stat) != t)
		;
	if (t == -1)
		perror("fork");
	for (t = 2; t < 4; t++)
		sigset(t, sig[t-2]);
	printf("!\n");
	return(0);
}

/*
 * Fork an interactive shell.
 */

dosh(str)
	char *str;
{
	int (*sig[2])(), stat[1];
	register int t;
	char *Shell;
	if ((Shell = value("SHELL")) == NOSTR)
		Shell = SHELL;
	for (t = 2; t < 4; t++)
		sig[t-2] = sigset(t, SIG_IGN);
	t = vfork();
	if (t == 0) {
		sigchild();
		for (t = 2; t < 4; t++)
			if (sig[t-2] != SIG_IGN)
				sigsys(t, SIG_DFL);
		execl(Shell, Shell, 0);
		perror(Shell);
		_exit(1);
	}
	while (wait(stat) != t)
		;
	if (t == -1)
		perror("fork");
	for (t = 2; t < 4; t++)
		sigsys(t, sig[t-2]);
	putchar('\n');
	return(0);
}

/*
 * Expand the shell escape by expanding unescaped !'s into the
 * last issued command where possible.
 */

char	lastbang[128];

bangexp(str)
	char *str;
{
	char bangbuf[BUFSIZ];
	register char *cp, *cp2;
	register int n;
	int changed = 0;

	cp = str;
	cp2 = bangbuf;
	n = BUFSIZ;
	while (*cp) {
		if (*cp == '!') {
			if (n < strlen(lastbang)) {
overf:
				printf("Command buffer overflow\n");
				return(-1);
			}
			changed++;
			strcpy(cp2, lastbang);
			cp2 += strlen(lastbang);
			n -= strlen(lastbang);
			cp++;
			continue;
		}
		if (*cp == '\\' && cp[1] == '!') {
			if (--n <= 1)
				goto overf;
			*cp2++ = '!';
			cp += 2;
			changed++;
		}
		if (--n <= 1)
			goto overf;
		*cp2++ = *cp++;
	}
	*cp2 = 0;
	if (changed) {
		printf("!%s\n", bangbuf);
		fflush(stdout);
	}
	strcpy(str, bangbuf);
	strncpy(lastbang, bangbuf, 128);
	lastbang[127] = 0;
	return(0);
}

/*
 * Print out a nice help message from some file or another.
 */

help()
{
	register c;
	register FILE *f;

	if ((f = fopen(HELPFILE, "r")) == NULL) {
		perror(HELPFILE);
		return(1);
	}
	files[fileno(f)].filep = f;
	files[fileno(f)].flags = LIB_OPEN & ~KEEP_OPEN;
	while ((c = getc(f)) != EOF)
		putchar(c);
	fclose(f);
	return(0);
}

/*
 * Change user's working directory.
 */

schdir(str)
	char *str;
{
	register char *cp;

	for (cp = str; *cp == ' '; cp++)
		;
	if (*cp == '\0')
		cp = homedir;
	else
		if ((cp = expand(cp)) == NOSTR)
			return(1);
	if (chdir(cp) < 0) {
		perror(cp);
		return(1);
	}
	return(0);
}

/*
 * Reply to a list of messages.  Extract each name from the
 * message header and send them off to mail1()
 */

respond(msgvec)
	int *msgvec;
{
	struct message *mp;
	char *cp, *cp2, *cp3, *rcv, *replyto;
	char buf[2 * LINESIZE], **ap;
	struct name *np;
	struct header head;

	if (msgvec[1] != 0) {
		printf("Sorry, can't reply to multiple messages at once\n");
		return(1);
	}
	mp = &message[msgvec[0] - 1];
	dot = mp;
	rcv = NOSTR;
	cp = skin(nameof(mp, 1));
	if (cp != NOSTR)
	    rcv = cp;
	cp = skin(hfield("from", mp));
	if (cp != NOSTR)
	    rcv = cp;
	replyto = skin(hfield("reply-to", mp));
	strcpy(buf, "");
	if (replyto != NOSTR)
		strcpy(buf, replyto);
	else {
		cp = skin(hfield("to", mp));
		if (cp != NOSTR)
			strcpy(buf, cp);
	}
	np = elide(extract(buf, GTO));
	/* rcv = rename(rcv); */
	mapf(np, rcv);
	/*
	 * Delete my name from the reply list,
	 * and with it, all my alternate names.
	 */
	np = delname(np, myname, icequal);
	if (altnames)
		for (ap = altnames; *ap; ap++)
			np = delname(np, *ap, icequal);
	head.h_seq = 1;
	cp = detract(np, 0);
	if (cp != NOSTR && replyto == NOSTR) {
		strcpy(buf, cp);
		strcat(buf, " ");
		strcat(buf, rcv);
	}
	else {
		if (cp == NOSTR && replyto != NOSTR)
			printf("Empty reply-to field -- replying to author\n");
		if (cp == NOSTR)
			strcpy(buf, rcv);
		else
			strcpy(buf, cp);
	}
	head.h_to = buf;
	head.h_subject = hfield("subject", mp);
	if (head.h_subject == NOSTR)
		head.h_subject = hfield("subj", mp);
	head.h_subject = reedit(head.h_subject);
	head.h_cc = NOSTR;
	if (replyto == NOSTR) {
		cp = hfield("cc", mp);
		if (cp != NOSTR) {
			np = elide(extract(cp, GCC));
			mapf(np, rcv);
			np = delname(np, myname, icequal);
			if (altnames != 0)
				for (ap = altnames; *ap; ap++)
					np = delname(np, *ap, icequal);
			head.h_cc = detract(np, 0);
		}
	}
	head.h_bcc = NOSTR;
	mail1(&head);
	return(0);
}

/*
 * Modify the subject we are replying to to begin with Re: if
 * it does not already.
 */

char *
reedit(subj)
	char *subj;
{
	char sbuf[10];
	register char *newsubj;

	if (subj == NOSTR)
		return(NOSTR);
	strncpy(sbuf, subj, 3);
	sbuf[3] = 0;
	if (icequal(sbuf, "re:"))
		return(subj);
	newsubj = salloc(strlen(subj) + 6);
	sprintf(newsubj, "Re:  %s", subj);
	return(newsubj);
}

/*
 * Preserve the named messages, so that they will be sent
 * back to the system mailbox.
 */

preserve(msgvec)
	int *msgvec;
{
	register struct message *mp;
	register int *ip, mesg;

	if (edit) {
		printf("Cannot \"preserve\" in edit mode\n");
		return(1);
	}
	for (ip = msgvec; *ip != NULL; ip++) {
		mesg = *ip;
		mp = &message[mesg-1];
		mp->m_flag |= MPRESERVE;
		mp->m_flag &= ~MBOX;
		dot = mp;
	}
	return(0);
}

/*
 * Print the size of each message.
 */

messize(msgvec)
	int *msgvec;
{
	register struct message *mp;
	register int *ip, mesg;

	for (ip = msgvec; *ip != NULL; ip++) {
		mesg = *ip;
		mp = &message[mesg-1];
		printf("%d: %ld\n", mesg, mp->m_size);
	}
	return(0);
}

/*
 * Quit quickly.  If we are sourcing, just pop the input level
 * by returning an error.
 */

rexit(e)
{
	if (sourcing)
		return(1);
	if (Tflag != NOSTR)
		close(creat(Tflag, 0600));
	exit(e);
}

/*
 * Set or display a variable value.  Syntax is similar to that
 * of csh.
 */

set(arglist)
	char **arglist;
{
	register struct var *vp;
	register char *cp, *cp2;
	char varbuf[BUFSIZ], **ap, **p;
	int errs, h, s;

					/*no arguments passed so prints out
					 *values set.
					 */
	if (argcount(arglist) == 0) {
		for (h = 0, s = 1; h < HSHSIZE; h++)
			for (vp = variables[h]; vp != NOVAR; vp = vp->v_link)
				s++;
		ap = (char **) salloc(s * sizeof *ap);
		for (h = 0, p = ap; h < HSHSIZE; h++)
			for (vp = variables[h]; vp != NOVAR; vp = vp->v_link)
				*p++ = vp->v_name;
		*p = NOSTR;
		sort(ap);
		for (p = ap; *p != NOSTR; p++)
			printf("%s\t%s\n", *p, value(*p));
		return(0);
	}
	errs = 0;			/*initialize err indicator*/
	for (ap = arglist; *ap != NOSTR; ap++) {
					/*copy 1 arg at a time into varbuf*/
		cp = *ap;
		cp2 = varbuf;
		while (*cp != '=' && *cp != '\0')
			*cp2++ = *cp++;
		*cp2 = '\0';		/*terminate arg*/
		if (*cp == '\0')
			cp = "";
		else
			cp++;
		if (equal(varbuf, "")) {
			printf("Non-null variable name required\n");
			errs++;
			continue;
		}
		assign(varbuf, cp);	/*assign a value to variable in varbuf*/
	}
	return(errs);
}

/*
 * Unset a bunch of variable values.
 */

unset(arglist)
	char **arglist;
{
	register struct var *vp, *vp2;
	register char *cp;
	int errs, h;
	char **ap;

	errs = 0;
	for (ap = arglist; *ap != NOSTR; ap++) {
		if ((vp2 = lookup(*ap)) == NOVAR) {
			if (!sourcing) {
				printf("\"%s\": undefined variable\n", *ap);
				errs++;
			}
			continue;
		}
		h = hash(*ap);
		if (vp2 == variables[h]) {
			variables[h] = variables[h]->v_link;
			vfree(vp2->v_name);
			vfree(vp2->v_value);
			cfree(vp2);
			continue;
		}
		for (vp = variables[h]; vp->v_link != vp2; vp = vp->v_link)
			;
		vp->v_link = vp2->v_link;
		vfree(vp2->v_name);
		vfree(vp2->v_value);
		cfree(vp2);
	}
	return(errs);
}

/*
 * Put add users to a group.
 */

group(argv)
	char **argv;
{
	register struct grouphead *gh;
	register struct group *gp;
	register int h;
	int s;
	char **ap, *gname, **p;

	if (argcount(argv) == 0) {	/*if no args then only wants to see a
					 *a list of the aliases.
					 */
		for (h = 0, s = 1; h < HSHSIZE; h++)
			for (gh = groups[h]; gh != NOGRP; gh = gh->g_link)
				s++;
		ap = (char **) salloc(s * sizeof *ap);
		for (h = 0, p = ap; h < HSHSIZE; h++)
			for (gh = groups[h]; gh != NOGRP; gh = gh->g_link)
				*p++ = gh->g_name;
		*p = NOSTR;
		sort(ap);
		for (p = ap; *p != NOSTR; p++)
			printgroup(*p);
		return(0);
	}
	if (argcount(argv) == 1) {	/*if only 1 arg then only want to see
					 *the aliases for that specified alias.
					 */
		printgroup(*argv);
		return(0);
	}
	gname = *argv;
	h = hash(gname);
	if ((gh = findgroup(gname)) == NOGRP) {	/*if group not found, then must
						 *allocate space for grouphead
						 *structure.
						 */
		gh = (struct grouphead *) calloc(sizeof *gh, 1);
		gh->g_name = vcopy(gname);	/*get space for name + copy it
						 *there; return ptr for g_name.
						 */
		gh->g_list = NOGE;		/*initiate g_list to NIL.*/
		gh->g_link = groups[h];		/*insert grouphead structure*/
		groups[h] = gh;			/* at beginning of thread*/
	}

	/*
	 * Insert names from the command list into the group.
	 * Who cares if there are duplicates?  They get tossed
	 * later anyway.
	 */

	for (ap = argv+1; *ap != NOSTR; ap++) {
						/*allocate space for group
						 *structure.
						 */
		gp = (struct group *) calloc(sizeof *gp, 1);
		gp->ge_name = vcopy(*ap);	/*get space for name + copy it
						 *there; return ptr for ge_name.
						 */
		gp->ge_link = gh->g_list;	/*insert group structure at*/
		gh->g_list = gp;		/* beginning of thread*/
	}
	return(0);
}
/*addition for 12-Jan-84*/
ungroup(argv)
	char **argv;
{
	struct grouphead *gh1;
	register struct grouphead *gh2;
	register struct group *gp;
	register char **ap;
	int h;

	if (argcount(argv) == 0)	/*no group specified, so nothing to do*/
		return(0);
	ap = argv;
	while (*ap != NOSTR) {
		h = hash(*ap);		/*get index into group hash table*/
		if (groups[h] == NOGRP)
					/*if no entry in groups[], then no need
					 *to look further.
					 */
			gh1 = NOGRP;	/*set gh1 to reflect this*/
		else {			/*not nil, so check if 1st entry on
					 *thread is the one we want.
					 */
			if (equal(groups[h]->g_name, *ap)) {
					/*this is it, so reset ptrs to drop it
					 *from the thread.
					 */
				gh1 = groups[h];
				groups[h] = groups[h]->g_link;
			}
			else {		/*look down the thread for the one we
					 *want.
					 */
				gh1 = NOGRP;
				gh2 = groups[h];
				while (gh2->g_link != NOGRP) {
					if(equal(gh2->g_link->g_name, *ap)) {
						gh1 = gh2->g_link;
						gh2->g_link = gh1->g_link;
					}
					gh2 = gh2->g_link;
				}
			}
		}
		if (gh1 == NOGRP)	/*not found*/
			printf("\"%s \": not a group\n",*ap);
		else {			/*found it, so return the space*/
			gp = gh1->g_list;
			while (gp != NOGE) {	/*return groups str & structs*/
				free(gp->ge_name);	/*return str space*/
				free(gp);		/*return struct space*/
				gp = gp->ge_link;	/*space has been return,
							 *but contents are still
							 *intact, so this works.
							 */
			}		/*return grouphead str and structure*/
			free(gh1->g_name);	/*return str space*/
			free(gh1);		/*return struct space*/
		}
		ap++;
	}
	return(0);
}
/*end of addition*/

/*
 * Sort the passed string vecotor into ascending dictionary
 * order.
 */

sort(list)
	char **list;
{
	register char **ap;
	int diction();

	for (ap = list; *ap != NOSTR; ap++)
		;
	if (ap-list < 2)
		return;
	qsort(list, ap-list, sizeof *list, diction);
}

/*
 * Do a dictionary order comparison of the arguments from
 * qsort.
 */

diction(a, b)
	register char **a, **b;
{
	return(strcmp(*a, *b));
}

/*
 * The do nothing command for comments.
 */

null(e)
{
	return(0);
}

/*
 * Print out the current edit file, if we are editing.
 * Otherwise, print the name of the person who's mail
 * we are reading.
 */

file(argv)
	char **argv;
{
	register char *cp;
	char fname[BUFSIZ];
	int edit;

	if (argv[0] == NOSTR) {
		newfileinfo();
		return(0);
	}

	/*
	 * Acker's!  Must switch to the new file.
	 * We use a funny interpretation --
	 *	# -- gets the previous file
	 *	% -- gets the invoker's post office box
	 *	%user -- gets someone else's post office box
	 *	& -- gets invoker's mbox file
	 *	string -- reads the given file
	 */

	cp = getfilename(argv[0], &edit);
	if (cp == NOSTR)
		return(-1);
	if (setfile(cp, edit)) {
		perror(cp);
		return(-1);
	}
	newfileinfo();
}

/*
 * Evaluate the string given as a new mailbox name.
 * Ultimately, we want this to support a number of meta characters.
 * Possibly:
 *	% -- for my system mail box
 *	%user -- for user's system mail box
 *	# -- for previous file
 *	& -- get's invoker's mbox file
 *	file name -- for any other file
 */

char	prevfile[PATHSIZE];

char *
getfilename(name, aedit)
	char *name;
	int *aedit;
{
	register char *cp;
	char savename[BUFSIZ];
	char oldmailname[BUFSIZ];

	/*
	 * Assume we will be in "edit file" mode, until
	 * proven wrong.
	 */
	*aedit = 1;
	switch (*name) {
	case '%':
		*aedit = 0;
		strcpy(prevfile, mailname);
		if (name[1] != 0) {
			strcpy(savename, myname);
			strcpy(oldmailname, mailname);
			strncpy(myname, name+1, PATHSIZE-1);
			myname[PATHSIZE-1] = 0;
			findmail();
			cp = savestr(mailname);
			strcpy(myname, savename);
			strcpy(mailname, oldmailname);
			return(cp);
		}
		strcpy(oldmailname, mailname);
		findmail();
		cp = savestr(mailname);
		strcpy(mailname, oldmailname);
		return(cp);

	case '#':
		if (name[1] != 0)
			goto regular;
		if (prevfile[0] == 0) {
			printf("No previous file\n");
			return(NOSTR);
		}
		cp = savestr(prevfile);
		strcpy(prevfile, mailname);
		return(cp);

	case '&':
		strcpy(prevfile, mailname);
		if (name[1] == 0)
			return(mbox);
		/* Fall into . . . */

	default:
regular:
		strcpy(prevfile, mailname);
		cp = expand(name);
		return(cp);
	}
}

/*
 * Expand file names like echo
 */

echo(argv)
	char **argv;
{
	register char **ap;
	register char *cp;

	for (ap = argv; *ap != NOSTR; ap++) {
		cp = *ap;
		if ((cp = expand(cp)) != NOSTR)
			printf("%s ", cp);
	}
	return(0);
}

/*
 * Reply to a series of messages by simply mailing to the senders
 * and not messing around with the To: and Cc: lists as in normal
 * reply.
 */

Respond(msgvec)
	int msgvec[];
{
	struct header head;
	struct message *mp;
	register int i, s, *ap;
	register char *cp, *cp2, *subject;

	for (s = 0, ap = msgvec; *ap != 0; ap++) {
		mp = &message[*ap - 1];
		dot = mp;
		if ((cp = skin(hfield("from", mp))) != NOSTR)
		    s+= strlen(cp) + 1;
		else
		    s += strlen(skin(nameof(mp, 2))) + 1;
	}
	if (s == 0)
		return(0);
	cp = salloc(s + 2);
	head.h_to = cp;
	for (ap = msgvec; *ap != 0; ap++) {
		mp = &message[*ap - 1];
		if ((cp2 = skin(hfield("from", mp))) == NOSTR)
		    cp2 = skin(nameof(mp, 2));
		cp = copy(cp2, cp);
		*cp++ = ' ';
	}
	*--cp = 0;
	mp = &message[msgvec[0] - 1];
	subject = hfield("subject", mp);
	head.h_seq = 0;
	if (subject == NOSTR)
		subject = hfield("subj", mp);
	head.h_subject = reedit(subject);
	if (subject != NOSTR)
		head.h_seq++;
	head.h_cc = NOSTR;
	head.h_bcc = NOSTR;
	mail1(&head);
	return(0);
}

/*
 * Conditional commands.  These allow one to parameterize one's
 * .mailrc and do some things if sending, others if receiving.
 */

ifcmd(argv)
	char **argv;
{
	register char *cp;

	if (cond != CANY) {
		printf("Illegal nested \"if\"\n");
		return(1);
	}
	cond = CANY;
	cp = argv[0];
	switch (*cp) {
	case 'r': case 'R':
		cond = CRCV;
		break;

	case 's': case 'S':
		cond = CSEND;
		break;

	default:
		printf("Unrecognized if-keyword: \"%s\"\n", cp);
		return(1);
	}
	return(0);
}

/*
 * Implement 'else'.  This is pretty simple -- we just
 * flip over the conditional flag.
 */

elsecmd()
{

	switch (cond) {
	case CANY:
		printf("\"Else\" without matching \"if\"\n");
		return(1);

	case CSEND:
		cond = CRCV;
		break;

	case CRCV:
		cond = CSEND;
		break;

	default:
		printf("Mail's idea of conditions is screwed up\n");
		cond = CANY;
		break;
	}
	return(0);
}

/*
 * End of if statement.  Just set cond back to anything.
 */

endifcmd()
{

	if (cond == CANY) {
		printf("\"Endif\" without matching \"if\"\n");
		return(1);
	}
	cond = CANY;
	return(0);
}

/*
 * Set the list of alternate names.
 */
alternates(namelist)
	char **namelist;
{
	register int c;
	register char **ap, **ap2, *cp;

	c = argcount(namelist) + 1;
	if (c == 1) {
		if (altnames == 0)
			return(0);
		for (ap = altnames; *ap; ap++)
			printf("%s ", *ap);
		printf("\n");
		return(0);
	}
	if (altnames != 0)
		cfree((char *) altnames);
	altnames = (char **) calloc(c, sizeof (char *));
	for (ap = namelist, ap2 = altnames; *ap; ap++, ap2++) {
		cp = (char *) calloc(strlen(*ap) + 1, sizeof (char));
		strcpy(cp, *ap);
		*ap2 = cp;
	}
	*ap2 = 0;
	return(0);
}
