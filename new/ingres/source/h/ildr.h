/*
 *		@(#)ildr.h	1.1	(ULTRIX)	1/8/85
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
** Lockdriver moved to user process 7/19/82
*/

/* Only the parameters NLOCKS, PLOCKS, RLOCKS and DLOCKS
 * may be changed by an INGRES installation.  See 
 * 'HOW TO INSTALL CONCURRENCY DEVICE' for details.
 * The file can be printed by "nroff .../doc/other/lockdev.nr"
 */

# define	DLOCKS	10	/* max number of data base locks (limits # of ingreses */
# define	RLOCKS	(2*DLOCKS + 6)
# define	PLOCKS	(RLOCKS + 3)
# define	NLOCKS	(PLOCKS + 1)
# define	KEYSIZE	12

# define	M_EMTY		0
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
# define	A_ABT		5

# define	W_ON		1
# define	W_OFF		0

struct Lockreq
			/* Lock Request */
{
	char	lr_act;		/* requested action:
				 *	=1 request lock, err return
				 *	=2 request lock, sleep
				 *	=3 release lock
				 *	=4 release all locks for pid
				 */
	char	lr_type;	/* same as Locktab l_type */
	char	lr_mod;		/* same as Locktab l_mod */
	char	lr_key[KEYSIZE];/* requested key	*/
};


# define	LOCKPRI	(PZERO+1)	/* sleep priority */

/*
 *	data structure for Lock table
 */
struct	Lockform
{
	int	l_pid;
	char	l_wflag;	/* wait flag: = 1 a process is waiting*/
	char	l_type;		/* type of lock:
					= 0 for critical section
					= 1 for page
					= 2 for logical
					= 3 for data base
				*/
	char	l_mod;		/* mod of Lock or lock action requested 
				 *	= 0 slot empty
				 *	= 1 exclusive lock
				 *	= 2 shared lock
				 */
	char 	l_key[KEYSIZE];
} Locktab[NLOCKS];

int	Lockset[] =
			/* array of number of locks which can be
			 * set for each lock.
			 */
{
	NLOCKS,
	PLOCKS,
	RLOCKS,
	DLOCKS
};

