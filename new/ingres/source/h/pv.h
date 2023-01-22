/*
 *		@(#)pv.h	1.1	(ULTRIX)	1/8/85
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
**  PV.H -- definitions for parameter vectors
**
**	Version:
**		@(#)pv.h	7.1	2/5/81
*/

# ifndef PV_MAXPC


/* setable constants */
# define	PV_MAXPC	125	/* maximum number of parameters */

/* the parameter vector type */
typedef struct
{
	short	pv_type;	/* the type, see below */
	short	pv_len;		/* the length of the value */
	union
	{
		short			pv_int;		/* PV_INT */
		struct querytree	*pv_qtree;	/* PV_QTREE */
		char			*pv_str;	/* PV_STR */
		char			*pv_tuple;	/* PV_TUPLE */
	} pv_val;
}  PARM;

/* pv_type values */
# define	PV_EOF		0	/* end of list */
# define	PV_INT		1	/* integer */
# define	PV_STR		2	/* string */
# define	PV_QTREE	3	/* query tree */
# define	PV_TUPLE	4	/* tuple */


# endif PV_MAXPC
