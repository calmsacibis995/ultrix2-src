/* @(#)config.h	1.2	 (ULTRIX)	7/8/86 */

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
/*
 * Modification history:
 *
 * 7-jul-86   -- jaw 	added adapter alive bit for Mr. Installation.
 *
 * 5-Jun-86   -- jaw 	changes to config.
 *
 */

struct config_adpt {
	char 	*p_name;
	int	p_num;
	char	*c_name;
	int	c_num;
	char	c_type;		
	/* if c_type = 'D' then c_ptr is pointer to device struct */
	/* if c_type = 'C' then c_ptr is pointer to controller struct */
	/* if c_type = 'A' then c_ptr is set if ALIVE */
	char	*c_ptr;	   
};

#define CONFIG_ALIVE 	1  /* c_ptr is set if adapter is alive */
