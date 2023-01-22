/************************************************************************
 *									*
 *			Copyright (c) 1985, 1986 by			*
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
static	char	*sccsid = "@(#)boot.c	1.17	(ULTRIX)	3/23/86";
#endif lint

/* ------------------------------------------------------------------------
 * Modification History:
 *
 * 09-Oct-85 -- jaw
 *	diagnostic supervisor support for 8200.
 *
 * 19-Sep-85 -- jaw
 *	fix so boot_justask works right for 8200 floppy-BDA boot.
 *
 * 15-Jul-85 -- map
 *	Fix definition of saveCSR - make it extern
 *
 * 18-Jun-85 -- jaw
 *  	make it so rx50 boot can switch to ra to get vmunix....
 *
 * 18-Jun-85 -- jaw
 *  	fixed it so rabads would load....saveCSR defined wrong
 *	
 * 17-Jun-85 -- jaw
 *	add support for console floppy on VAX8200
 *
 * 13-Apr-85 --map
 *	added saveCSR - used for saving boot device CSR
 *
 * 28-Mar-85 --tresvik
 *	added support of the 8600 diag supers
 *
 * 29-Nov-84 --tresvik
 *	changed call to _exit to a call to exit.  exit is defined locally
 *
 * 21 Oct 84 --tresvik
 *
 *	moved the partition selection to the bits 28-31 of the boot flag
 *	word (R11).  This is so that 11/750 partition selection can be
 *	accomplished.  (B/60000010) to get at the DS in the `g' filesystem.
 *
 * 24 Sep 84 --tresvik
 *
 *	image specifier strings are now developed using a local sprintf
 *	diagnostic supervisor boot is from /usr/field.
 *
 * 20 Mar 84 --tresvik
 *
 *	Beginnings of Diagnostic Supervisor load support.  Requires -DDS
 *	option to compiler to include new code.  Not yet included in the
 *	makefile.
 * 
 * 10 Jan 84 --tresvik
 *	Added support for loading 750 microcode on Rev 5 VAX 750s.
 *	There are some spurious changes from emacs reformatting, too.
 *
 * 16 Feb 84 -- rjl
 *	Added support for passing boot device through to kernel as dev_t.
 *
 *  9 Dec 83 --jmcg
 *	Added support for boot from non-zero unit.  The unit number is
 *	conveyed in as the second byte of the device type (r10).
 *
 *  9 Dec 83 --jmcg
 *	Derived from 4.2BSD, labeled:
 *		boot.c	6.1	83/07/29
 *
 * ------------------------------------------------------------------------
 */

#include "../h/param.h"
#include "../h/gnode_common.h"
#include "../ufs/ufs_inode.h"
#include "../h/gnode.h"
#include "../h/vm.h"
#include "../vax/mtpr.h"
#include "../vax/cpu.h"
#include <a.out.h>

#include "../machine/pte.h"
#include "../vax/nexus.h"
#include "../vaxuba/ubareg.h"
#include "../vaxmba/mbareg.h"

#include "saio.h"
#include "savax.h"
#include "../h/reboot.h"

#ifdef DS
#define RB_LOADDS RB_INITNAME		/* defined as 0x10 */ 
#endif DS

/*
 * Boot program... arguments passed in r10 and r11 determine
 * whether boot stops to ask for system name and which device
 * boot comes from.
 * R9 contains boot device CSR.
 */

/* Types in r10 specifying major device */
#define MAX_DEV	17
/* The following defines are used when the boot device is a 8200 console rx50 */
/* and the desired boot device is an ra type. The defines must be kept the */
/* same as the device types in the following table. */

#define cx_type 16
#define ra_type 9
char    devname[][2] = {
	'h', 'p',				/* 0 = hp */
	0, 0,					/* 1 = ht */
	'u', 'p',				/* 2 = up */
	'h', 'k',				/* 3 = hk */
	0, 0,					/* 4 = sw */
	0, 0,					/* 5 = tm */
	0, 0,					/* 6 = ts */
	0, 0,					/* 7 = mt */
	0, 0,					/* 8 = tu */
	'r', 'a',				/* 9 = ra */
	'u', 't',				/* 10 = ut */
	'r', 'b',				/* 11 = rb */
	0, 0,					/* 12 = uu */
	0, 0,					/* 13 = rx */
	'r', 'l',				/* 14 = rl */
	0, 0,					/* 15 = ?? */
	'c', 's',				/* 16 = cs */
	0, 0,					/* 17= ?? */
};

#ifdef DS
char	*tmp, line[40];
/*
 * Diagnostic Supervisors will live in /usr/field so it will be
 * necessary to specify the /usr disk partition when booting.
 */
char	ds730[] = "field/ensaa.exe";
char	ds750[] = "field/ecsaa.exe";
char	ds780[] = "field/essaa.exe";
char	ds8200[] = "field/ebsaa.exe";
char	ds8600[] = "field/edsaa.exe";
char    unix_name[] = "vmunix";
#else DS
char    line[40] = "vmunix";
#endif DS
char	pcs[40];
char    pcs_name[] = "pcs750.bin";

extern long saveCSR;				/* used to save boot CSR */
extern int nuba;
extern char can_bda;
char 	cx_yes=0;
int     retry = 0;
int     load_microcode = 0;			/*
						 * flag for use with
						 * 11/750 Patchable
						 * Control Store
						 */

main()
{
	register howto, devtype, bootcsr;	/* howto=r11, devtype=r10, bootcsr=r9 */
	int io, bootdevice, partition, console_flag,i;
	unsigned	unit;
	union cpusid	sid;

#ifdef lint
	howto = 0; devtype = 0; bootcsr = 0;
#endif lint

	saveCSR = bootcsr;			/* salt away boot device CSR for later use */

/* Grab the System Id reg.  Determine if it is a VAX 11/750 and what it's
 * current microcode level is.  If the level is 95 (decimal) or above then we
 * will attempt to load the patchable control store from pcs750.bin which
 * should reside in the root directory of the device being booted.  Rev level
 * >= 95 indicates that the L0008 module is installed. ( <= 94 is the older
 * ROM style L0005 which cannot be loaded. )  If pcs750.bin cannot be opened,
 * Unix is allowed to boot, however it will be running with the current level
 * of microcode which one can assume will be the powerup level of 95 which is
 * functionally the same as 94.
 */

	sid.cpusid = mfpr(SID);			/* Get System ID */
	if (sid.cpuany.cp_type == VAX_750)	/* Is this a 750? */
		if (sid.cpu750.cp_urev >= 95)	/* Yes, is microcode >= 95? */
			load_microcode = 1;	/* Yes, set the load flag */
#ifdef DS 
	tmp = unix_name;			/* Point to the unix name */
	if (howto&RB_LOADDS)			/* Are we loading the DS */
		switch (sid.cpuany.cp_type) 	/* What kind of cpu is it */
			{
			case VAX_730:		/* point to 'ensaa' */
				tmp=ds730;
				break;
			case VAX_750:		/* point to 'ecsaa' */
				tmp=ds750;
				break;
			case VAX_780:		/* point to 'essaa' */
				tmp=ds780;
				break;
			case VAX_8200:		/* point to 'ebsaa' */
				tmp=ds8200;
				break;
			case VAX_8600:		/* point to 'edsaa' */
				tmp=ds8600;
				break;
			default:
				_stop("No DS boot support for this processor type\n");
				break;
			}
#endif DS 
	printf ("\nBoot\n");
#ifdef JUSTASK
	howto = RB_ASKNAME | RB_SINGLE;
#endif JUSTASK
	/*
	 * Second byte of devtype (R10) contains the unit number
	 */
	unit = (devtype & 0xff00) >> 8;/* second byte is unit */
	/*
	 * Bits 28-31 of howto (R11) contains the partion number.
	 * Placing the partition here allows 750 and MVAX to work.
	 */
	partition = (howto & 0xf0000000) >> 28; 
	console_flag = devtype & 0x80000000;/* keep console type flag */
	devtype &= 0377;	/* retain major device type */

	/* The following code checks if we are booting from a 8200
	 * console rx_50 and if we are sets cx_yes to 1
	 */
	if ((sid.cpuany.cp_type == VAX_8200) && (devtype == cx_type)) 
		cx_yes = 1;

#ifndef JUSTASK
	if ((howto & RB_ASKNAME) == 0) {
		/*
		 * Build a name based on the device type and unit number if they
		 * are sane, otherwise just ask.
		 */
		if( devtype >= 0 
		    && devtype < sizeof(devname)/2
		    && devname[devtype][0] ) {
			/*
			 * Build the string from devname, unit, minor, image
			 */
			sprintf(line, "%c%c(%d,%d)%s",
			    devname[devtype][0],
			    devname[devtype][1],
			    unit, partition, tmp);
		} else
			howto = RB_SINGLE | RB_ASKNAME;
	}
#endif JUSTASK
	for (;;) {
		if (howto & RB_ASKNAME) {
			printf(": ");
			gets(line);
		} else
			printf(": %s\n", line);

		if (cx_yes && (line[0] == 'r' && line[1] == 'a')) { /* we have to figure out which ra controller to use */

			for (i = 0; i < nuba; i++)
				{
				if ((((struct bi_regs *)ubaddr[i])->bi_typ & BITYP_TYPE) == BI_BDA) {
					saveCSR = (long)ubaddr[i];
					break;
					}
				}
			if (i == nuba) can_bda = 0; /* Use the uda */
		}

		io = open(line, 0);
		if (io >= 0) {
			unit = atoi(index(line, '(') + 1);
			partition = atoi(index(line, ',') + 1);
			/*
		  	 * If the specified Unix device and file spec 
			 * could be opened then let's go after the Ucode,
			 * if necessary, We don't want to attempt to 
			 * locate pcs750.bin on a bad device spec.
			 */
			if (load_microcode) {
				/*
				 * Are we updating 750 microcode?
				 * Build the pcs750.bin file spec
				 */
				printf ("Updating 11/750 microcode ...\n");
				/*
				 * Build the string from devname, 
				 * unit, minor, image
				 */
				sprintf(pcs, "%c%c(%d,%d)%s",
			    		line[0], line[1],
					unit, partition, pcs_name);
				loadpcs ();	/* Go load the microcode */
				/*
				 * Let the Ucode load
				 * happen only once in
				 * case we end up looping
				 * on a bad unix load.
				 * Clear the flag.
				 */
				load_microcode = 0;
			}
			/*
			 * Build a bootdevice based on the open string
			 */
			for( devtype = 0 ; devtype < MAX_DEV ; devtype++)
				if( devname[devtype][0] == line[0] && devname[devtype][1] == line[1] )
					break; 
#ifdef DS 
			if (howto&RB_LOADDS) {
				load_ds(io, howto, unit, devtype);
				/*
				 * if all goes well we won't get back to
				 * here.
				 */
				_stop("DS load failed\n");
			}
			else 
#endif DS 
			if( devtype < MAX_DEV && unit >= 0 && unit < 256 && partition >= 0 && partition <= 7 ) {
				bootdevice = (devtype<<8) | (unit*8 + partition) | console_flag;
				copyunix(howto, bootdevice, bootcsr, io);
			}
		}
		if (++retry > 2)
#ifdef DS
			if (howto&RB_LOADDS)
				_stop("DS boot failed\n");
			else
#endif DS
				howto = RB_SINGLE | RB_ASKNAME;
	}
}

/*ARGSUSED*/
copyunix(howto, bootdevice, bootcsr, io)
	register howto, bootdevice, bootcsr; 	/* r11=howto r10=bootdevice r9=bootcsr */
	int	 io;
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

/*
 * "loadpcs"  is  entered  only if the CPU is a 11/750 (type 2) and the
 * microcode  level  of  the  machine  is 95 or greater indicating that the
 * L0008  Patchable  Control  Store module is installed in the machine. The
 * character  string "pcs" is used to locate the PCS microcode update file.
 * The  file  must  be  located  in  the  root directory of the boot device
 * specified  or  defaulted to. If the file cannot be located a messages is
 * printed and Unix continues through a normal boot. If the file is located
 * and  successfully loaded the updated microcode level is displayed on the
 * console.  This will be helpful to easily identify the microcode level of
 * the running system.
 */

loadpcs () {
	int     pcs_io;
	int     cnt;

	/*
	 * If the open of pcs750.bin fails it will tell us and we will want
	 * to return.
	 */
	pcs_io = open (pcs, 0);
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
	if (cnt != 22 * 512) {
		printf ("Load pcs750 failed: Error reading %s\n", pcs);
		return;
	}
	/* Go off and load the patch bits and the actual updated microcode. */
	pcsloadpatch ();			/* Load patchbits */
	pcsloadpcs ();				/* Load pcs code */
}

#ifdef DS 

int rpb[0x40];

load_ds(ds_io, howto, unit, devtype)
int ds_io, howto, unit, devtype;
{
	int cnt, i, csr;
	char *addr = (char *) 0xfe00;

	/* initialize rpb */
	for(i=0;i<0x40;i++) {
	 	rpb[i] = 0;
	}
	/*
	 * Build a VMS style Restart Parameter Block for the DS
	 */
	switch(devtype&0xff) {		/* Translate Ultrix major number */
	case 0:
		i=0;			/* Massbus disk */
		break;
	case 3:
		i=1;			/* RK07 disk */
		csr = 0177400;
		break;
	case 9:
	
#define BIADPT	0x007E0000
  		 /* If can be a BDA and csr is not a BI Adapter window */
		if (can_bda && ((saveCSR&BIADPT)== 0)) {
			i=0x21;
			csr = saveCSR;
		}
		else {
			i=0x11;			/* UDA-50 */
			csr = 0172150;
		}
		break;
	case 11:
		i=3;			/* IDC */
		csr = 0175606;
		break;
	case 14:
		i=2;			/* RL11/RL02 */
		csr = 0174400;
		break;
	}
	rpb[0x19] |= i<<16;		/* Plug in RPB$B_DEVTYP */
	rpb[0x19] |= (UNITTODRIVE(unit)&0xffff);
					/* Plug in RPB$W_UNIT */
	rpb[0x0c] = howto;		/* Plug in RPB$L_BOOTR5 */

	if (i==0) {			/* MASSBUS */
		rpb[0x17] = (int)mbamba(unit);	/* RPB$L_ADPPHY */
		rpb[0x15] = (int)mbadrv(unit);	/* RPB$L_CSRPHY */
	} 
	else if (i==0x21) {			/* KDB50 */
		rpb[0x17] = (int)saveCSR & 0x1fff;	/* RPB$L_ADPPHY */
		rpb[0x15] = (int)saveCSR & 0x1fff;      /* RPB$L_CSRPHY */
	}
	else {
		rpb[0x17] = (int)ubauba(unit);	/* RPB$L_ADPPHY */
		rpb[0x15] = (int)ubamem(unit,csr); /* RPB$L_CSRPHY */
	}
	/*
	 * Read in the Diagnostic Supervisor
	 */
	for (i=0; ; i+=10240) {
		cnt = read (ds_io, addr+i, 10240);
		if (!cnt)
			break;
	}
	printf("\n");
	asm ("mfpr $0x3e,r0");		/* Put the SID in r0 */
	asm ("movab _rpb,r11");		/* Pass the address of RPB in R11 */
	asm ("jmp *$0x10000");		/* Startup the DS */
}
#endif DS 
