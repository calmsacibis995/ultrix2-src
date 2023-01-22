#ifndef lint
static	char	*sccsid = "@(#)fseek.c	1.2	(ULTRIX)	8/2/85";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
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
/************************************************************************
 *			Modification History
 *
 *	David L Ballenger, 01-Aug-1985
 * 001	Make System V style append mode work correctly.
 *
 *	Based on:  fseek.c	4.3 (Berkeley) 9/25/83
 *
 ************************************************************************/

/*
 * Seek for standard library.  Coordinates with buffering.
 */

#include	<stdio.h>

long lseek();

fseek(iop, offset, ptrname)
FILE *iop;
long offset;
{
	register resync, c;
	long p;

	iop->_flag &= ~_IOEOF;
	if (iop->_flag&_IOREAD) {
		if (ptrname<2 && iop->_base &&
			!(iop->_flag&_IONBF)) {
			
			/* In System V append mode _cnt is stored as a
			 * negative number.  See _filbuf() for details.
			 */
			c = (iop->_flag&_IOAPPEND) ? -iop->_cnt : iop->_cnt ;
			p = offset;
			if (ptrname==0)
				p += c - lseek(fileno(iop),0L,1);
			else
				offset -= c;
			if(!(iop->_flag&_IORW) && c>0&&p<=c
			    && p>=iop->_base-iop->_ptr){
				iop->_ptr += (int)p;

				/* In System V append mode _cnt is stored as a
				 * negative number.  See _filbuf() for
				 * details.
				 */
				if (iop->_flag&_IOAPPEND)
					iop->_cnt += (int)p;
				else
					iop->_cnt -= (int)p;
				return(0);
			}
			resync = offset&01;
		} else 
			resync = 0;
		if (iop->_flag & _IORW) {
			iop->_ptr = iop->_base;
			iop->_flag &= ~_IOREAD;
			resync = 0;
		}
		p = lseek(fileno(iop), offset-resync, ptrname);
		iop->_cnt = 0;
		if (resync)
			getc(iop);
	}
	else if (iop->_flag & (_IOWRT|_IORW)) {
		
		/* Have to flush anything in the output buffer before
		 * doing the lseek().  Fflush() will now reset the
		 * the iop fields properly.
		 */
		fflush(iop);
		p = lseek(fileno(iop), offset, ptrname);
	}
	return(p==-1?-1:0);
}
