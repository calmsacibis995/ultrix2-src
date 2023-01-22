/*	@(#)reboot.h	1.4	Ultrix	8/10/86	*/

/************************************************************************
 *									*
 *			Copyright (c) 1984, 1986 by			*
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
 ************************************************************************
 *
 *  05 Aug 86 -- Fred Canter
 *	Added VAXstar restart/boot/halt codes.
 *
 * 12 Nov 84 -- rjl
 *	Added constants used to communicate boot options to the MicroVAX-II
 *	console program
 *
 *
 * Arguments to reboot system call.
 * These are passed to boot program in r11,
 * and on to init.
 */
#define	RB_AUTOBOOT	0	/* flags for system auto-booting itself */

#define	RB_ASKNAME	0x01	/* ask for file name to reboot from */
#define	RB_SINGLE	0x02	/* reboot to single user only */
#define	RB_NOSYNC	0x04	/* dont sync before reboot */
#define	RB_HALT		0x08	/* don't reboot, just halt */
#define	RB_INITNAME	0x10	/* name given for /etc/init */

#define	RB_PANIC	0	/* reboot due to panic */
#define	RB_BOOT		1	/* reboot due to boot() */

/*
 * Flags for MicroVAX-II console program communication
 */
#define RB_RESTART	0x21	/* Restart, english	*/
#define RB_REBOOT	0x22	/* Reboot, english	*/
#define RB_HALTMD	0x23	/* Halt, english	*/

/*
 * Flags for VAXstar console program communication.
 * NOTE: must be left shifted two bits.
 */
#define	RB_VS_RESTART	(0x1<<2)	/* restart, boot, halt */
#define	RB_VS_REBOOT	(0x2<<2)	/* boot, halt */
#define	RB_VS_HALTMD	(0x3<<2)	/* halt */
