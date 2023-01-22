/*
 *		@(#)opsys.h	1.2	(ULTRIX)	2/1/85
 */

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
/************************************************************************
 *			Modification History				*
 *									*
 *	David L Ballenger, 1-Feb-1985					*
 * 001	Undefine NULL if <sys/param.h> is included, so that we don't	*
 *	get redefinition errors.					*
 *									*
 ************************************************************************/

#

/*
**  OPSYS.H -- operating system dependent definitions.
**
**	This header file contains everything that we at Berkeley
**	think might vary from system to system.  Before
**	compiling INGRES you should eyeball this file to see
**	if your system is consistant with us.
**
**	Version:
**		@(#)opsys.h	7.2	5/19/83
*/

# ifndef USERINGRES

/*
**  UNIX version flags.
**	The following flags define what version of UNIX is being run.
**	They should be set as follows:
**
**	Bell-style version six UNIX, with 8-bit user id's and 8-bit
**	group id's, fetched with calls to getuid() and getgid()
**	respectively, and stored in the passwd file as two separate
**	eight-bit fields:
**		Set the xV6_UNIX flag only.
**
**	Berkeley-style version 6/7x-05a UNIX, with a single 16-bit
**	user id and no group id (and no getgid() or setgid() calls),
**	and stored in the passwd file as two separate eight-bit fields,
**	combined to make a single 16-bit field:
**		Set the xB_UNIX flag only.
**
**	Bell-style version seven UNIX, with 16-bit user id's and
**	16-bit group id's, fetched with calls to getuid() and
**	getgid() respectively, and stored in the passwd file as
**	two separate 16-bit fields:
**		Set the xV7_UNIX flag only.
*/

/* set for version six */
/* # define	xV6_UNIX	/* Bell v6 UNIX flag */
# define	xV7_UNIX	/* Bell v7 UNIX flag */
/* # define	xB_UNIX		/* Berkeley UNIX flag */


/*
**  Maximum number of open files per process.
**  Must match 'NOFILE' entry in /usr/sys/param.h
*/

/*
** if xV7_UNIX is defined, we include <sys/param.h>, which will
** define NOFILE, so we avoid defining it here.
*/
# ifndef	xV7_UNIX
# define	NOFILE		20
# endif		xV7_UNIX


/*
**	USERINGRES is the UNIX login name of the INGRES superuser,
**		normally "ingres" of course.  The root of this persons
**		subtree as listed in /etc/passwd becomes the root of
**		the INGRES subtree.
*/

# define	USERINGRES	"ingres"


/*
**  Structure for 'gtty' and 'stty'
*/

# ifndef xV7_UNIX
struct sgttyb
{
	char	sg_ispeed;
	char	sg_ospeed;
	char	sg_erase;
	char	sg_kill;
	int	sg_flags;
};
# else
# include	<sgtty.h>
# endif xV7_UNIX


/*
**  Structure for 'fstat' and 'stat' system calls.
*/

# ifndef xV7_UNIX
struct stat {
	short	st_dev;		/* +0: device of i-node */
	short	st_ino;		/* +2 */
	short	st_mode;	/* +4: see below */
	char	st_nlink;	/* +6: number of links to file */
	char	st_uid;		/* +7: user ID of owner */
	char	st_gid;		/* +8: group ID of owner */
	char	st_sz0;		/* +9: high byte of 24-bit size */
	int	st_sz1;		/* +10: low word of 24-bit size */
	int	st_addr[8];	/* +12: block numbers or device number */
	int	st_atime[2];	/* +28: time of last access */
	int	st_mtime[2];	/* +32: time of last modification */
};

#define		IALLOC	0100000
#define		S_IFMT	060000
#define		S_IFDIR	040000
#define		IFCHR	020000
#define		IFBLK	060000
# endif
# ifdef xV7_UNIX

# ifdef NULL
# undef	NULL	/* Undefine NULL, because <sys/param.h> defines it. DLB001 */
# endif NULL
# ifdef bitset
# undef		bitset
# undef		setbit
# undef		clrbit
# include	<sys/param.h>
# include	<sys/stat.h>
# undef		bitset
# undef		setbit
# undef		clrbit
# define	bitset(bit, word)	((bit) & (word))
# define	setbit(bit, word)	word |= bit
# define	clrbit(bit, word)	word &= ~bit
# else  bitset
# include	<sys/param.h>
# include	<sys/stat.h>
# undef		bitset
# undef		setbit
# undef		clrbit
# endif bitset

# endif xV7_UNIX


# endif USERINGRES
