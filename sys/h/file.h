/* @(#)file.h	1.12 (ULTRIX) 6/10/86 */

/************************************************************************
 *									*
 *			Copyright (c) 1985, 1986 by			*
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
 *			Modification History				*
 *									*
 *	11-Mar-86	Larry Palmer 					*
 *	Added flag to mark a file as being used by n-bufferring.
 *									*
 *	Stephen Reilly, 09-Sept-85					*
 * 	Modified to handle the new lockf code.				*
 *									*
 *	Stephen Reilly, 09-Sept-85					*
 * 	Modified to handle the new lockf code.				*
 *
 *	David L Ballenger, 28-Mar-1985					*
 * 0001	Modify so that <fcntl.h> can simply include this file.  This	*
 *	file already contains all the definitions contained in		*
 *	<fcntl.h>, plus all the definitions that would need to be added	*
 *	to <fcntl.h> from the BRL version for System V emulation.  No	*
 *	sense in defining things in multiple places.			*
 *									*
 *	Larry Cohen, 4-April-1985					*
 * 0002 Changes to support open block if in use capability		*
 *	FBLKINUSE							*
 *	O_BLKINUSE - open blocks if IINUSE bit in inode is set		*
 *									*
 *	FBLKANDSET							*
 *	O_BLKANDSET - open blocks if IINUSE bit in inode is set		*
 *			and sets after open succeeds.			*
 *									*
 * 	Greg Depp 8-April-1985						*
 * 0003 Added DTYPE_PORT to define System V IPC Named Pipe type		*
 *									*
 *	Paul Shaughnessy, 24-December-1985				*
 * 0004 Added syncronous write capability in open and fcntl system      *
 *	calls.								*
 *									*
 *	Tim Burke,	10-June-1986					*
 * 0005 Inserted FSYSV and O_SYSV which tells if the program is to	*
 *	run as a System V program.  If so use a "RAW" type of default   *
 *	settings rather that the normal "COOKED" style on terminal 	*
 *	line defaults.							*
 *									*
 ************************************************************************/


/* Don't define things twice.  This protects form a source file which
 * includes both <fcntl.h> and <sys/file.h>
 */
#ifndef __FCNTL__
#define __FCNTL__

#ifdef KERNEL
/*
 * Descriptor table entry.
 * One for each kernel object.
 */
struct	file {
	int	f_flag;		/* see below */
	short	f_type;		/* descriptor type */
	short	f_count;	/* reference count */
	short	f_msgcount;	/* references from message queue */
	struct	fileops {
/*		return	function	arguments		*/
		int	(*fo_rw)(	/* fp,uio,rw		*/ );
		int	(*fo_ioctl)(	/* fp,com,data,cred	*/ );
		int	(*fo_select)(	/* fp,which		*/ );
		int	(*fo_close)(	/* fp			*/ );
	} *f_ops;
	caddr_t	f_data;		/* inode */
	off_t	f_offset;
	struct ucred *f_cred;
};

struct	file *file, *fileNFILE;
int	nfile;
struct	file *getf();
struct	file *falloc();
#endif

/*
 * flags- also for fcntl call.
 */
#define	FOPEN		(-1)
#define	FREAD		00001		/* descriptor read/receive'able */
#define	FWRITE		00002		/* descriptor write/send'able */
#define	FNDELAY		00004		/* no delay */
#define	FAPPEND		00010		/* append on each write */
#define	FMARK		00020		/* mark during gc() */
#define	FDEFER		00040		/* defer for next gc pass */
#define	FASYNC		00100		/* signal pgrp when data ready */
#define	FSHLOCK		00200		/* shared lock present */
#define	FEXLOCK		00400		/* exclusive lock present */
#define FSYNCRON	0100000		/* Write file syncronously *0004*/
#define FNBUF		0200000		/* file used for n-buffering */
/* bits to save after open */
#define	FMASK		0110113     /* 0004 */
#define	FCNTLCANT	(FREAD|FWRITE|FMARK|FDEFER|FSHLOCK|FEXLOCK)

/* open only modes */
#define	FCREAT		01000		/* create if nonexistant */
#define	FTRUNC		02000		/* truncate to zero length */
#define	FEXCL		04000		/* error if already created */
#define FBLKINUSE      010000		/* block if "in use"	*0002*/
#define FBLKANDSET     (020000 | FBLKINUSE) /* block, test and set "in use" */	
#define FSYSV		0200000		/* System V style program */

/* fcntl(2) requests
 */
#define	F_DUPFD	0	/* Duplicate fildes */
#define	F_GETFD	1	/* Get fildes flags */
#define	F_SETFD	2	/* Set fildes flags */
#define	F_GETFL	3	/* Get file flags */
#define	F_SETFL	4	/* Set file flags */
#define	F_GETOWN 5	/* Get owner */
#define F_SETOWN 6	/* Set owner */
#define	F_GETLK	7	/* Get file lock */
#define	F_SETLK	8	/* Set file lock */
#define	F_SETLKW 9	/* Set file lock and wait */
#define F_SETSYN 10	/* Set syncronous write *0004*/
#define F_CLRSYN 11	/* Clear syncronous write *0004*/

/*
 * User definitions.
 */

/* file segment locking set data type - information passed to system by user */
#ifndef	F_RDLCK
struct flock {
	short	l_type;
	short	l_whence;
	long	l_start;
	long	l_len;		/* len = 0 means until end of file */
	int	l_pid;
};
#endif

/*
 * Open call.
 */
#define	O_RDONLY	000		/* open for reading */
#define	O_WRONLY	001		/* open for writing */
#define	O_RDWR		002		/* open for read & write */
#define	O_NDELAY	FNDELAY		/* non-blocking open */
#define	O_APPEND	FAPPEND		/* append on each write */
#define	O_CREAT		FCREAT		/* open with file create */
#define	O_TRUNC		FTRUNC		/* open with truncation */
#define	O_EXCL		FEXCL		/* error on create if file exists */
#define O_BLKINUSE      FBLKINUSE	/* block if "in use"	*0002*/
#define O_BLKANDSET     FBLKANDSET	/* block, test and set "in use"	*/
#define O_FSYNC		FSYNCRON	/* syncronous write *0004*/
#define O_SYNC		O_FSYNC		/* system V synchronous write */
#define O_SYSV		FSYSV		/* system V style program */
/*
 * Flock call.
 */
#define	LOCK_SH		1	/* shared lock */
#define	LOCK_EX		2	/* exclusive lock */
#define	LOCK_NB		4	/* don't block when locking */
#define	LOCK_UN		8	/* unlock */

/* 
 *	file segment locking types
 */
#define	F_RDLCK	01	/* Read lock */
#define	F_WRLCK	02	/* Write lock */
#define	F_UNLCK	03	/* Remove lock(s) */

/*
 * Access call.
 */
#define	F_OK		0	/* does file exist */
#define	X_OK		1	/* is it executable by caller */
#define	W_OK		2	/* writable by caller */
#define	R_OK		4	/* readable by caller */

/*
 * Lseek call.
 */
#define	L_SET		0	/* absolute offset */
#define	L_INCR		1	/* relative to current offset */
#define	L_XTND		2	/* relative to end of file */

#ifdef KERNEL
#define	GETF(fp, fd) { \
	if ((unsigned)(fd) >= NOFILE || ((fp) = u.u_ofile[fd]) == NULL) { \
		u.u_error = EBADF; \
		return; \
	} \
}
#define	DTYPE_INODE	1	/* file */
#define	DTYPE_SOCKET	2	/* communications endpoint */
#define	DTYPE_PORT	3	/* port (named pipe) 0003 */
#endif KERNEL

#endif __FCNTL__
