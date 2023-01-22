/*
 *		@(#)version.h	1.1	(ULTRIX)	1/8/85
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
**  VERSION.H -- system version definition file.
**
**	NOTICE:
**		Version numbers stored in files are SCCS id's
**		and may not correspond to the external distribution
**		version number.  The distribution number applies to
**		the entire system and not to any particular file.
**		This file defines a "release" number, used for
**		creating file names.  The entire system version
**		number (including mod number) is defined by
**		conf/version.c.
**
**	Version:
**		@(#)version.h	7.1	2/5/81
*/


/*
**	VERSION is the version number of this incarnation of INGRES
**		for purposes of creating file names.
**	DBVERCODE is the code for this database version stored in
**		the admin file.
**	PATHEXT is an extension for the path as derived from the
**		"ingres" entry in the password file to determine
**		the "Pathname" variable.  If it is not defined,
**		no extension is made.
*/

# define	VERSION		"7"		/* version number */
# define	DBVERCODE	1		/* database version code */
/* # define	PATHEXT		"/x"		/* the root path extension */
