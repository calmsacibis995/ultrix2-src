#ifndef lint
static char	*sccsid = "@(#)CONFIG.c	1.2	(ULTRIX)	1/24/86";
#endif lint

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

/************************************************************************
*
*			Modification History
*
*		David Metsky,	20-Jan-86
*
* 001	Replaced old version with BSD 4.3 version as part of upgrade
*
*	Based on:	CONFIG.c	5.1		6/4/85
*
*************************************************************************/

    /*
     *	make sure you are editing
     *		CONFIG.c
     *	editing config.c won't work
     */

    /*
     *	the version of translator
     */
char	*version = "VERSION (DATE)";

    /*
     *	the location of the error strings
     *	and the length of the path to it
     *	(in case of execution as a.something)
     */
char	*err_file = "LIBDIR/ERRORSTRINGS";
int	err_pathlen = sizeof("LIBDIR/")-1;

    /*
     *	the location of the short explanation
     *	and the length of the path to it
     *	the null at the end is so pix can change it to pi'x' from pi.
     */
char	*how_file = "LIBDIR/HOWFILE\0";
int	how_pathlen = sizeof("LIBDIR/")-1;
    
    /*
     *	things about the interpreter.
     *	these are not used by the compiler.
     */
#ifndef PC
char	*px_header = "LIBDIR/px_header";	/* px_header's name */
#endif

#ifndef PXP
char	*pi_comp = "INSTALLDIR/pi";		/* the compiler's name */
char	*px_intrp = "INSTALLDIR/px";		/* the interpreter's name */
char	*px_debug = "INSTALLDIR/pdx";		/* the debugger's name */
#endif
