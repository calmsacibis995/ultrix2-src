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

#ifndef lint
static	char	*sccsid = "@(#)tpcopy-boot.c	1.2	(ULTRIX)	3/23/86";
#endif lint

/* ------------------------------------------------------------------------
 * Modification History:
 *
 * ------------------------------------------------------------------------
 */

#include "../h/param.h"
#include "../h/gnode.h"
#include <a.out.h>

#include "saio.h"


/*
 * Copy 'from' 'to' in 10K units.
 * Followed by booting vmunix in the image that was placed at 'to'.
 * Intended for use with TK50 MicroVAX II system installation.
 * For ease-of-use it defaults the 'from'[tms(0,1)] and 'to'[ra(0,1)] devices.
 */

main()
{
	int from, to;
	char fbuf[50], tbuf[50];
	char buffer[10240];
	register int record;
	extern int errno;
	int error = 0;

	from = getdev("From [tms(0,1)] ", "tms(0,1)", fbuf, 0);
	to = getdev("To [ra(0,1)] ", "ra(0,1)", tbuf, 1);
	for (record = 0; ; record++) {
		int rcc, wcc;

		rcc = read(from, buffer, sizeof (buffer));
		if (rcc == 0)
			break;
		if (rcc < 0) {
			printf("Record %d: read error, errno=%d\n",
				record, errno);
			error = 1;
			break;
		}
		if (rcc < sizeof (buffer))
			printf("Record %d: read short; expected %d, got %d\n",
				record, sizeof (buffer), rcc);
		/*
		 * For bug in ht driver.
		 */
		if (rcc > sizeof (buffer))
			rcc = sizeof (buffer);

		wcc = write(to, buffer, rcc);
		if (wcc < 0) {
			printf("Record %d: write error: errno=%d\n",
				record, errno);
			error = 1;
			break;
		}
		if (wcc < rcc) {
			printf("Record %d: write short; expected %d, got %d\n",
				record, rcc, wcc);
			error = 1;
			break;
		}
	}
	if (error == 0) {
		printf("Copy completed: %d records copied\n", record);
		close(from);
		close(to);
		strcat(tbuf, "vmunix");
		to = open(tbuf, 0);
		if (to >= 0) {
			printf("\nBooting Mini-Root...\n");
			copyunix(to);
			/* No Return, copyunix gets an image running or dies */
		}
		else {
			printf("Open of boot device failed. Can't boot it.\n");
		}
	}
	/* can't call exit here */
}

/*
 * Get user's choice of device and open it:
 *	for input from tape its device(unit, file)
 *	for output to disk its device(unit, partition)
 * The default is shown in the prompt, if user hits return only then the
 *	default is used.
 */

getdev(prompt, defdev, buf, mode)
	char *prompt,			/* prompt string, including default */
	     *defdev,			/* default device */
	     *buf;			/* read user input into */
	int mode;			/* file open mode */
{
	register int i;

	do {
		printf("%s: ", prompt);
		gets(buf);
		if (!strcmp(buf,""))
			strcpy(buf, defdev);
		i = open(buf, mode);
	} while (i <= 0);
	return (i);
}


/*
 * Copy an image from the device referenced by file desc parameter 'io',
 * into memory and start it running.
 * This is essentially the same as copyunix from boot.c
 */

copyunix(io)
	int	 io;			/* file desc for the tape */
{
	struct exec x;
	register int    i;
	char   *addr;

	i = read(io, (char *)&x, sizeof x);
	if (i != sizeof x ||
	    (x.a_magic != 0407 && x.a_magic != 0413 && x.a_magic != 0410))
		_stop("Bad format\n");
	printf("%d", x.a_text);
	if (x.a_magic == 0413 && lseek(io, 0x400, 0) == -1)
		goto shread;
	if (read(io, (char *)0, x.a_text) != x.a_text)
		goto shread;
	addr = (char *)x.a_text;
	if (x.a_magic == 0413 || x.a_magic == 0410)
		while ((int)addr & CLOFSET)
			*addr++ = 0;
	printf("+%d", x.a_data);
	if (read(io, addr, x.a_data) != x.a_data)
		goto shread;
	addr += x.a_data;
	printf("+%d", x.a_bss);
	x.a_bss += 128*512;	/* slop */
	for (i = 0; i < x.a_bss; i++)
		*addr++ = 0;
	x.a_entry &= 0x7fffffff;
	printf(" start 0x%x\n", x.a_entry);
	(*((int (*)()) x.a_entry))();
	exit();
shread:
	_stop("Short read\n");
}
