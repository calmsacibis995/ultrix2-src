/*	invdist.h
 *		header info for handling inv(5) files.
 * SCCSID = "@(#)invdist.h	1.2   8/10/86"
 *
 *			Copyright (c) 1985 by
 *		Digital Equipment Corporation, Maynard, MA
 *			All rights reserved.
 *								
 *	This software is furnished under a license and may be used and
 *	copied  only  in accordance with the terms of such license and
 *	with the  inclusion  of  the  above  copyright  notice.   This
 *	software  or  any  other copies thereof may not be provided or
 *	otherwise made available to any other person.  No title to and
 *	ownership of the software is hereby transferred.		
 *								
 *	The information in this software is subject to change  without
 *	notice  and should not be construed as a commitment by Digital
 *	Equipment Corporation.					
 *								
 *	Digital assumes no responsibility for the use  or  reliability
 *	of its software on equipment which is not supplied by Digital.
 *
 *	MODIFICATION:
 *	000	ccb	1986.03.10
*/

#define	INV_NFLDS	12
#define	INV_FACLEN	32
#define	INV_DATLEN	12
#define	INV_REVLEN	8

typedef struct INV {
	char		i_fac[INV_FACLEN];	/* facility name */
	off_t 		i_size;			/* size of file in bytes */
	unsigned	i_sum;			/* checksum */
	short		i_uid;			/* user id */
	short		i_gid;			/* group id */
	u_short		i_mode;			/* file access mode */
	char		i_date[INV_DATLEN];	/* last rev date */
	char		i_rev[INV_REVLEN];	/* rev level */
	char		i_type;			/* file type in [lsdf] */
	char		i_path[MAXPATHLEN];	/* name */
	char		i_linkto[MAXPATHLEN];	/* path of file linked to */
	char		i_sub[INV_FACLEN];	/* subset for this file */
} INV;

#define	INVIFMT	"%s%d%u%hu%hu%ho%s%s%1s%s%s%s"
#define	INVOFMT	"%s%d%d%d%d%o%s%s%c%s%s%s"

