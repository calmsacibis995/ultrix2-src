#ifndef lint
static char	*sccsid = "@(#)treen.c	1.2	(ULTRIX)	1/27/86";
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
*	Based on:	treen.c		5.1		6/5/85
*
*************************************************************************/

    /*
     *	is there some reason why these aren't #defined?
     */

#include	"0.h"
#include	"tree_ty.h"

struct tnode *
tree1 ( arg1 )
    int		arg1;
    {
	return( tree ( 1 , arg1 ));
    }

struct tnode *
tree2 ( arg1 , arg2 )
    int		arg1 , arg2;
    {
	return( tree ( 2 , arg1 , arg2 ));
    }

struct tnode *
tree3 ( arg1 , arg2 , arg3 )
    int		arg1 , arg2 ;
    struct	tnode *arg3;
    {
	return( tree ( 3 , arg1 , arg2 , arg3 ));
    }

struct tnode *
tree4 ( arg1 , arg2 , arg3 , arg4 )
    int		arg1 , arg2 ;
    struct tnode *arg3 , *arg4;
    {
	return( tree ( 4 , arg1 , arg2 , arg3 , arg4 ));
    }

struct tnode *
tree5 ( arg1 , arg2 , arg3 , arg4 , arg5 )
    int		arg1 , arg2 ;
    struct tnode *arg3 , *arg4 , *arg5;
    {
	return (tree ( 5 , arg1 , arg2 , arg3 , arg4 , arg5 ));
    }

