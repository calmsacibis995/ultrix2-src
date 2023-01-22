/*
 *		@(#)assert.h	1.3	(ULTRIX)	4/16/85
 */

/************************************************************************
 *									*
 *			Copyright (c) 1985 by				*
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
 *	David L Ballenger, 29-Mar-1985					*
 * 0001	Add defintions for System V compatibility.			*
 *									*
 ************************************************************************/

/*	assert.h	4.1	83/05/03	*/

#ifndef SYSTEM_FIVE
/*
 * ULTRIX definitions
 */
#ifndef NDEBUG
#define _assert(ex) \
	{if (!(ex)) {\
		fprintf(stderr,"Assertion failed: file %s, line %d\n",\
			__FILE__, __LINE__);\
		exit(1);\
	}}
#else NDEBUG
#define _assert(ex) ;
#endif NDEBUG
#define assert(ex) _assert(ex)

#else SYSTEM_FIVE
/*
 * System V definitions of assert.
 */
#ifndef NDEBUG
extern void _assert();
#define assert(EX) if (EX) ; else _assert("EX", __FILE__, __LINE__)
#else NDEBUG
#define assert(EX)
#endif NDEBUG
#endif SYSTEM_FIVE
