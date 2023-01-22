#ifndef lint
static	char	*sccsid = "@(#)readadmin.c	1.1	(ULTRIX)	1/8/85";
#endif lint

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

# include	<ingres.h>
# include	<access.h>
# include	<aux.h>
# include	<lock.h>


/*
**  READADMIN -- read admin file into 'Admin' cache
**
**	The admin file in the current directory is opened and read
**	into the 'Admin' cache.  The admin file contains the following
**	information:
**
**	A header block, containing the owner of the database (that is,
**	the DBA), and a set of status bits for the database as a whole.
**	These bits are defined in aux.h.  This header also includes a
**	field that defines the length of the header part & a version
**	stamp.
**
**	Descriptors for the relation and attribute relations.  These
**	descriptors should be completely correct except for the
**	relfp and relopn fields.  These are required so that the
**	process of opening a relation is not recursive.
**
**	After the admin file is read in, the relation and attribute
**	files are opened, and the relfp and relopn fields in both
**	descriptors are correctly initialized.  Both catalogs are
**	opened read/write.
**
**	WARNING:
**		This routine is redefined by creatdb.  If this
**		routine is changed, check that program also!!
**
**	Parameters:
**		none
**
**	Returns:
**		none
**
**	Side Effects:
**		The 'Admin' struct is filled in from the 'admin' file
**			in the current directory.
**		The 'relation....xx' and 'attribute...xx' files are
**			opened.
**
**	Files:
**		./admin
**			The bootstrap description of the database,
**			described above.
**
**	Trace Flags:
**		none
*/

readadmin()
{
	register int	i;
	char		relname[MAXNAME + 4];
	extern long	lseek();

	/* read the stuff from the admin file */
	i = open("admin", 0);
	if (i < 0)
		syserr("readadmin: open admin %d", i);
	checkadmin(i);
	close(i);

	/* open the physical files for 'relation' and 'attribute' */
	ingresname(Admin.adreld.reldum.relid, Admin.adreld.reldum.relowner, relname);
	if ((Admin.adreld.relfp = open(relname, 2)) < 0)
		syserr("readadmin: open rel %d", Admin.adreld.relfp);
	ingresname(Admin.adattd.reldum.relid, Admin.adattd.reldum.relowner, relname);
	if ((Admin.adattd.relfp = open(relname, 2)) < 0)
		syserr("readadmin: open att %d", Admin.adattd.relfp);
	Admin.adreld.relopn = (Admin.adreld.relfp + 1) * -5;
	/* we just want to read here create, modify and destroy fix it up */
	Admin.adattd.relopn = (Admin.adattd.relfp + 1) * 5;

	return;
}
