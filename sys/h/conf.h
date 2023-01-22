/*		@(#)conf.h	1.5		(ULTRIX)	7/23/86		*/
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
/************************************************************************
 *
 *			Modification History
 *
 *	23-Jul-86	Paul Shaughnessy -- prs
 *	Added the structure definition for genericconf.
 *
 *	11-Mar-86	Larry Palmer -- lp
 * 	Added strategy routine to cdevsw for n-buffered I/O. If
 *	the strategy routine is non-null this implies device can do
 *	multiple bufferring.
 *
 *	13-Sep-84	Stephen Reilly
 * 001- Add ioctl calls to the bdevsw
 *
 ***********************************************************************/
/*	conf.h	6.1	83/07/29	*/

/*
 * Declaration of block device
 * switch. Each entry (row) is
 * the only link between the
 * main unix code and the driver.
 * The initialization of the
 * device switches is in the
 * file conf.c.
 */
struct bdevsw
{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_strategy)();
	int	(*d_dump)();
	int	(*d_psize)();
	int	d_flags;
	int	(*d_ioctl)();				/* 001 */
};
#ifdef KERNEL
struct	bdevsw bdevsw[];
#endif

/*
 * Character device switch.
 */
struct cdevsw
{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_read)();
	int	(*d_write)();
	int	(*d_ioctl)();
	int	(*d_stop)();
	int	(*d_reset)();
	struct tty *d_ttys;
	int	(*d_select)();
	int	(*d_mmap)();
	int	(*d_strat)();
};
#ifdef KERNEL
struct	cdevsw cdevsw[];
#endif

/*
 * tty line control switch.
 */
struct linesw
{
	int	(*l_open)();
	int	(*l_close)();
	int	(*l_read)();
	int	(*l_write)();
	int	(*l_ioctl)();
	int	(*l_rint)();
	int	(*l_rend)();
	int	(*l_meta)();
	int	(*l_start)();
	int	(*l_modem)();
};
#ifdef KERNEL
struct	linesw linesw[];
#endif

/*
 * Swap device information
 */
struct swdevt
{
	dev_t	sw_dev;
	int	sw_freed;
	int	sw_nblks;
};
#ifdef KERNEL
struct	swdevt swdevt[];
#endif

/*
 * Generic configuration table
 */
struct genericconf
{
	caddr_t	gc_driver;
	char	*gc_name;
	dev_t	gc_root;
	int	gc_BTD_type;
};
#ifdef KERNEL
struct genericconf genericconf[];
#endif
