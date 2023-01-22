
#ifndef lint
static char *sccsid = "@(#)mkubglue.c	1.9	ULTRIX	10/3/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984,86 by			*
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
 * Make the uba interrupt file ubglue.s
 */

/*************************************************************************
 *			Modification History
 *
 * 2 jul 86  -- fred (Fred Canter)
 *	Include pseudo DMA code if VAXstar console SLU (ss) configured.
 *
 * 8 Apr 86  -- lp
 *	Added bvp support
 *
 * 05-Mar-86 -- jrs
 *	Added support for configuring direct bi devices.
 *
 * 19-FEB-86 -- jaw
 *	added other direct vectored VAX 8800.
 *
 * 11-Sep-85 -- jaw
 *	added other direct vectored VAX's (8200,MVAX).
 *
 * 5-May-85 -- Larry Cohen
 *	special case the dmz so that we can determine which octet is
 *	interrupting
 */

#include <stdio.h>
#include "config.h"
#include "y.tab.h"

ubglue()
{
	register FILE *fp;
	register struct device *dp, *mp;

	fp = fopen(path("ubglue.s"), "w");
	if (fp == 0) {
		perror(path("ubglue.s"));
		exit(1);
	}
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (mp != 0 && 
		    ((mp != TO_NEXUS && !eq(mp->d_name, "mba"))
		     || eq(dp->d_name,"ci")) ) {
			struct idlst *id, *id2;

			for (id = dp->d_vec; id; id = id->id_next) {
				for (id2 = dp->d_vec; id2; id2 = id2->id_next) {
					if (id2 == id) {
						dump_vec(fp, id->id, 
							 dp->d_unit);
						break;
					}
					if (!strcmp(id->id, id2->id))
						break;
				}
			}
		}
	}
	(void) fclose(fp);
}

int dmzr = 0;
int dmzx = 0;
/*
 * print an interrupt vector
 */
dump_vec(fp, vector, number)
	register FILE *fp;
	char *vector;
	int number;
{
	char nbuf[80];
	register char *v = nbuf;

	(void) sprintf(v, "%s%d", vector, number);
	fprintf(fp, "\t.globl\t_X%s\n\t.align\t2\n_X%s:\n\tpushr\t$0x3f\n",
	    v, v);
	if (strncmp(vector, "dzx", 3) == 0)
		fprintf(fp, "\tmovl\t$%d,r0\n\tjmp\tdzdma\n\n", number);
	else if (strncmp(vector, "ssx", 3) == 0)
		fprintf(fp, "\tmovl\t$%d,r0\n\tjmp\tssdma\n\n", number);
	else {
		if (strncmp(vector, "uur", 3) == 0) {
			fprintf(fp, "#ifdef UUDMA\n");
			fprintf(fp, "\tmovl\t$%d,r0\n\tjsb\tuudma\n", number);
			fprintf(fp, "#endif\n");
		}
		if (strncmp(vector, "dmzr", 4)==0 )  {
			vector[strlen(vector)-1] = '\0';
			fprintf(fp, "\tpushl\t$%d\n", (dmzr++ % 3));
		}
		if (strncmp(vector, "dmzx", 4)==0 ) {
			vector[strlen(vector)-1] = '\0';
			fprintf(fp, "\tpushl\t$%d\n", (dmzx++ % 3));
		}
		fprintf(fp, "\tpushl\t$%d\n", number);
		if (strncmp(vector, "dmz", 3)==0 )
			fprintf(fp, "\tcalls\t$2,_%s\n\tpopr\t$0x3f\n", vector);
		else
			fprintf(fp, "\tcalls\t$1,_%s\n\tpopr\t$0x3f\n", vector);
		fprintf(fp, "#if defined(VAX750) || defined(VAX730) || defined(VAX8200) || defined(MVAX) || defined(VAX8800) \n");
		fprintf(fp, "\tincl\t_cnt+V_INTR\n#endif\n\trei\n\n");
	}
}
