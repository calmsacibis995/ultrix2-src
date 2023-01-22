/*
 *		@(#)pipes.h	1.1	(ULTRIX)	1/8/85
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
**  PIPES.H -- definitions for pipe blocks.
**
**	Version:
**		@(#)pipes.h	7.1	2/5/81
*/

# ifndef PB_DBSIZE

/*
**  The 'pb_t' (pipe block type) should be arranged so that the
**  size of the structure excluding pb_xptr is some nice power
**  of two.
*/

# define	PB_DBSIZE	116
# define	PB_IOSIZE	128

typedef struct _pb_t
{
	char	pb_st;		/* the state to enter */
	char	pb_proc;	/* the proc to enter */
	char	pb_resp;	/* the proc to respond to */
	char	pb_padxx;	/* --- unused at this time --- */
	char	pb_from;	/* the immediate writer of this block */
	char	pb_type;	/* the block type, see below */
	short	pb_stat;	/* a status word, see below */
	short	pb_nused;	/* the number of bytes used in this block */
	short	pb_nleft;	/* the number of bytes left in this block */
	char	pb_data[PB_DBSIZE];	/* the data area */
	char	*pb_xptr;	/* the data pointer (not written) */
}  pb_t;

/* possible values for pb_type */
# define	PB_NOTYPE	0	/* unknown type */
# define	PB_REG		1	/* regular block */
# define	PB_RESP		2	/* response block */
# define	PB_ERR		3	/* error message */
# define	PB_SYNC		4	/* interrupt sync */
# define	PB_RESET	5	/* system reset */
# define	PB_TRACE	6	/* set new trace flags */
/* more meta definitions go before this line */

/* definitions for pb_stat */
# define	PB_EOF		00001	/* end of file block */
# define	PB_FRFR		00002	/* originated from front end */
# define	PB_INFO		00004	/* info purposes only, no response */

/* definitions for pb_proc */
# define	PB_WILD		-2	/* all processes */
# define	PB_UNKNOWN	-1	/* unknown */
# define	PB_FRONT	0	/* front end */
/* other processes are given numbers from 2 */

/* definitions for pb_st */
/* define	PB_UNKNOWN	-1	*/
# define	PB_NONE		0	/* response block */

# endif PB_DBSIZE
