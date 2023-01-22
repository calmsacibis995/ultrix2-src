/*
 *		@(#)limits.h	1.5	(ULTRIX)	10/21/85
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
 *			Modification History
 *
 *	David L Ballenger,  24-May-1984
 * 001	Increase size of SYS_NMLN to have more descriptive system name
 *	and a better chance at handling most node names.
 *
 * 002  Increase OPEN_MAX to 64 or NOFILE if defined
 *
 * 003	David L Ballenger, 30-Sep-1985
 *	Change ARG_MAX, PIPE_BUF, and PIPE_MAX to correspond to the 
 *	correct ULTRIX values.
 *
 * 004	David L Ballenger, 17-Oct-1985
 *	Remove #undef's at end of file so that #define's using those
 *	macros will work.
 *
 ***********************************************************************/

/*
	limits.h -- /usr/group Standard environmental limits
				(BRL UNIX System V emulation version)

	last edit:	85/02/24	D A Gwyn

	SCCS ID:	@(#)limits.h	1.1

	WARNING!  The ANSI C Standard is likely to change some of these,
	especially the floating-point definitions, which were botched.
*/

#ifndef	_LIMITS_H_
#define	_LIMITS_H_			/* once-only latch */

#if defined(pdp11)
#define	ARG_MAX		5120		/* max length of arguments to exec */
#else	/* 4.2BSD vax, gould */
#define	ARG_MAX		10240		/* max length of arguments to exec */
#endif
#if defined(gcos)
#define	CHAR_BIT	9		/* # of bits in a "char" */
#else
#define	CHAR_BIT	8		/* # of bits in a "char" */
#endif

/* Macros local to this file, used in later definitions: */
#define _BITS_( type )	(CHAR_BIT * (int)sizeof(type))
#define	_HIC_		((char)(1 << CHAR_BIT - 1))
#define	_HIS_		((short)(1 << _BITS_( short ) - 1))
#define	_HII_		(1 << _BITS_( int ) - 1)
#define	_HIL_		(1L << _BITS_( long ) - 1)

#if defined(pdp11) || defined(vax) /* gould ? */	/* "plain" char is signed */
#define	CHAR_MAX	 ((int)(char)~_HIC_)	/* max integer value of a "char" */
#define	CHAR_MIN	_HIC_		/* min integer value of a "char" */
#else					/* "plain" char is unsigned */
#define	CHAR_MAX	((char)-1)	/* max integer value of a "char" */
#define	CHAR_MIN	0		/* min integer value of a "char" */
#endif
#define	CHILD_MAX	25		/* max # of processes per user id */
#define	CLK_TCK		60		/* # of clock ticks per second */
#if defined(u3b) || defined(u3b5)
#define	DBL_DIG		15		/* digits of precision of a "double" */
#define	DBL_MAX	1.79769313486231470e+308	/* max decimal value of a "double" */
#endif
#if defined(pdp11) || defined(vax)
#define	DBL_DIG		16		/* digits of precision of a "double" */
#define	DBL_MAX	1.701411834604692293e+38	/* max decimal value of a "double" */
#endif
#if defined(u370) || defined(gould)
#define	DBL_DIG		16		/* digits of precision of a "double" */
#define	DBL_MAX	0.7237005577332262e+76	/* max decimal value of a "double" */
#endif
#if defined(gcos)
#define	DBL_DIG		18		/* digits of precision of a "double" */
#define	DBL_MAX	2.9387358770557187699e-39	/* max decimal value of a "double" */
#endif
#define	DBL_MIN		(-DBL_MAX)	/* min decimal value of a "double" */
#define	FCHR_MAX	1048576		/* max size of a file in bytes */
#if defined(u3b) || defined(u3b5)
#define	FLT_DIG		7		/* digits of precision of a "float" */
#define	FLT_MAX	3.40282346638528860e+38	/* max decimal value of a "float" */
#endif
#if defined(pdp11) || defined(vax)
#define	FLT_DIG		7		/* digits of precision of a "float" */
#define	FLT_MAX	1.701411733192644299e+38	/* max decimal value of a "float" */
#endif
#if defined(u370) || defined(gould)
#define	FLT_DIG		6		/* digits of precision of a "float" */
#define	FLT_MAX	0.7237005145e+76	/* max decimal value of a "float" */
#endif
#if defined(gcos)
#define	FLT_DIG		8		/* digits of precision of a "float" */
#define	FLT_MAX	1.7014118219281863150e+38	/* max decimal value of a "float" */
#endif
#define	FLT_MIN		(-FLT_MAX)	/* min decimal value of a "float" */
/* The following should be DBL_MAX but it isn't at present. */
#define	HUGE_VAL	((float)FLT_MAX)	/* error value returned by Math lib */
#define	INT_MAX		(~_HII_)	/* max decimal value of an "int" */
#define	INT_MIN		_HII_		/* min decimal value of an "int" */
#define	LOCK_MAX	0 /* for now */	/* max # of entries in system lock table */
/* 4.2BSD actually has no link limit; i.e. it is 65535 then a black hole results. */
#define	LINK_MAX	1000		/* max # of links to a single file */
#define	LONG_MAX	(~_HIL_)	/* max decimal value of a "long" */
#define	LONG_MIN	_HIL_		/* min decimal value of a "long" */
#if defined(pdp11)
#define	NAME_MAX	14		/* max # of characters in a file name */
#else	/* 4.2BSD vax, gould */
#define	NAME_MAX	255		/* max # of characters in a file name */
#endif
#ifdef NOFILE
#define	OPEN_MAX	NOFILE		/* max # of files a process can have open */
#else
#define	OPEN_MAX	64		/* max # of files a process can have open */
#endif
#define	PASS_MAX	8		/* max # of characters in a password */
#if defined(pdp11)
#define	PATH_MAX	256		/* max # of characters in a path name */
#else	/* 4.2BSD vax, gould */
#define	PATH_MAX	1024		/* max # of characters in a path name */
#endif
#define	PID_MAX		30000		/* max value for a process ID */
/* 4.2BSD appear to have no natural pipe size. */
#define	PIPE_BUF	4096		/* max # bytes atomic in write to pipe */
#define	PIPE_MAX	4096		/* max # bytes written to a pipe in a write */
/* The following is pure invention. */
#define	PROC_MAX	100		/* max # of simultaneous processes */
#define	SHRT_MAX	((int)(short)~_HIS_)	/* max decimal value of a "short" */
#define	SHRT_MIN	_HIS_		/* min decimal value of a "short" */
#if defined(u3b) || defined(u3b5)
#define	STD_BLK		1024		/* # bytes in a physical I/O block */
#endif
#if defined(pdp11)
#define	STD_BLK		512		/* # bytes in a physical I/O block */
#endif
#if defined(vax) || defined(gould)	/* 4.2BSD; your mileage may vary */
#define	STD_BLK		8192		/* # bytes in a physical I/O block */
#endif
#if defined(u370)
#define	STD_BLK		4096		/* # bytes in a physical I/O block */
#endif
#define	SYS_NMLN	32		/* # of chars in uname-returned strings */
/* The following is pure invention. */
#define	SYS_OPEN	200		/* max # of files open on system */
#define	TMP_MAX		17576		/* max # of calls to tmpnam(3S) before */
					/* recycling of names occurs */
#define	UID_MAX		60000		/* max value for a user or group ID */
#define	USI_MAX		((unsigned)-1)	/* max decimal value of an "unsigned" */
#define	WORD_BIT	_BITS_( int )	/* # of bits in a "word" or "int" */


#endif	_LIMITS_H_
