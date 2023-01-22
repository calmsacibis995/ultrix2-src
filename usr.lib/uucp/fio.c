#ifndef lint
static	char	*sccsid = "@(#)fio.c	1.2	(ULTRIX)	6/28/85";
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
 * flow control protocol.
 *
 * This protocol relies on flow control of the data stream.
 * It is meant for working over links that can (almost) be
 * guaranteed to be errorfree, specifically X.25/PAD links.
 * A sumcheck is carried out over a whole file only. If a
 * transport fails the receiver can request retransmission(s).
 * This protocol uses a 7-bit datapath only, so it can be
 * used on links that are not 8-bit transparent.
 *
 * When using this protocol with an X.25 PAD:
 * Although this protocol uses no control chars except CR,
 * control chars NULL and ^P are used before this protocol
 * is started; since ^P is the default char for accessing
 * PAD X.28 command mode, be sure to disable that access
 * (PAD par 1). Also make sure both flow control pars
 * (5 and 12) are set. The CR used in this proto is meant
 * to trigger packet transmission, hence par 3 should be 
 * set to 2; a good value for the Idle Timer (par 4) is 10.
 * All other pars should be set to 0.
 * Normally a calling site will take care of setting the
 * local PAD pars via an X.28 command and those of the remote
 * PAD via an X.29 command, unless the remote site has a
 * special channel assigned for this protocol with the proper
 * par settings.
 *
 * Author: Piet Beertema, CWI, Amsterdam, Sep 1984
 */

#include "uucp.h"
#include <signal.h>
#ifdef SIII
#  include <termio.h>
#else
#  include <sgtty.h>
#endif SIII
#include <setjmp.h>
#include <sys/time.h>
#include <sys/types.h>


#define Not080		/* marc */
#define FBUFSIZ		256
#define MAXMSGLEN	512  /* this is a guess  - marc */
#define MAXCHARTIME 	MAXMSGTIME	/* - marc */

static int chksum;
static jmp_buf Ffailbuf;

static
falarm()
{
	signal(SIGALRM, falarm);
	longjmp(Ffailbuf, 1);
}

static int (*fsig)();

#ifndef SIII
#define TCGETA	TIOCGETP
#define TCSETA	TIOCSETP
#define termio	sgttyb
#endif SIII

fturnon()
{
	int ret;
	struct termio ttbuf;

	ioctl(Ifn, TCGETA, &ttbuf);
#ifdef SIII
	ttbuf.c_iflag = IXOFF|IXON|ISTRIP;
	ttbuf.c_cc[4] = FBUFSIZ > 64 ? 64 : FBUFSIZ;
	ttbuf.c_cc[5] = 5;
#else
	ttbuf.sg_flags = ANYP|CBREAK|TANDEM;
#endif SIII
	ret = ioctl(Ifn, TCSETA, &ttbuf);
	ASSERT(ret >= 0, "STTY FAILED", "", ret);
	fsig = signal(SIGALRM, falarm);
	/* give the other side time to perform its ioctl;
	 * otherwise it may flush out the first data this
	 * side is about to send.
	 */
	sleep(2);
	return 0;
}

fturnoff()
{
	(void) signal(SIGALRM, fsig);
	return 0;
}

fwrmsg(type, str, fn)
register char *str;
int fn;
char type;
{
	register char *s;
	char bufr[MAXMSGLEN];

	s = bufr;
	*s++ = type;
	while (*str)
		*s++ = *str++;
	if (*(s-1) == '\n')
		s--;
	*s++ = '\r';
	(void) write(fn, bufr, s - bufr);
	return 0;
}

frdmsg(str, fn)
register char *str;
register int fn;
{
	register char *smax;

	if (setjmp(Ffailbuf))
		return FAIL;
	smax = str + MAXMSGLEN - 1;
	(void) alarm(2*MAXMSGTIME);
	for (;;) {
		if (read(fn, str, 1) <= 0)
			goto msgerr;
		if (*str == '\r')
			break;
		if (*str < ' ')
			continue;
		if (str++ >= smax)
			goto msgerr;
	}
	*str = '\0';
	(void) alarm(0);
	return 0;
msgerr:
	(void) alarm(0);
	return FAIL;
}

fwrdata(fp1, fn)
FILE *fp1;
int fn;
{
	register int flen, alen, ret;
	register char *obp;
	char ibuf[FBUFSIZ];
	char ack;
	long abytes, fbytes;
	time_t t1, t2;

	ret = FAIL;
retry:
	chksum = 0xffff;
	abytes = fbytes = 0L;
	time(&t1);
	while ((flen = fread(ibuf, sizeof (char), FBUFSIZ, fp1)) > 0) {
		alen = fwrblk(fn, ibuf, flen);
		abytes += alen >= 0 ? alen : -alen;
		if (alen <= 0)
			goto acct;
		fbytes += flen;
	}
	sprintf(ibuf, "\176\176%04x\r", chksum);
	abytes += alen = strlen(ibuf);
	if (write(fn, ibuf, alen) == alen) {
		DEBUG(8, "%d\n", alen);
		DEBUG(8, "checksum: %04x\n", chksum);
	}
	if (frdmsg(ibuf, fn) < 0) {
		ack = 0;
		goto acct;
	}
	ack = ibuf[0];
	DEBUG(4, "ack - '%c'\n", ack);
	if (ack == 'G')
		ret = 0;
acct:
	time(&t2);
	sprintf(ibuf, ret == 0 ?
		"sent data %ld bytes %ld secs" :
		"send failed after %ld bytes %ld secs",
		fbytes, t2 - t1);
	DEBUG(1, "%s\n", ibuf);
	syslog(ibuf);
#ifdef SYSACCT
	sysacct(abytes, t2 - t1);
#endif SYSACCT
	if (ack == 'R') {
		DEBUG(4, "RETRY:\n", 0);
		fseek(fp1, 0L, 0);
		goto retry;
	}
#ifdef SYSACCT
	if (ret)
		sysaccf(NULL);		/* force accounting */
#endif SYSACCT
	return ret;
}

/* max. attempts to retransmit a file: */
#define MAXRETRIES	(fbytes < 10000L ? 2 : 1)

frddata(fn, fp2)
register int fn;
register FILE *fp2;
{
	register int flen;
	register char eof;
	char ibuf[FBUFSIZ];
	int ret, alen, retries = 0;
	long abytes, fbytes;
	time_t t1, t2;

	ret = FAIL;
retry:
	chksum = 0xffff;
	abytes = fbytes = 0L;
	time(&t1);
	do {
		flen = frdblk(ibuf, fn, &alen);
		abytes += alen;
		if (flen < 0)
			goto acct;
		if (eof = flen > FBUFSIZ)
			flen -= FBUFSIZ + 1;
		fbytes += flen;
		if (fwrite(ibuf, sizeof (char), flen, fp2) != flen)
			goto acct;
	} while (!eof);
	ret = 0;
acct:
	time(&t2);
	sprintf(ibuf, ret == 0 ? 
		"received data %ld bytes %ld secs" :
		"receive failed after %ld bytes %ld secs",
		fbytes, t2 - t1);
	DEBUG(1, "%s\n", ibuf);
	syslog(ibuf);
#ifdef SYSACCT
	sysacct(abytes, t2 - t1);
#endif SYSACCT
	if (ret) {
		if (retries++ < MAXRETRIES) {
			DEBUG(8, "send ack: 'R'\n", 0);
			fwrmsg('R', "", fn);
			fseek(fp2, 0L, 0);
			DEBUG(4, "RETRY:\n", 0);
			goto retry;
		}
		DEBUG(8, "send ack: 'Q'\n", 0);
		fwrmsg('Q', "", fn);
#ifdef SYSACCT
		sysaccf(NULL);		/* force accounting */
#endif SYSACCT
	} else {
		DEBUG(8, "send ack: 'G'\n", 0);
		fwrmsg('G', "", fn);
	}
	return ret;
}

static
frdbuf(blk, len, fn)
register char *blk;
register int len;
register int fn;
{
	static int ret = FBUFSIZ / 2;
#ifndef Not080
	extern int linebaudrate;
#endif Not080

	if (setjmp(Ffailbuf))
		return FAIL;
#ifndef Not080
	if (len == FBUFSIZ && ret < FBUFSIZ / 2 &&
	    linebaudrate > 0 && linebaudrate < 4800)
		sleep(1);
#endif Not080
	(void) alarm(MAXCHARTIME);
	ret = read(fn, blk, len);
	alarm(0);
	return ret <= 0 ? FAIL : ret;
}

/* Byte conversion:
 *
 *   from        pre       to
 * 000-037       172     100-137
 * 040-171               040-171
 * 172-177       173     072-077
 * 200-237       174     100-137
 * 240-371       175     040-171
 * 372-377       176     072-077
 */

static
fwrblk(fn, ip, len)
int fn;
register char *ip;
register int len;
{
	register char *op;
	register int sum, nl;
	int ret;
	char obuf[FBUFSIZ * 2];

	DEBUG(8, "%d/", len);
	op = obuf;
	nl = 0;
	sum = chksum;
	do {
		if (sum & 0x8000) {
			sum <<= 1;
			sum++;
		} else
			sum <<= 1;
		sum += *ip;
		sum &= 0xffff;
		if (*ip & 0200) {
			*ip &= 0177;
			if (*ip < 040) {
				*op++ = '\174';
				*op++ = *ip++ + 0100;
			} else
			if (*ip <= 0171) {
				*op++ = '\175';
				*op++ = *ip++;
			}
			else {
				*op++ = '\176';
				*op++ = *ip++ - 0100;
			}
			nl += 2;
		} else {
			if (*ip < 040) {
				*op++ = '\172';
				*op++ = *ip++ + 0100;
				nl += 2;
			} else
			if (*ip <= 0171) {
				*op++ = *ip++;
				nl++;
			} else {
				*op++ = '\173';
				*op++ = *ip++ - 0100;
				nl += 2;
			}
		}
	} while (--len);
	chksum = sum;
	DEBUG(8, "%d,", nl);
	ret = write(fn, obuf, nl);
	return ret == nl ? nl : ret < 0 ? 0 : -ret;
}

static
frdblk(ip, fn, rlen)
register char *ip;
int fn, *rlen;
{
	register char *op, c;
	register int sum, len, nl;
	char buf[5], *erbp = ip;
	int i;
	static char special = 0;

	if ((len = frdbuf(ip, FBUFSIZ, fn)) == FAIL)
		goto dcorr;
	*rlen = len;
	DEBUG(8, "%d/", len);
	op = ip;
	nl = 0;
	sum = chksum;
	do {
		if ((*ip &= 0177) >= '\172') {
			if (special) {
				DEBUG(8, "%d", nl);
				special = 0;
				op = buf;
				if (*ip++ != '\176' || (i = --len) > 5)
					goto dcorr;
				while (i--)
					*op++ = *ip++;
				while (len < 5) {
					i = frdbuf(&buf[len], 5 - len, fn);
					if (i == FAIL) {
						len = FAIL;
						goto dcorr;
					}
					DEBUG(8, ",%d", i);
					len += i;
					*rlen += i;
				}
				if (buf[4] != '\r')
					goto dcorr;
				sscanf(buf, "%4x", &chksum);
				DEBUG(8, "\nchecksum: %04x\n", sum);
				if (chksum == sum)
					return FBUFSIZ + 1 + nl;
				else {
					DEBUG(8, "\n", 0);
					DEBUG(4, "Bad checksum\n", 0);
					return FAIL;
				}
			}
			special = *ip++;
		} else {
			if (*ip < '\040') {
				/* error: shouldn't get control chars */
				goto dcorr;
			}
			switch (special) {
			case 0:
				c = *ip++;
				break;
			case '\172':
				c = *ip++ - 0100;
				break;
			case '\173':
				c = *ip++ + 0100;
				break;
			case '\174':
				c = *ip++ + 0100;
				break;
			case '\175':
				c = *ip++ + 0200;
				break;
			case '\176':
				c = *ip++ + 0300;
				break;
			}
			*op++ = c;
			if (sum & 0x8000) {
				sum <<= 1;
				sum++;
			} else
				sum <<= 1;
			sum += c & 0377;
			sum &= 0xffff;
			special = 0;
			nl++;
		}
	} while (--len);
	chksum = sum;
	DEBUG(8, "%d,", nl);
	return nl;
dcorr:
	DEBUG(8, "\n", 0);
	DEBUG(4, "Data corrupted\n", 0);
	while (len != FAIL) {
		if ((len = frdbuf(erbp, FBUFSIZ, fn)) != FAIL)
			*rlen += len;
	}
	return FAIL;
}
