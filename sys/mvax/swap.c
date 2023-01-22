#ifndef lint
static	char	*sccsid = "@(#)swap.c	1.2	(ULTRIX)	12/4/86";
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
 * Modification history : /sys/vax/swap.c
 *
 * 04-Dec-86 -- prs
 *	Added conditional, so "swap on boot" syntax will work on
 *	a MicroVAX I processor.
 *
 * 06-Aug-86 -- prs
 *	Merged the two files, swapgeneric.c and swapboot.c into one,
 *	swap.c. Added the compiler option SWAPTYPE to distinguish
 *	between the two types of "swap on ...." syntax.
 *	SWAPTYPE = 0 ----> "swap on ra0" or alike syntax
 *	SWAPTYPE = 1 ----> "swap on generic"
 *	SWAPTYPE = 2 ----> "swap on boot"
 *
 ************************************************************************/

#include "mba.h"

#include <strings.h>

#include "../h/types.h"
#include "../vax/pte.h"
#include "../vax/cpu.h"

#include "../h/param.h"
#include "../h/conf.h"
#include "../h/buf.h"
#include "../h/vm.h"
#include "../h/vmmac.h"
#include "../h/systm.h"
#include "../h/reboot.h"

#include "../vax/cons.h"
#include "../vax/mtpr.h"
#include "../vax/rpb.h"
#include "../vaxmba/mbareg.h"
#include "../vaxmba/mbavar.h"
#include "../vaxuba/ubareg.h"
#include "../vaxuba/ubavar.h"

#include "../sas/vmb.h"

/*
 * Swap on boot generic configuration
 */
dev_t	rootdev, argdev, dumpdev;
int	nswap;

#if SWAPTYPE > 0
struct	swdevt swdevt[] = {
	{ -1,	1,	0 },
	{ 0,	0,	0 },
};
#endif

long	dumplo;
int	dmmin, dmmax, dmtext;
int	unit = 0;
int	swaponroot = 0;

extern struct mba_device mbdinit[];
extern struct uba_device ubdinit[];

struct	uba_driver *uba_drive;
struct	uba_hd	   *uba_head;
struct	uba_device *uba_dev;

struct	mba_driver *mba_drive;
struct	mba_hd	   *mba_head;
struct	mba_device *mba_dev;

extern 	int *vmbinfo;

extern	struct rpb;

setconf()
/*
 * Only include this section of code if "swap on boot" syntax was used.
 */
#if SWAPTYPE == 2
{
	register struct genericconf *gc;
	int found_flag = 0;
	long longp;

	if (!vmbinfo) {
		printf("boot device not found\n");
		ask_for_root();
		return(0);
	}
	/*
	 * Search table for boot device
	 */
	for (gc = genericconf; gc->gc_driver; gc++)
		if (gc->gc_BTD_type == rpb.devtyp) {
			found_flag = 1;
			break;
		}
	if (!found_flag) {
		printf("boot device not in table\n");
		ask_for_root();
		return(0);
	}
	unit = -1;
	/*
	 * Scan device structures for ULTRIX number of the boot device.
	 */
	switch(rpb.devtyp) {

	case BTD$K_MB:
		for (mba_dev = (struct mba_device *)mbdinit;mba_dev->mi_driver;mba_dev++) {
			if (mba_dev->mi_alive) {
				mba_drive = mba_dev->mi_driver;
				if (*mba_drive->md_dname == *gc->gc_name)
				if (mba_dev->mi_drive == rpb.unit) {
					mba_head = mba_dev->mi_hd;
					longp = (long)mba_head->mh_physmba;
					if (longp == rpb.adpphy) {
						unit = (long)mba_dev->mi_unit;
						break;
					}
				}
			}
		}
		break;
	case BTD$K_BDA:
		for (uba_dev = (struct uba_device *)ubdinit;uba_dev->ui_driver;uba_dev++) {
			if (uba_dev->ui_alive) {
				longp = (long)uba_dev->ui_physaddr;
				longp &= 0xffffff00;
				if (longp == rpb.csrphy) {
				   uba_drive = uba_dev->ui_driver;
				   if (*uba_drive->ud_dname ==*gc->gc_name)
				      if (uba_dev->ui_slave == rpb.unit) {
				         unit = (long)uba_dev->ui_unit;
				         break;
				      }
				}
			}
		}
		break;
	case BTD$K_DQ:
		for (uba_dev = (struct uba_device *)ubdinit;uba_dev->ui_driver;uba_dev++) {
			if (uba_dev->ui_alive) {
				uba_drive = uba_dev->ui_driver;
				if (*uba_drive->ud_dname ==*gc->gc_name)
				if (uba_dev->ui_slave == rpb.unit) {
				     uba_head = uba_dev->ui_hd;
				     longp = (long)uba_head->uh_physuba;
				     if (longp == rpb.adpphy) {
				 	  unit = (long)uba_dev->ui_unit;
					  break;
				     }
				}
			}
		}
		break;
	default:
		for (uba_dev = (struct uba_device *)ubdinit;uba_dev->ui_driver;uba_dev++) {
			if (uba_dev->ui_alive) {
				if ((long)uba_dev->ui_physaddr == rpb.csrphy) {
					uba_drive = uba_dev->ui_driver;
					if (*uba_drive->ud_dname == *gc->gc_name)
					if (uba_dev->ui_slave == rpb.unit) {
					     if((cpu_subtype == ST_VAXSTAR) ||
					       (cpu == MVAX_I)) {
					 	 unit = (long)uba_dev->ui_unit;
						 break;
					     }
					     uba_head = uba_dev->ui_hd;
					     longp = (long)uba_head->uh_physuba;
					     if (longp == rpb.adpphy) {
					 	  unit = (long)uba_dev->ui_unit;
						  break;
					     }
					}
				}
			}
		}
	} /* switch */

	if (unit == -1) {
		printf("root device not configured\n");
		ask_for_root();
		return(0);
	}
	do_config(gc, unit);
	return;
}

/*
 * This code is taken from swapgeneric
 */

ask_for_root()
#endif SWAPTYPE == 2
{
/*
 * If the "swap on ra0" or alike syntax was used, then basically return.
 * Please note that the "swap on boot" and the "swap on generic" syntax
 * use this section of code.
 */
#if SWAPTYPE > 0
	register struct mba_device *mi;
	register struct uba_device *ui;
	register struct genericconf *gc;
	char name[128];
	char *name_hold = "  ";
/*
 * The swapgeneric code essentially had 2 functions, one to
 * prompt the user for the root device if the boot ask flag was set.
 * The second funtion it had was to first try to find a suitable root,
 * if none was found then halt. Since the "swap on boot" syntax defaults
 * to the prompting functionality, we have to special case the functionality
 * of the swap program figuring out a suitable root.
 */
#if SWAPTYPE == 1
	if (boothowto & RB_ASKNAME) {
#endif
retry:
		printf("root device? ");
		gets(name);
		for (gc = genericconf; gc->gc_driver; gc++)
			if (gc->gc_name[0] == name[0] &&
		    	gc->gc_name[1] == name[1])
				goto gotit;
		goto bad;
gotit:
		if (name[3] == '*') {
			name[3] = name[4];
			swaponroot++;
		}
		if (name[2] >= '0' && name[2] <= '7' && name[3] == 0) {
			unit = name[2] - '0';
			do_config(gc, unit);
			return;
		}
		printf("bad/missing unit number\n");
bad:
		printf("use ");
		for (gc = genericconf; gc->gc_driver; gc++) {
			/*
		 	 * Don't print ra twice !!
		 	 */
			if (strcmp(name_hold, gc->gc_name) != 0)
				printf("%s%%d ", gc->gc_name);
			name_hold = gc->gc_name;
		}
		printf("\n");
		goto retry;
/*
 * We must special case the functionality of calculating a suitable root
 * device, so the "swap on boot" syntax can take advantage of this code.
 */
#if SWAPTYPE == 1
	}
	unit = 0;
	for (gc = genericconf; gc->gc_driver; gc++) {
		for (mi = mbdinit; mi->mi_driver; mi++) {
			if (mi->mi_alive == 0)
				continue;
			if (mi->mi_unit == 0 && mi->mi_driver ==
			    (struct mba_driver *)gc->gc_driver) {
				printf("root on %s0\n",
				    mi->mi_driver->md_dname);
				do_config(gc, unit);
				return;
			}
		}
		for (ui = ubdinit; ui->ui_driver; ui++) {
			if (ui->ui_alive == 0)
				continue;
			if (ui->ui_unit == 0 && ui->ui_driver ==
			    (struct uba_driver *)gc->gc_driver) {
				printf("root on %s0\n",
				    ui->ui_driver->ud_dname);
				do_config(gc, unit);
				return;
			}
		}
	}
	printf("no suitable root\n");
	asm("halt");
#endif SWAPTYPE == 1
#endif SWAPTYPE > 0
}

do_config(gc, unit)
	struct genericconf *gc;
	int unit;
{
	gc->gc_root = makedev(major(gc->gc_root), unit*8);
	rootdev = gc->gc_root;
	swdevt[0].sw_dev = argdev = dumpdev =
	    makedev(major(rootdev), minor(rootdev)+1);
	/* swap size and dumplo set during autoconfigure */
	if (swaponroot)
		rootdev = dumpdev;
}

getchar()
{
	register c;
#ifdef MVAX
	extern (*v_consgetc)();
	if( v_consgetc ) {
		c = (*v_consgetc)();
	} else {
#endif MVAX
		while ((mfpr(RXCS)&RXCS_DONE) == 0)
			;
		c = mfpr(RXDB)&0177;
#ifdef MVAX
	}
#endif MVAX
	if (c == '\r')
		c = '\n';
	cnputc(c);
	return (c);
}

gets(cp)
	char *cp;
{
	register char *lp;
	register c;

	lp = cp;
	for (;;) {
		c = getchar() & 0177;
		switch (c) {
		case 0021:
		case 0023:
			continue;	/* ignore ^Q and ^S */
		case '\n':
		case '\r':
			*lp++ = '\0';
			return;
		case '\b':
		case '#':
			lp--;
			if (lp < cp)
				lp = cp;
			continue;
		case '@':
		case 'u'&037:
			lp = cp;
			cnputc('\n');
			continue;
		default:
			*lp++ = c;
		}
	}
}
