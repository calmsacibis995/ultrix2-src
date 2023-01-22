/*
 * get.c
 */

#ifndef lint
static	char	*sccsid = "@(#)get.c	1.2	8/21/85";
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
/*	get.c	6.1 (extracted from prf.c) 83/07/29	*/

#include "../h/param.h"

#include "../vax/mtpr.h"
#include "../vax/cons.h"

static int delflg = 0;


#ifdef VCONS
extern (*v_getc)();
#endif VCONS
getchar()
{
	register c;

#ifdef VCONS
	if( v_getc ) 
		c = (*v_getc)();
	else {
#endif VCONS
		while((mfpr(RXCS)&RXCS_DONE) == 0)
			;
		c = mfpr(RXDB)&0177;
#ifdef VCONS
	}
#endif VCONS
	if (c=='\r')
		c = '\n';
	if (!delflg)
		putchar(c);
	return(c);
}

gets(buf)
	char *buf;
{
	register char *lp;
	register c;

	lp = buf;
	for (;;) {
		c = getchar() & 0177;
	store:
		switch(c) {
		case '\n':
		case '\r':
			if (delflg) {
				putchar('/');
				putchar(c);
				delflg = 0;
				}
			c = '\n';
			*lp++ = '\0';
			return;
		case '':			/* Delete key */
			if (lp != buf)
				if (!delflg) {
					delflg++;
					putchar('\\');
					}
			if (lp > buf)
				putchar(*--lp);
			if (lp < buf)
				lp = buf;
			continue;
		/*
		 * Backspace and pound sign are left for backward
		 * compatability.  Strange results are guaranteed if
		 * either of these are mixed with <del> on the same
		 * input line.
		 */
		case '\b':
		case '#':
			delflg = 0;
			lp--;
			if (lp < buf)
				lp = buf;
			continue;
		case '@':
		case '':			/* Control U */
			delflg = 0;
			lp = buf;
			printf("^U\n");
			continue;
		default:
			if (delflg) {
				putchar('/');
				putchar(c);
				delflg = 0;
				}
			*lp++ = c;
		}
	}
}
