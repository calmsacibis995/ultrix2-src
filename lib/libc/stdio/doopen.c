#ifndef lint
static char *sccsid = "@(#)doopen.c	1.3	ULTRIX	10/3/86";
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
 * 002  Returned proper error number in errno if error didn't occur in
 *	open system call.
 *
 *	David L Ballenger, 01-Aug-1985
 * 001	Set _IOAPPEND flag for files opened with "A" or "A+" mode.  This
 *	provides System V style "append" mode on ULTRIX.
 *
 ************************************************************************/

/* _doopen()
 *
 * Common routine to open a file for both fopen() and freopen().
 *
 */

#include	<stdio.h>
#include	<sys/file.h>
#include	<sys/types.h>
#include 	<errno.h>

FILE *
_doopen(file, mode, iop)
	char *file;
	register char *mode;
	register FILE *iop;
{
	register unsigned o_flags;

	if (iop == NULL) {
		/*002 - invalid arguement, set errno */ 
		errno = EINVAL;
		return(NULL);
	}
	/* Check the basic mode for the file.
	 */
	switch(mode[0]) {
		case 'r': o_flags = O_RDONLY ;
			  iop->_flag = _IOREAD ;
			  break ;
		case 'w': o_flags = O_TRUNC|O_CREAT|O_WRONLY ;
			  iop->_flag = _IOWRT ;
			  break ;

			/* Append mode is handled differently in the System
			 * V environment.  In that environment files opened
			 * with "a" or "a+" are really opened with O_APPEND.
			 * This functionality is provided by the "A" and "A+"
			 * modes in the ULTRIX environment.
			 */
		case 'a':
#ifndef SYSTEM_FIVE	  /* Same as BSD "append modes" */
			  o_flags = O_CREAT|O_WRONLY ;
			  iop->_flag = _IOWRT ;
			  break ;
		case 'A':
#endif			  /* Same as System V "append" modes */
			  o_flags = O_APPEND|O_CREAT|O_WRONLY ;
			  iop->_flag = _IOAPPEND|_IOWRT ;
			  break ;
		default:
			/*002 Invalid argument, set errno */
			errno = EINVAL;   
			return(NULL);
	}

	if (mode[1] == '+') {
		/* 
		 * In update mode.  Turn off the the readonly/writeonly
		 * flags and turn on the read/write flags.
		 */
		o_flags &= ~(O_RDONLY|O_WRONLY) ;
		o_flags |= O_RDWR ;
		iop->_flag &= ~(_IOREAD|_IOWRT) ;
		iop->_flag |= _IORW ;

	} else if (mode[1] != '\000') {
		iop->_flag = 0 ;
		errno = EINVAL;		/* 002 - set errno */ 
		return(NULL);		/* Illegal mode */
	}
	
	/* Try to open the file.
	 */
	if ((iop->_file = open(file,o_flags,0666)) < 0) {
		/* 002 - errno will be set to the correct 
		 * error message number by open sys call
		 */ 
		iop->_flag = 0 ;
		return(NULL);
	}

	iop->_cnt = 0;

	/* If in append mode seek to the end of the file
	 */
	if ( mode[0] == 'a'
#ifndef SYSTEM_FIVE
	     || mode[0] == 'A'
#endif
	   )
		(void)lseek(fileno(iop),(off_t)0,L_XTND);
	return(iop);
}
