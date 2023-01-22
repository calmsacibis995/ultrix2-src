/*
 *		@(#)decomp.h	1.1	(ULTRIX)	1/8/85
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
**	This header file contains all the defined constant
**	and special structures used by decomposition. Certain
**	global variables which are referenced by many modules
**	are also included. By convention global names always
**	begin with a capital letter.
**
**	Version:
**		@(#)decomp.h	7.2	3/5/81
*/


# include	<pv.h>


   
# define OVHDP		2		/*  overhead for a projection  */
# define OVHDM		10		/*  overhead for a modify  */

# define MAXRELN	6		/* size of relation descriptor cache */
  
# define QBUFSIZ	2000		/* buffer size (bytes) of original query tree */
# define SQSIZ		10000		/* buffer size for tree copies + sub-queries */
# define AGBUFSIZ	350		/* buffer size for temp agg tree components */
# define PBUFSIZE	500		/* size of parameter buffer area for setp() */
# define PARGSIZE	PV_MAXPC	/* max number of arguments for setp() */

/* error messages */
# define NODESCAG	4602	/* no descriptor for aggr func */
# define QBUFFULL	4610	/* Initial query buffer overflow */
# define SQBUFFULL	4612	/* sub-query buffer overflow */
# define STACKFULL	4613	/* trbuild stack overflow */
# define AGBUFFULL	4614	/* agg buffer overflow */
# define AGFTOBIG	4615	/* agg function exceeds MAXTUP or MAXDOM */
# define TOOMANYAGGS	4616	/* more than MAXAGG aggregates */
# define RETUTOBIG	4620	/* retr unique target list exceeds MAXTUP */

/* symbolic values for GETNXT parameter of fcn GET */
# define NXTTUP	1	/* get next tuple after one specified by tid */

/* flag for no result relation */
# define	NORESULT	-1

/* Range table slot which is always free for aggregate temp rels */
# define	FREEVAR		MAXRANGE	/* free var number for aggs */

/* Range table slot which is used for secondary index */
# define	SECINDVAR	MAXRANGE + 1



# define	FIRSTNUM	MAXRANGE + 3
# define	LASTNUM		100
