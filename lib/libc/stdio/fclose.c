#ifndef lint
static	char	*sccsid = "@(#)fclose.c	1.3	(ULTRIX)	9/23/85";
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
 * 005	David L Ballenger, 05-Sep-1985
 *	Have fflush() reset all information in the iob to look as if
 *	there is nothing to write, before doing the write().  This
 *	prevents a signal handler from getting into an infinite loop
 *	if it attempts to fclose() / fflush() the file and the write 
 *	to the file causes the signal which the handler is catching.
 *
 *	David L Ballenger, 08-Aug-1985
 * 004	Have fclose() make sure the file is open for output before
 *	attempting to flush it.
 *
 *	David L Ballenger, 01-Aug-1985
 * 003	Move fflush() and fclose() into a separate file so that _flsbuf()
 *	is not included unneccesarily.  Also add code to fflush() to make
 *	System V style append mode work correctly.
 *
 *	David L Ballenger, 15-Jul-1985
 * 002	Clean up buffer handling for unbuffered files.
 *
 *	David L Ballenger, 26-Jun-1985
 * 001	Clean up buffer allocation.  Also fix problems with files opened
 *	in append mode.
 *
 *	Based on:  flsbuf.c	4.6 (Berkeley) 6/30/83
 *
 ************************************************************************/

#include	<stdio.h>

fflush(iop)
	register FILE *iop;
{
	register char 	*base ;
	register int	n_chars ;

	/* Make sure this isn't readonly
	 */
	if ((iop->_flag & (_IOREAD|_IORW)) == _IOREAD) {
		iop->_flag |= _IOERR;
		return(EOF);
	}

	/* See if there is anything to flush.
	 */
	if ( iop->_flag & _IOWRT && (base=iop->_base) != NULL )
		n_chars = iop->_ptr - base ;
	else 
		n_chars = 0 ;

	/* Reset the FILE info to indicate that the buffers
	 * have been flushed.  For {line,un}buffered, update, or
	 * System V append mode files _cnt is set to 0 so that
	 * the next putc() / getc() will call _flsbuf() / _filbuf().
	 * In update mode the write flag is turned off to indicate
	 * that there are no characters in the file to be written.
	 */
	iop->_ptr = iop->_base;
	iop->_cnt = (iop->_flag & (_IOLBF|_IONBF|_IORW|_IOAPPEND)) 
	          ? 0 : iop->_bufsiz ;
	if (iop->_flag & _IORW) 
		iop->_flag &= ~_IOWRT ;

	/* Now flush the buffer if there is actually anything to write.
	 * This is done after all the flags are reset, so that an
	 * interrupted write won't cause an infinite loop, if the signal
	 * handler tries to flush/close this file again.  This can happen
	 * when a pclose() is done as a result of a SIGPIPE signal, without
	 * the calling program setting the action for SIGPIPE to SIG_IGN.
	 */
	if (n_chars > 0) {
		if (write(fileno(iop),base,n_chars) != n_chars){
			iop->_flag |= _IOERR;
				return(EOF) ;
		}
	}
	
	return(0);
}

fclose(iop)
	register FILE *iop;
{
	register int r;

	r = EOF;
	if (iop->_flag&(_IOREAD|_IOWRT|_IORW) && (iop->_flag&_IOSTRG)==0) {

		/* Flush the file if it is open for output, then close it
		 * and free any buffers allocated by stdio.
		 */
		if (iop->_flag & (_IOWRT|_IORW))
			r = fflush(iop);
		else
			r = 0;
		if (close(fileno(iop)) < 0)
			r = EOF;
		if (iop->_flag&_IOMYBUF)
			free(iop->_base);
	}

	iop->_cnt = 0;
	iop->_base = (char *)NULL;
	iop->_ptr = (char *)NULL;
	iop->_bufsiz = 0;
	iop->_flag = 0;
	iop->_file = 0;
	return(r);
}
