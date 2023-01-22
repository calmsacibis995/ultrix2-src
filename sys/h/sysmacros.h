/*
 *		@(#)sysmacros.h	1.1	(ULTRIX)	4/16/85
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
 *	Create ULTRIX version of <sys/sysmacros.h> from System V	*
 *									*
 ************************************************************************/

/* Most of the definitions in the System V version of these files are 
 * defined in other places in ULTRIX.  Include those files to make sure
 * we are compatible.
 */

/* Macros dealing with click conversions come from <sys/param.h>.
 * Include them if they haven't already been defined.
 */
#ifndef ctos
#include <sys/param.h>
#endif

/* Macros dealing with disk address and offsets come from <sys/fs.h>
 * Include them if they haven't already been defined.
 */
#ifndef itod
#include <sys/fs.h>
#endif

/* Macros dealing with device numbers come from <sys/types.h>
 * Include them if they haven't already been defined.
 */
#ifndef major
#include <sys/types.h>
#endif

/* System V extra device number macros.
 */
#define bmajor(x)	major(x)
#define brdev(x)	((x)&0x1fff)
