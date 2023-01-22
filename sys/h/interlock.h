/*	@(#)interlock.h	1.4	(ULTRIX)	6/20/86 	*/
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

/* ---------------------------------------------------------------------
 * Modification History 
 *
 * 05-Mar-86 -- bjg
 *	Added interlock for error logging
 *
 * 03-Mar-86 -- jrs
 *	Added interlock for cpu utilization table updating
 * 
 * 15 Jul 85 --jrs
 *	Created to hold resource lock related definitions
 * ---------------------------------------------------------------------
 */

/*
 * lock word bits
 */

#define	LOCK_RQ			31	/* run queue lock */
#define LOCK_TRACE		4	/* tracer lock */
#define LOCK_ERRLOG		3	/* error logging allocation lock */
#define	LOCK_ACTV		2	/* cpu active counter lock */
#define	LOCK_CPTIME		1	/* cpu utilization counter lock */
