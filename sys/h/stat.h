
/*	@(#)stat.h	1.8	(ULTRIX)	12/4/86	*/

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
 *	Paul Shaughnessy (prs), 04-Dec-1986				*
 * 0002 Modified usr/group compatability section.			*
 *									*
 *	David L Ballenger, 28-Mar-1985					*
 * 0001 Add definitions from BRL package for System V support		*
 *									*
 ************************************************************************/


struct	stat
{
	dev_t	st_dev;
	ino_t	st_ino;
	u_short st_mode;
	short	st_nlink;
	short	st_uid;
	short	st_gid;
	dev_t	st_rdev;
	off_t	st_size;
	time_t	st_atime;
	int	st_spare1;
	time_t	st_mtime;
	int	st_spare2;
	time_t	st_ctime;
	int	st_spare3;
	long	st_blksize;
	long	st_blocks;
	long	st_spare4[2];
};

#define	S_GFMT	0170000		/* type of file */
#define		S_GFDIR	0040000	/* directory */
#define		S_GFCHR	0020000	/* character special */
#define		S_GFBLK	0060000	/* block special */
#define		S_GFREG	0100000	/* regular */
#define		S_GFLNK	0120000	/* symbolic link */
#define		S_GFSOCK 0140000/* socket */
#define		S_IFIFO	0010000	/* FIFO - named pipe */
#define		S_GFPORT S_IFIFO/* Used for FIFOs in kernal? */
#define	S_GSUID	0004000		/* set user id on execution */
#define	S_GSGID	0002000		/* set group id on execution */
#define	S_GSVTX	0001000		/* save swapped text even after use */
#define	S_GREAD	0000400		/* read permission, owner */
#define	S_GWRITE 0000200	/* write permission, owner */
#define	S_GEXEC	0000100		/* execute/search permission, owner */

/* the following are included for compatibility with non-gfs modules */


#define	S_IFMT	S_GFMT
#define		S_IFDIR	S_GFDIR
#define		S_IFCHR	S_GFCHR
#define		S_IFBLK	S_GFBLK
#define		S_IFREG	S_GFREG
#define		S_IFLNK	S_GFLNK
#define		S_IFSOCK	S_GFSOCK
#define		S_IFPORT	S_GFPORT
#define	S_ISUID	S_GSUID
#define	S_ISGID	S_GSGID
#define	S_ISVTX	S_GSVTX
#define	S_IREAD	S_GREAD
#define	S_IWRITE	S_GWRITE
#define	S_IEXEC	S_GEXEC



/* Additions from BRL package to support /usr/group standard
 */
#ifdef USRGROUP
/* DAG -- The following added to support the /usr/group Standard: */

/* macro to test for block special file */
#define	S_ISBLK( mode )		(((mode) & S_GFMT) == S_GFBLK)

/* macro to test for character special file */
#define	S_ISCHR( mode )		(((mode) & S_GFMT) == S_GFCHR)

/* macro to test for directory file */
#define	S_ISDIR( mode )		(((mode) & S_GFMT) == S_GFDIR)

/* macro to test for fifo special file */
#define	S_ISFIFO( mode )	(((mode) & S_GFMT) == S_IFIFO)

/* macro to test for regular file */
#define	S_ISREG( mode )		(((mode) & S_GFMT) == S_GFREG)

#define	S_ENFMT	S_GSGID		/* record locking enforcement flag */

#define	S_IRWXU	00700		/* read, write, execute: owner */
#define	S_IRUSR	00400		/*  read permission: owner */
#define	S_IWUSR	00200		/*  write permission: owner */
#define	S_IXUSR	00100		/*  execute permission: owner */

#define	S_IRWXG	00070		/* read, write, execute: group */
#define	S_IRGRP	00040		/*  read permission: group */
#define	S_IWGRP	00020		/*  write permission: group */
#define	S_IXGRP	00010		/*  execute permission: group */

#define	S_IRWXO	00007		/* read, write, execute: other */
#define	S_IROTH	00004		/*  read permission: other */
#define	S_IWOTH	00002		/*  write permission: other */
#define	S_IXOTH	00001		/*  execute permission: other */

/* these are included for compatibility */

#define	S_GSBLK S_ISBLK	
#define	S_GSCHR S_ISCHR	
#define	S_GSDIR S_ISDIR	
#define	S_GSFIFO S_ISFIFO 
#define S_GFIFO S_IFIFO
#define	S_GSREG S_ISREG	

#define	S_GRWXU S_IRWXU	
#define	S_GRUSR S_IRUSR	
#define	S_GWUSR S_IWUSR	
#define	S_GXUSR S_IXUSR	

#define	S_GRWXG S_IRWXG	
#define	S_GRGRP S_IRGRP	
#define	S_GWGRP S_IWGRP	
#define	S_GXGRP S_IXGRP	

#define	S_GRWXO S_IRWXO	
#define	S_GROTH S_IROTH	
#define	S_GWOTH S_IWOTH	
#define	S_GXOTH S_IXOTH	

#endif USRGROUP
