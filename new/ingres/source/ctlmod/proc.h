/*
 *		@(#)proc.h	1.1	(ULTRIX)	1/8/85
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
**  PROC.H -- process descriptors
**
**	Version:
**		@(#)proc.h	7.1	2/5/81
*/

# ifndef CM_MAXPROC


# define	CM_MAXPROC	10	/* maximum # of procs */

typedef struct
{
	char	pr_stat;	/* status byte for this proc, see below */
	char	pr_file;	/* file descriptor to get to this proc */
	char	pr_ninput;	/* new cm_input after writing to this proc */
}  proc_t;

# define	PR_BCAST	00001	/* write broadcasts on this pipe */
# define	PR_RADJCT	00002	/* adjacent on read pipe */
# define	PR_WADJCT	00004	/* adjacent on write pipe */


# endif CM_MAXPROC
