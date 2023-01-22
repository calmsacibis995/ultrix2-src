#ifndef lint
static char *sccsid = "@(#)fdopen.c	1.6	ULTRIX	10/3/86";
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
 * 	Lu Anne Van de Pas, 22-may-1986
 * 004  returned proper error number in errno if error didn't occur in
 *	open system call.
 *
 *	David L Ballenger, 21-Aug-1985
 * 003	Correct test for null character at end of string.
 *
 *	David L Ballenger, 01-Aug-1985
 * 002	Set the _IOAPPEND flag for files "opened" in append mode ("A" or
 *	"A+").  This means the file was opened with the O_APPEND flag and
 *	all writes will go to the end of the file even if the file has
 *	been repositioned.
 *
 *	David L Ballenger, 24-Jun-1985
 * 001	Use _findiop() to get first free FILE structure.  This allows 
 *	them to be dynamically allocated.
 *
 *	Based on:  fdopen.c	4.4 (Berkeley) 9/12/83
 *
 ************************************************************************/


/*
 * Unix routine to do an "fopen" on file descriptor
 * The mode has to be repeated because you can't query its
 * status
 */

#include 	<errno.h>
#include	<sys/types.h>
#include	<sys/file.h>
#include	<stdio.h>

extern FILE *_findiop();

FILE *
fdopen(fd, mode)
register char *mode;
{
	static int	n_files = 0;
	register FILE 	*iop;

	/* Get the size of the file descriptor table on the first call
	 * to this routine.
	 */
	if (n_files == 0)
		n_files = getdtablesize();

	/* Make sure we have a valid file descriptor.
	 */
	if ( (fd < 0) || (fd >= n_files)) {
		/* 004 -invalid arguement, set errno */
		errno = EINVAL;
		return (NULL);
	}

	/* Call _findiop(), to get a pointer to an unused FILE structure.
	 */
	iop = _findiop();
	if (iop != NULL) {

		/* Initialize the structure, then return the pointer
		 */
		iop->_cnt = 0;
		iop->_file = fd;
		iop->_bufsiz = 0;
		iop->_base = iop->_ptr = NULL ;

		switch (*mode) {
			case 'r':
				iop->_flag = _IOREAD ;
				break;

			/* Append mode is handled differently in the System
			 * V environment.  In that environment files opened
			 * with "a" or "a+" are really opened with O_APPEND.
			 * This functionality is provided by the "A" and "A+"
			 * modes in the ULTRIX environment.
			 */
			case 'a': 
#ifndef SYSTEM_FIVE		/* Same as BSD "append modes" */
				(void)lseek(fd, (off_t)0, L_XTND);
				iop->_flag = _IOWRT;
				break;
			case 'A':
#endif				/* Same as System V "append" modes */
				(void)lseek(fd, (off_t)0, L_XTND);
				iop->_flag = _IOWRT|_IOAPPEND;
				break;
			case 'w':
				iop->_flag = _IOWRT ;
				break ;
			default:
				/* 004 Invalid arguement, set errno */
				errno = EINVAL;
				return(NULL) ;
		}
		mode++;
		if (*mode == '+') {
			mode++;
			iop->_flag &= ~(_IOREAD|_IOWRT) ;
			iop->_flag |= _IORW ;
		} 
		if (*mode != '\000') {
			iop->_flag = 0;
			/* 004 Invalid, set errno */
			errno = EINVAL; 
			return(NULL);
		}
	}
	return(iop);
}
