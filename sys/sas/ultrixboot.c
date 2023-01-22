/*
 * ultrixboot.c
 */

#ifndef lint
static	char	*sccsid = "@(#)ultrixboot.c	1.4  (ULTRIX)        3/12/86";
#endif lint
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
#include "vmb.h"
#include "../h/param.h"
#include <a.out.h>
#include "../h/reboot.h"
#include "../vax/cpu.h"
#include "../vax/mtpr.h"
#include "../vax/rpb.h"

char    ds730[] = "(x)field/ensaa.exe";
char    ds750[] = "(x)field/ecsaa.exe";
char    ds780[] = "(x)field/essaa.exe";
char    ds8200[] = "(x)field/ebsaa.exe";
char    ds8600[] = "(x)field/edsaa.exe";
char    vmunix_name[] = "(a)vmunix";
char    ask_name[INBUFSZ];
char    pcs_ucode[] = "(a)pcs750.bin";

#define RB_LOADDS RB_INITNAME			/* defined as 0x10 */

char	ucodedone=0;				/* mark one time ucode
						   load for 11/750 (successful
						   or unsuccessful) */
char   *imagename;
extern	int      mode, cpu;
extern	struct	vmb_info *vmbinfo;

int	debug=0;

/*
 * Functional Discription:
 *	main of `ultrixboot' program ... 
 *
 * Inputs:
 *	none although R10 and R11 are preserved
 *
 * Outputs:
 *	none
 *
 */
main () {
	register howto, devtype;		/* howto=r11, devtype=r10 */
	union cpusid sid; 
	int	io;
	char	fs;

	printf("\nUltrixboot (using VMB version %d)\n\n", vmbinfo->vmbvers);
	/*
	 * If the amount of memory found by VMB exceeds the amount of
	 * contiguous memory sized for Ultrix by 16 pages, warn the
	 * user, otherwise let it slide.  Rememeber, MVAX II reserves two
	 * pages at the top for the console.  Also, leave some slop for
	 * the way the bit map is counted (by byte rather than by bit).
	 * This warning is intended to find large holes in memory and
	 * and answer the question 'Why is Ultrix only seeing x of the y
	 * Meg of memory?'
	 */
	if (((((struct rpb *)(vmbinfo->rpbbas))->pfncnt) - vmbinfo->memsiz/512) > 16) 
		printf("\nWARNING:\n\tA total of %d bytes of memory where found,\n\tof which %d are contiguous.  Ultrix will\n\tonly use the contiguous memory.\n\n",
		(((struct rpb *)(vmbinfo->rpbbas))->pfncnt)*512, 
		vmbinfo->memsiz);
	fs = ((howto & 0x70000000) >> 28) + 'a';
	io = -1;				/* set to start loop */
	while (io < 0) {
	/* 
	 * If we are loading the supervisor, imagename is
	 * already set.  If we are not, then check for RB_ASKNAME.
	 * If RB_ASKNAME is not set, then assume the default 
	 * 'vmunix' name, otherwise ask for a unix file name to
	 * load.  Entering a diagnostic supervisor name will not
	 * work as it is not a unix image.
	 */

		if (howto & RB_ASKNAME) {
			printf ("Enter %s name: ",
				howto & RB_LOADDS ? "supervisor" : "image");
			gets (ask_name);
			if (ask_name[0] == 0)
				continue;
			imagename = ask_name;
		} else
			if (howto & RB_LOADDS) {/* Are we loading the DS? */
				switch (cpu) {	/* Which cpu is this? */
				case VAX_730:	/* point to 'ensaa' */
					imagename = ds730;
					break;
				case VAX_750: 	/* point to 'ecsaa' */
					imagename = ds750;
					break;
				case VAX_780: 	/* point to 'essaa' */
					imagename = ds780;
					break;
				case VAX_8200:	/* point to 'ebsaa' */
					imagename = ds8200;
					break;
				case VAX_8600:	/* point to 'edsaa' */
					imagename = ds8600;
					break;
				default: 
				/* 
				 * Only the above CPU's are supported.
				 * Remember that 11/780 = 11/785
				 *           	 11/730 = 11/725
				 *		 8600	= 8650
				 */
					printf ("No DS load support for cpu type %d\n", cpu);
					stop();
				}
			} else
				imagename = vmunix_name;

		if ((howto & RB_ASKNAME) == 0) {
			imagename[1] = fs;
		}
		/*
		 * Try only once to load the pcs ucode for a 11/750 if
		 * the rev level indicates rev 95.  If it is greater
		 * than 95 then don't attempt a redundant load.
		 */
		if ((cpu == VAX_750) && (!ucodedone)) {	
			sid.cpusid = mfpr(SID);	/* read to see any changes */
			if (sid.cpu750.cp_urev == 95) {
				printf ("Updating 11/750 microcode ...\n");
				loadpcs();	
				ucodedone++;
			}
		}
		if ((io = open (imagename, 0)) < 0) {
			printf ("can't open %s\n", imagename);
			howto |= RB_ASKNAME;
			continue;
		}
		printf ("Loading %s ...\n", imagename);
		/* 
	 	 * If we are loading the supervisor, call load_ds which
	 	 * is a special load routine, just for the supervisor.
	 	 * copyunix will not load the diagnostic supervisor.
	 	 */
		if (howto & RB_LOADDS) {
			if (load_ds(io))
				start_ds();
		} else
			copyunix (howto, devtype, io);
		close (io);
		io = -1;
	}
}

/*
 * Functional Discription:
 *	This routine reads in a loads an ultrix image, usually vmunix.
 *
 * Inputs:
 *	R11,R10 and the io channel
 *
 * Outputs:
 *	-1 returned on error
 *	otherwise, the image is called.
 *
 */
copyunix (howto, devtype, io)
	register howto, devtype, io;		/* howto=r11, devtype=r10 */
{
	struct exec     x;
	register int    i;
	char   *addr;

	i = read (io, (char *) & x, sizeof x);
	if (i != sizeof x ||
		(x.a_magic != 0407 && x.a_magic != 0413 && x.a_magic != 0410)){
		printf ("Bad a.out format\n");
		return (-1);
	}
	printf ("\nSizes:\ntext = %d\n", x.a_text);
	if (x.a_magic == 0413 && lseek (io, 0x400, 0) == -1)
		goto shread;
	if (read (io, (char *) 0, x.a_text) != x.a_text)
		goto shread;
	addr = (char *) x.a_text;
	if (x.a_magic == 0413 || x.a_magic == 0410)
		while ((int) addr & CLOFSET)
			*addr++ = 0;
	printf ("data = %d\n", x.a_data);
	if (read (io, addr, x.a_data) != x.a_data)
		goto shread;
	addr += x.a_data;
	printf ("bss  = %d\n", x.a_bss);
	x.a_bss += 128 * 512;	/* slop */
	for (i = 0; i < x.a_bss; i++)
		*addr++ = 0;
	x.a_entry &= 0x7fffffff;
	printf ("Starting at 0x%x\n\n", x.a_entry);
	(*((int (*) ()) x.a_entry)) (vmbinfo);
	stop();
shread: 
	printf ("Short read\n");
	return (-1);
}

/*
 * Functional Discription:
 *	This routine loads the diagnostic supervisor at address 0xfe00.
 *	it always assumes good memory.
 *
 * Inputs:
 *	io channel
 *
 * Outputs:
 *	0 = error loading
 *	1 = good load
 *
 */
load_ds (io)
	int     io;
{
	int     cnt, i;
	char	*addr = (char *) 0xfe00;

	for (i = 0;; i += 10240) {
		if ((cnt = read (io, addr + i, 10240)) < 0)
			return(0);		/* bad load */
		if (!cnt)
			break;
	}
	printf ("\n");
	return(1);
}

/*
 * Functional Discription:
 *	This routine is called only once if ucode is out of date on an
 *	11/750.  It returns nothing since is only called once.
 *
 * Inputs:
 *	none
 *
 * Outputs:
 *	none
 *
 */
loadpcs () {
	int     pcs_io;
	int     cnt;

	/*
	 * If the open of pcs750.bin fails it will tell us and we will want
	 * to return.
	 */
	pcs_io = open (pcs_ucode, 0);
	if (pcs_io < 0)
		return;
	/*
	 * Read in the file starting at memory location 0
	 * We read a little more than the actual file size.
	 * This way we can check to make sure we read exactly
	 * the number of bytes we expected to.
	 */
	cnt = read (pcs_io, (char *) 0, 23 * 512);
	/*
	 * Make sure the read was successful. If not, report it and return.
	 * Allow Unix to boot anyway
	 */
	if (cnt != 22 * 512)
		printf ("Load pcs750 failed: Error reading %s\n", pcs_ucode);
	else {
		pcsloadpatch ();			/* Load patchbits */
		pcsloadpcs ();				/* Load pcs code */
	}
	close(pcs_io);
	return;
}
