/*	@(#)dir.h	1.7	(ULTRIX)	12/16/86	*/

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
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
 * A directory consists of some number of blocks of DIRBLKSIZ
 * bytes, where DIRBLKSIZ is chosen such that it can be transferred
 * to disk in a single atomic operation (e.g. 512 bytes on most machines).
 *
 * Each DIRBLKSIZ byte block contains some number of directory entry
 * structures, which are of variable length.  Each directory entry has
 * a struct direct at the front of it, containing its inode number,
 * the length of the entry, and the length of the name contained in
 * the entry.  These are followed by the name padded to a 4 byte boundary
 * with null bytes.  All names are guaranteed null terminated.
 * The maximum length of a name in a directory is MAXNAMLEN.
 *
 * The macro DIRSIZ(dp) gives the amount of space required to represent
 * a directory entry.  Free space in a directory is represented by
 * entries which have dp->d_reclen > DIRSIZ(dp).  All DIRBLKSIZ bytes
 * in a directory block are claimed by the directory entries.  This
 * usually results in the last entry in a directory having a large
 * dp->d_reclen.  When entries are deleted from a directory, the
 * space is returned to the previous entry in the same directory
 * block by increasing its dp->d_reclen.  If the first entry of
 * a directory block is free, then its dp->d_ino is set to 0.
 * Entries other than the first in a directory do not normally have
 * dp->d_ino set to 0.
 */
#ifndef	_DIRINCLUDE
#define	_DIRINCLUDE
/* so user programs can just include dir.h */
#if	!defined(KERNEL) && !defined(DEV_BSIZE)
#define	DEV_BSIZE	512
#endif	NOT KERNEL && NOT DEV_BSIZE
#define	DIRBLKSIZ	DEV_BSIZE
#define	MAXNAMLEN	255

/* all this garbage should be moved to a ufs specific header file
 * the only things that should be in this file should be gen_dir
 * and MAXNAMLEN
 */

/* NB:
 *	gd_name may be shorter than MAXNAMLEN.  we try to pack as
 *	much data into the alloted user space as we can (hence
 *	the gd_reclen.
 */

struct gen_dir {
		u_long	gd_ino;		/* not meaningful for all sfs types */
		u_short	gd_reclen;	/* how large this structure really is */
		u_short	gd_namelen;	/* how long the named component is */
		char	gd_name[MAXNAMLEN + 1]; /* what the name is */
	};
	
#define	direct		gen_dir
#define	d_ino		gd_ino
#define	d_fileno	gd_ino		/* sun compatibility */
#define	d_reclen	gd_reclen
#define	d_namlen	gd_namelen
#define	d_name		gd_name

/*
 * The DIRSIZ macro gives the minimum record length which will hold
 * the directory entry.  This requires the amount of space in struct direct
 * without the d_name field, plus enough space for the name with a terminating
 * null byte (dp->d_namlen+1), rounded up to a 4 byte boundary.
 */
#undef	DIRSIZ
#define	DIRSIZ(dp) \
    ((sizeof (struct direct) - (MAXNAMLEN+1)) + (((dp)->d_namlen+1 + 3) &~ 3))

#ifndef KERNEL
/*
 * Definitions for library routines operating on directories.
 */
typedef	struct _dirdesc {
	int	dd_fd;
	long	dd_loc;
	long	dd_size;
	long	dd_bbase;
	long	dd_entno;
	long	dd_bsize;
	char *	dd_buf;
} DIR;

#ifndef	NULL
#define	NULL 0
#endif	NULL
extern	DIR *opendir();
extern	struct direct *readdir();
extern	long telldir();
extern	void seekdir();
#define rewinddir(dirp) seekdir((dirp), (long)0)
extern	void closedir();
#endif	NOT KERNEL

#ifdef	KERNEL
/*
 * Template for manipulating directories.
 * Should use struct direct's, but the name field
 * is MAXNAMLEN - 1, and this just won't do.
 */
struct dirtemplate {
	u_long	dot_ino;
	short	dot_reclen;
	short	dot_namlen;
	char	dot_name[4];		/* must be multiple of 4 */
	u_long	dotdot_ino;
	short	dotdot_reclen;
	short	dotdot_namlen;
	char	dotdot_name[4];		/* ditto */
};
#endif KERNEL
#endif _DIRINCLUDE
