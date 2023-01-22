/*
 *		@(#)state.h	1.1	(ULTRIX)	1/8/85
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
**  STATE.H -- definitions for parameter vectors
**
**	Version:
**		@(#)state.h	7.1	2/5/81
*/

# ifndef CM_MAXST


# define	CM_MAXST	40	/* maximum # of states */

/* the state descriptor type */
typedef struct
{
	char	st_stat;	/* status bits, see below */
	char	st_type;	/* the type, see below */
	union
	{
		struct			/* ST_REMOT */
		{
			char	st_proc;	/* the remote process */
		} st_rem;
		struct			/* ST_LOCAL */
		{
			char	st_funcno;	/* the function number to call */
			char	st_next;	/* the next state */
		} st_loc;
	} st_v;
} state_t;

/* bits for st_stat */
# define	ST_EXTERN	0001	/* can be executed by user */

/* values for st_type */
# define	ST_UNDEF	0	/* undefined state */
# define	ST_LOCAL	1	/* state exists in this proc */
# define	ST_REMOT	2	/* state exists in another proc */


# endif CM_MAXST
