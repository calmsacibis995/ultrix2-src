/*
 *		@(#)unistd.h	1.2	(ULTRIX)	10/21/85
 */

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
 * 001	David L Ballenger, 17-Oct-1985
 *	Change defintion of IN_PATH so that it is not BRL specific.
 *
 *	Based on:  unistd.h	1.1 (BRL UNIX System V emulation version)
 *		   85/02/24	D A Gwyn
 *
 ************************************************************************/

#ifndef	_UNISTD_H_
#define	_UNISTD_H_			/* once-only latch */

/* Symbolic constants for the "access" function: */
#define	R_OK	4	/* Test for "Read Permission */
#define	W_OK	2	/* Test for "Write Permission */
#define	X_OK	1	/* Test for "Execute" (Search) Permission */
#define	F_OK	0	/* Test for existence of file */

/* Symbolic constants for the "lockf" function: */
#define	F_ULOCK	0	/* Unlock a previously locked region */
#define	F_LOCK	1	/* Lock a region for exclusive use */
#define	F_TLOCK	2	/* Test and lock a region for exclusive use */
#define	F_TEST	3	/* Test region for other processes' locks */

/* Symbolic constants for the "lseek" function: */
#define	SEEK_SET 0	/* Set file pointer to offset */
#define	SEEK_CUR 1	/* Set file pointer to its current value plus offset */
#define	SEEK_END 2	/* Set file pointer to the size of the file plus offset */

/* Path names: */
#define	GF_PATH	"/etc/group"	/* Path name of the "group" file */
#define	PF_PATH	"/etc/passwd"	/* Path name of the "passwd" file */
#define	IN_PATH	"/usr/include"	/* Path name for <...> directory */

/* >>> Structure for "utime" function not included in AT&T version <<< */

#endif	_UNISTD_H_
