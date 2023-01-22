# ifndef lint
static char *sccsid = "@(#)uba.c	1.9	(ULTRIX)	3/23/86";
# endif not lint

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


/* ------------------------------------------------------------------------
 * Modification History: /sys/stand/uba.c
 *
 * 19-Jun-85 -- jaw
 *	VAX8200 name change.
 *
 * 20-Mar-85 -jaw
 *	Changes for support of the VAX8200 were merged in.
 *
 * 27-Feb-85 -tresvik
 *	Changes for support of the VAX8600 were merged in.
 *
 * 12 Nov 84 -- rjl
 *	Fixed type casting problem
 *
 * 29 Dec 83 --jmcg
 *	Added handling for MicroVAX_I Q22 bus.  There is no mapping
 *	available.   Fortunately none is necessary for the restricted
 *	set of operations attempted by the standalone code.
 *
 * 28 Dec 83 --jmcg
 *	Derived from 4.2BSD, labeled:
 *		uba.c	6.1	83/07/29
 *
 * ------------------------------------------------------------------------
 */

#include "../machine/pte.h"

#include "../h/param.h"
#include "../h/gnode.h"
#include "../h/vm.h"

#include "../vax/cpu.h"
#include "../vaxuba/ubareg.h"
#include "../vaxbi/buareg.h"

#include "saio.h"
#include "savax.h"

/*
 * Note... this routine does not
 * really allocate; unless bdp == 2
 * you always get the same space.
 * When bdp == 2 you get some other space.
 */
ubasetup(io, bdp)
	register struct iob *io;
	int bdp;
{
	int npf;
	unsigned v;
	register struct pte *pte;
	int o, temp, reg;
	static int lastreg = 128+64;

	if( cpu == MVAX_I )
		return	(int)io->i_ma;
	v = btop(io->i_ma);
	o = (int)io->i_ma & PGOFSET;
	npf = btoc(io->i_cc + o) +1;
	if (bdp == 2) {
		reg = lastreg;
		lastreg += npf;
		bdp = 0;
	} else
		reg = 0;
	
	if (cpu == VAX8200) pte = (&((struct bua_regs *)ubauba(io->i_unit))->bua_map[reg]);


	else pte = &ubauba(io->i_unit)->uba_map[reg];
	temp = (bdp << 21) | UBAMR_MRV;
	if (bdp && (o & 01))
		temp |= UBAMR_BO;
	v &= 0x1fffff;			/* drop to physical addr */
	while (--npf != 0)
		*(int *)pte++ = v++ | temp;
	*(int *)pte++ = 0;
	return ((bdp << 28) | (reg << 9) | o);
}

ubafree(io, mr)
	struct iob *io;
	int mr;
{
	register int bdp;
 
	bdp = (mr >> 28) & 0x0f;
	if (bdp == 0)
		return;
	switch (cpu) {

	case VAX_8200:
#if defined(VAX8200)

		((struct bua_regs *)(ubauba(io->i_unit)))->bua_dpr[bdp] 
			|= BUADPR_PURGE;
		break;
#endif 

	case VAX_8600:
	case VAX_780:
#if defined(VAX780) || defined(VAX8600)
		ubauba(io->i_unit)->uba_dpr[bdp] |= UBADPR_BNE;
		break;
#endif

	case VAX_750:
#if defined(VAX750)
		ubauba(io->i_unit)->uba_dpr[bdp] |=
		     UBADPR_PURGE|UBADPR_NXM|UBADPR_UCE;
		break;
#endif

	case VAX_730:
		break;
	case MVAX_I:
	case MVAX_II:
		break;
	}
}
