/*
 *		@(#)lock.h	1.1	(ULTRIX)	1/8/85
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

/*
**  LOCK.H -- Concurency structs and global variables
**
**	Version:
**		@(#)lock.h	7.1	2/5/81
*/

# ifndef KEYSIZE


# define	M_SHARE		2
# define	M_EXCL		1
# define	T_CS		0
# define	T_PAGE		1
# define	T_REL		2
# define	T_DB		3
# define	A_RTN		1
# define	A_SLP		2
# define	A_RLS1		3
# define	A_RLSA		4
# define	KEYSIZE		12
struct lockreq
{
	char	lract;			/* requested action
					 *	=1 request lock,err return
					 *	=2 request lock,sleep
					 *	=3 release lock
					 *	=release all locks for pid
					 */
	char	lrtype;			/* type of lock:
					 *   =0, critical section lock
					 *   =1, page lock
					 *   =2, logical lock
					 *   =3, data base lock
					 */
	char	lrmod;			/* mode of lock
					 *	=1 exclusive lock
					 *	=2 shared lock
					*/
					/* key for the lock */
	char	dbnode[4];		/* inode of data base */
	char	lrel[4];		/* relation tid */
	char	lpage[4];		/* page address		*/
};

extern char	Acclock;		/* locks enabled flag */
extern int	Alockdes;		/* file descriptor for lock device*/
extern int	Lockrel;		/* lock relations flag*/


# endif KEYSIZE
