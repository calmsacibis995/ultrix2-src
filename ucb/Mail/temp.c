#ifndef lint
static	char	*sccsid = "@(#)temp.c	1.4	(ULTRIX)	12/10/84";
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
 * temp.c
 *
 *	sccsid[] = "@(#)temp.c	2.3 (Berkeley) 8/11/83";
 *
 *	17-Feb-84	mah. Added comments and copyright.
 *
 */

#include "rcv.h"

/*
 * Mail -- a mail program
 *
 * Give names to all the temporary files that we will need.
 */

char	tempMail[14];
char	tempQuit[14];
char	tempEdit[14];
char	tempSet[14];
char	tempResid[14];
char	tempMesg[14];

tinit()
{
	register char *cp, *cp2;
	char uname[PATHSIZE];
	register int err = 0;
	register int pid;

	pid = getpid();
	sprintf(tempMail, "/tmp/Rs%05d", pid);
	sprintf(tempResid, "/tmp/Rq%05d", pid);
	sprintf(tempQuit, "/tmp/Rm%05d", pid);
	sprintf(tempEdit, "/tmp/Re%05d", pid);
	sprintf(tempSet, "/tmp/Rx%05d", pid);
	sprintf(tempMesg, "/tmp/Rx%05d", pid);

	if (strlen(myname) != 0) {	/*if user specified, then get uid.
					 *Note: this is not necessarily the
					 *same as the uid executing Mail.
					 */
		uid = getuserid(myname);
		if (uid == -1) {
			printf("\"%s\" is not a user of this system\n",
			    myname);
			exit(1);
		}
	}
	else {				/*user not specified, so get user
					 *uid and name.
					 */
		uid = getuid() & UIDMASK;
		if (username(uid, uname) < 0) {
			copy("ubluit", myname);
			err++;
			if (rcvmode) {
				printf("Who are you!?\n");
				exit(1);
			}
		}
		else			/*successfully determined user name
					 *therefore set myname=uname.
					 */
			copy(uname, myname);
	}
	cp = value("HOME");		/*determine home directory*/
	if (cp == NOSTR)
		cp = ".";		/*if no value found for home, default
					 *to current directory
					 */
	copy(cp, homedir);		/*homedir = HOME or "."*/
	findmail();
	cp = copy(homedir, mbox);	/*mbox = homedir/box*/
	copy("/mbox", cp);
	cp = copy(homedir, mailrc);	/*mailrc = homedir/.mailrc*/
	copy("/.mailrc", cp);
	cp = copy(homedir, deadletter);	/*deadletter = homedir/dead.letter*/
	copy("/dead.letter", cp);
	if (debug) {			/*if debugging print out relavent
					 *information.
					 */
		printf("uid = %d, user = %s, mailname = %s\n",
		    uid, myname, mailname);
		printf("deadletter = %s, mailrc = %s, mbox = %s\n",
		    deadletter, mailrc, mbox);
	}
}
