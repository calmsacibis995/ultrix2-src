#ifndef lint
static char *sccsid = "@(#)mkioconf.c	1.9	ULTRIX	10/3/86";
#endif

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

/*-----------------------------------------------------------------------
 *
 *	Modification History
 *
 * 27-Aug-86 -- jmartin
 *	Fix segmentation fault caused here by missing or misplaced
 *	"controller" statement (SMU-307).
 *
 * 15-Apr-86 -- afd
 *	Removed reference to MACHINE_MVAX
 *
 * 8 Apr 86  -- lp
 *	Added bvp support
 *
 * 06-Mar-86 -- jrs
 *	Fix problem when both bi and massbus devices exist
 *
 * 05-Mar-86 -- jrs
 *	Add support for configuring native bi devices
 *
 *-----------------------------------------------------------------------
 */

#include <stdio.h>
#include "y.tab.h"
#include "config.h"

/*
 * build the ioconf.c file
 */
char	*qu();
char	*intv();

#if MACHINE_VAX
vax_ioconf()
{
	register struct device *dp, *mp, *np;
	register int uba_n, slave;
	int directconn;
	FILE *fp,*fpb;

	fp = fopen(path("ioconf.c"), "w");
	if (fp == 0) {
		perror(path("ioconf.c"));
		exit(1);
	}
	fpb = fopen(path("iobus.c"), "w");
	if (fpb == 0) {
		perror(path("iobus.c"));
		exit(1);
	}
	fprintf(fp, "#include \"../machine/pte.h\"\n");
	fprintf(fp, "#include \"../h/param.h\"\n");
	fprintf(fp, "#include \"../h/buf.h\"\n");
	fprintf(fp, "#include \"../h/map.h\"\n");
	fprintf(fp, "#include \"../h/vm.h\"\n");
	fprintf(fp, "#include \"../h/config.h\"\n");
	fprintf(fp, "\n");
	fprintf(fp, "#include \"../vaxmba/mbavar.h\"\n");
	fprintf(fp, "#include \"../vaxuba/ubavar.h\"\n\n");
	fprintf(fp, "\n");
	fprintf(fp, "#define C (caddr_t)\n\n");
	/*
	 * First print the mba initialization structures
	 */
	if (seen_mba) {
		for (dp = dtab; dp != 0; dp = dp->d_next) {
			mp = dp->d_conn;
			if(mp == 0 || mp == TO_NEXUS || !eq(mp->d_name, "mba"))
				continue;
			fprintf(fp, "extern struct mba_driver %sdriver;\n",
			    dp->d_name);
		}
		fprintf(fp, "\nstruct mba_device mbdinit[] = {\n");
		fprintf(fp, "\t/* Device,  Unit, Mba, Drive, Dk */\n");
		for (dp = dtab; dp != 0; dp = dp->d_next) {
			mp = dp->d_conn;
			if (dp->d_unit == QUES || mp == 0 ||
			    mp == TO_NEXUS || !eq(mp->d_name, "mba"))
				continue;
			if (dp->d_addr) {
				printf("can't specify csr address on mba for %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_vec != 0) {
				printf("can't specify vector for %s%d on mba\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_drive == UNKNOWN) {
				printf("drive not specified for %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_slave != UNKNOWN) {
				printf("can't specify slave number for %s%d\n", 
				    dp->d_name, dp->d_unit);
				continue;
			}
			fprintf(fp, "\t{ &%sdriver, %d,   %s,",
				dp->d_name, dp->d_unit, qu(mp->d_unit));
			fprintf(fp, "  %s,  %d },\n",
				qu(dp->d_drive), dp->d_dk);
		}
		fprintf(fp, "\t0\n};\n\n");
		/*
		 * Print the mbsinit structure
		 * Driver Controller Unit Slave
		 */
		fprintf(fp, "struct mba_slave mbsinit [] = {\n");
		fprintf(fp, "\t/* Driver,  Ctlr, Unit, Slave */\n");
		for (dp = dtab; dp != 0; dp = dp->d_next) {
			/*
			 * All slaves are connected to something which
			 * is connected to the massbus.
			 */
			if ((mp = dp->d_conn) == 0 || mp == TO_NEXUS)
				continue;
			np = mp->d_conn;
			if (np == 0 || np == TO_NEXUS ||!eq(np->d_name, "mba"))
				continue;
			fprintf(fp, "\t{ &%sdriver, %s",
			    mp->d_name, qu(mp->d_unit));
			fprintf(fp, ",  %2d,    %s },\n",
			    dp->d_unit, qu(dp->d_slave));
		}
		fprintf(fp, "\t0\n};\n\n");
	}
	/*
	 * Now generate interrupt vectors for the unibus
	 */
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if (dp->d_vec != 0) {
			struct idlst *ip;
			mp = dp->d_conn;
			if (mp == 0 || mp == TO_NEXUS||can_connect(dp->d_name))
				continue;
			if( (mp->d_conn != 0 && mp->d_conn != TO_NEXUS &&
				   eq(mp->d_conn->d_name,"mba")) )
			    continue;
			if(!can_connect(mp->d_name))
			    continue;
			fprintf(fp,
			    "extern struct uba_driver %sdriver;\n",dp->d_name);
			if(ip = dp->d_vec) {
				fprintf(fp, "extern ");
				
				for (;;) {
				    fprintf(fp,"X%s%d()", ip->id,  dp->d_unit);
				    ip = ip->id_next;
				    if (ip == 0)
					break;
				    fprintf(fp, ", ");
				}
				fprintf(fp, ";\n");
				fprintf(fp, "int\t (*%sint%d[])() = { ", dp->d_name,
					 dp->d_unit);
				ip = dp->d_vec;
				for (;;) {
				    fprintf(fp, "X%s%d", ip->id, dp->d_unit);
				    ip = ip->id_next;
				    if (ip == 0)
					break;
				    fprintf(fp, ", ");
				}
				fprintf(fp, ", 0 } ;\n");
			}
		}
	}
	fprintf(fp, "\nstruct uba_ctlr ubminit[] = {\n");
	fprintf(fp, "/*\t driver,\tctlr, \tparent, \tadpt, nexus,\tubanum,\talive,\tintr,\taddr*/\n");
	fprintf(fpb, "\nstruct config_adpt config_adpt[] = {\n");
	dump_adapt("vaxbi", fpb);
	dump_adapt("ci", fpb);
	dump_adapt("uba", fpb);
	dump_adapt("hsc", fpb);
	
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (dp->d_type != CONTROLLER || mp == TO_NEXUS || mp == 0
			|| dp->d_unit == QUES)
			continue;
		if(can_connect(dp->d_name))
		    continue;
		if (needs_vector(mp->d_name)) {
			if (dp->d_vec == 0) {
				printf("must specify vector for %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_addr == 0 && needs_csr(mp->d_name)) {
				printf("must specify csr address for %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
		}
		if (dp->d_drive != UNKNOWN || dp->d_slave != UNKNOWN) {
			printf("drives need their own entries; dont ");
			printf("specify drive or slave for %s%d\n",
			    dp->d_name, dp->d_unit);
			continue;
		}
		if (dp->d_flags) {
			printf("controllers (e.g. %s%d) ",
			    dp->d_name, dp->d_unit);
			printf("don't have flags, only devices do\n");
			continue;
		}
		
		if ((mp->d_name) && needs_pseudo_uba(mp->d_name)||
		      (eq(mp->d_name,"vaxbi")&&needs_pseudo_uba(dp->d_name))){
			if (dp->d_extranum == UNKNOWN) {
				uba_n = UNKNOWN;
			} else {
				uba_n = highuba + dp->d_extranum + 1;
			}
		} else {
			uba_n = uba_num(mp);
		}
		fprintf(fp, "\t{ &%sdriver,\t\"%s\",\t%d,  0, \t%s,",
		    dp->d_name, dp->d_name, dp->d_unit, qu(dp->d_adaptor));

		fprintf(fp," \t%s, ", qu(dp->d_nexus));
		fprintf(fp,"\t%s,",qu(uba_n));
		fprintf(fp,
		    "\t0,\t%sint%d, C 0%o",
		    dp->d_name, dp->d_unit, dp->d_addr);
		fprintf(fp, "},\n");
		if(can_connect(mp->d_name)) {
			/* dump out a bus structure for the unmentioned
			 * parent
			 */
		    print_tree(dp->d_conn,fpb);
		    fprintf(fpb,"\t{\"%s\", %s,C &%sdriver, %d, 'C' },\n",
			mp->d_name, qu(mp->d_unit), dp->d_name, dp->d_unit);
		    
		}
	}
	fprintf(fp, "\t0\n};\n");
/* unibus devices */
	fprintf(fp, "\nstruct uba_device ubdinit[] = {\n");
	fprintf(fp,
"\t/* driver,  unit, parent, adpt, nexus, ubanum,ctlr, slave,   intr,    addr,    dk, flags, name*/\n");
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (dp->d_unit == QUES || dp->d_type != DEVICE || mp == 0 ||
		    mp == TO_NEXUS ||
		    (mp->d_type == MASTER || eq(mp->d_name, "mba")))
			continue;
		np = mp->d_conn;
		if (np != 0 && np != TO_NEXUS && eq(np->d_name, "mba"))
		    continue;

		np = 0;
		directconn = (can_connect(mp->d_name));
		
		if (directconn) {
			if (dp->d_vec == 0) {
				printf("must specify vector for device %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (needs_csr(mp->d_name) && dp->d_addr == 0) {
				printf("must specify csr for device %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_drive != UNKNOWN || dp->d_slave != UNKNOWN) {
				printf("drives/slaves can be specified ");
				printf("only for controllers, ");
				printf("not for device %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}


/*			if (eq(dp->d_name,"uq") &&
			    needs_pseudo_uba(dp->d_conn)) {

*/
		if ((dp->d_conn) && needs_pseudo_uba(dp->d_conn)) {

				if (dp->d_extranum == UNKNOWN) {
					uba_n = UNKNOWN;
				} else {
					uba_n = highuba + dp->d_extranum + 1; 
				}
			} else {
				if(eq("uba",mp->d_name))
				    uba_n = mp->d_unit;
				else
				    uba_n = UNKNOWN;
			}
			slave = QUES;
		} else {
			if ((np = mp->d_conn) == 0) {
				printf("%s%d isn't connected to anything ",
				    mp->d_name, mp->d_unit);
				printf(", so %s%d is unattached\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (np != TO_NEXUS) {
			    if(needs_pseudo_uba(np->d_name) ||
			      ((mp) && needs_pseudo_uba(mp->d_name)
				 )) {
				if (mp->d_extranum == UNKNOWN) {
					uba_n = QUES;
				} else {
					uba_n = highuba + mp->d_extranum + 1; 
				}
			    } else {
		 		if(mp->d_unit == QUES)
				    uba_n = QUES;
				else
				    uba_n = uba_num(np);
			    }
			} else {
			    uba_n = QUES;
			}
			if (dp->d_drive == UNKNOWN) {
				printf("must specify ``drive number'' ");
				printf("for %s%d\n", dp->d_name, dp->d_unit);
				continue;
			}
			/* NOTE THAT ON THE UNIBUS ``drive'' IS STORED IN */
			/* ``SLAVE'' AND WE DON'T WANT A SLAVE SPECIFIED */
			if (dp->d_slave != UNKNOWN) {
				printf("slave numbers should be given only ");
				printf("for massbus tapes, not for %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_vec != 0) {
				printf("interrupt vectors should not be ");
				printf("given for drive %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_addr != 0) {
				printf("csr addresses should be given only ");
				printf("on controllers, not on %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			slave = dp->d_drive;
		}
		fprintf(fp, "\t{ &%sdriver,\t\"%s\",  %2d, 0, %s,",
		    directconn? dp->d_name : mp->d_name, 
		    dp->d_name, dp->d_unit,
		    qu(dp->d_adaptor));

		fprintf(fp, " %s, ",  qu(dp->d_nexus)); 
		fprintf(fp, " %s, ", qu(uba_n));

		fprintf(fp, " %s, ",directconn? " -1" : qu(mp->d_unit));
		fprintf(fp, "  %2d,   %s, C 0%-6o,  %d,",
		    slave, intv(dp), dp->d_addr, dp->d_dk);
		fprintf(fp, "  0x%x",
		    dp->d_flags);
		fprintf(fp, " },\n");
		if(can_connect(mp->d_name)&& directconn) {
			/* dump out a bus structure for the unmentioned
			 * parent
			 */
		   print_tree(dp->d_conn,fpb);
		   fprintf(fpb,"\t{\"%s\", %s,C &%sdriver, %d, 'D' },\n",
			mp->d_name, qu(mp->d_unit), dp->d_name, dp->d_unit);
		}

	}
	fprintf(fpb, "\t0\n};\n");
	fprintf(fp, "\t0\n};\n\n#include \"iobus.c\"\n", extrauba);
	(void) fclose(fp);
}
#endif

#if MACHINE_SUN
sun_ioconf()
{
	register struct device *dp, *mp;
	register int slave;
	FILE *fp;

	fp = fopen(path("ioconf.c"), "w");
	if (fp == 0) {
		perror(path("ioconf.c"));
		exit(1);
	}
	fprintf(fp, "#include \"../h/param.h\"\n");
	fprintf(fp, "#include \"../h/buf.h\"\n");
	fprintf(fp, "#include \"../h/map.h\"\n");
	fprintf(fp, "#include \"../h/vm.h\"\n");
	fprintf(fp, "\n");
	fprintf(fp, "#include \"../sundev/mbvar.h\"\n");
	fprintf(fp, "\n");
	fprintf(fp, "#define C (caddr_t)\n\n");
	fprintf(fp, "\n");
	/*
	 * Now generate interrupt vectors for the Multibus
	 */
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if (dp->d_pri != 0) {
			mp = dp->d_conn;
			if (mp == 0 || mp == TO_NEXUS ||
			    !eq(mp->d_name, "mb"))
				continue;
			fprintf(fp, "extern struct mb_driver %sdriver;\n",
			    dp->d_name);
		}
	}
	/*
	 * Now spew forth the mb_cinfo structure
	 */
	fprintf(fp, "\nstruct mb_ctlr mbcinit[] = {\n");
	fprintf(fp, "/*\t driver,\tctlr,\talive,\taddr,\tintpri */\n");
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (dp->d_type != CONTROLLER || mp == TO_NEXUS || mp == 0 ||
		    !eq(mp->d_name, "mb"))
			continue;
		if (dp->d_pri == 0) {
			printf("must specify priority for %s%d\n",
			    dp->d_name, dp->d_unit);
			continue;
		}
		if (dp->d_addr == 0) {
			printf("must specify csr address for %s%d\n",
			    dp->d_name, dp->d_unit);
			continue;
		}
		if (dp->d_drive != UNKNOWN || dp->d_slave != UNKNOWN) {
			printf("drives need their own entries; ");
			printf("dont specify drive or slave for %s%d\n",
			    dp->d_name, dp->d_unit);
			continue;
		}
		if (dp->d_flags) {
			printf("controllers (e.g. %s%d) don't have flags, ");
			printf("only devices do\n",
			    dp->d_name, dp->d_unit);
			continue;
		}
		fprintf(fp, "\t{ &%sdriver,\t%d,\t0,\tC 0x%x,\t%d },\n",
		    dp->d_name, dp->d_unit, dp->d_addr, dp->d_pri);
	}
	fprintf(fp, "\t0\n};\n");
	/*
	 * Now we go for the mb_device stuff
	 */
	fprintf(fp, "\nstruct mb_device mbdinit[] = {\n");
	fprintf(fp,
"\t/* driver,  unit, ctlr,  slave,   addr,    pri,    dk, flags*/\n");
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (dp->d_unit == QUES || dp->d_type != DEVICE || mp == 0 ||
		    mp == TO_NEXUS || mp->d_type == MASTER ||
		    eq(mp->d_name, "mba"))
			continue;
		if (eq(mp->d_name, "mb")) {
			if (dp->d_pri == 0) {
				printf("must specify vector for device %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_addr == 0) {
				printf("must specify csr for device %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_drive != UNKNOWN || dp->d_slave != UNKNOWN) {
				printf("drives/slaves can be specified only ");
				printf("for controllers, not for device %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			slave = QUES;
		} else {
			if (mp->d_conn == 0) {
				printf("%s%d isn't connected to anything, ",
				    mp->d_name, mp->d_unit);
				printf("so %s%d is unattached\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_drive == UNKNOWN) {
				printf("must specify ``drive number'' for %s%d\n",
				   dp->d_name, dp->d_unit);
				continue;
			}
			/* NOTE THAT ON THE UNIBUS ``drive'' IS STORED IN */
			/* ``SLAVE'' AND WE DON'T WANT A SLAVE SPECIFIED */
			if (dp->d_slave != UNKNOWN) {
				printf("slave numbers should be given only ");
				printf("for massbus tapes, not for %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_pri != 0) {
				printf("interrupt priority should not be ");
				printf("given for drive %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			if (dp->d_addr != 0) {
				printf("csr addresses should be given only");
				printf("on controllers, not on %s%d\n",
				    dp->d_name, dp->d_unit);
				continue;
			}
			slave = dp->d_drive;
		}
		fprintf(fp,
"\t{ &%sdriver,  %2d,   %s,    %2d,   C 0x%x, %d,  %d,  0x%x },\n",
		    eq(mp->d_name, "mb") ? dp->d_name : mp->d_name, dp->d_unit,
		    eq(mp->d_name, "mb") ? " -1" : qu(mp->d_unit),
		    slave, dp->d_addr, dp->d_pri, dp->d_dk, dp->d_flags);
	}
	fprintf(fp, "\t0\n};\n");
	(void) fclose(fp);
}
#endif

char *intv(dev)
	register struct device *dev;
{
	static char buf[20];
	register struct device *mp = dev->d_conn;

	if (dev->d_vec == 0)
		return ("     0");
	return (sprintf(buf, "%sint%d", dev->d_name, dev->d_unit));
}

char *
qu(num)
{

	if (num == QUES)
		return ("'?'");
	if (num == UNKNOWN)
		return (" -1");
	return (sprintf(errbuf, "%3d", num));
}

char  *tbl_can_connect[] = { "uba","mba","vaxbi","kdb","klesib","klesiu","aio","aie",
			     "uda", "ci", 0};

/*
 * can things connect to this name?
 */
can_connect(name)
register	char *name;
{
    register	char	**ptr = tbl_can_connect;
    
    while(*ptr)
	{
	    if(eq(*ptr,name))return(1);
	    ptr++;
	}
    return(0);
}

char *tbl_needs_vector[] = {  "uba", "vaxbi","aio","aie", "kdb", "klesib",
			      "uda", "klesiu", "klesiq", 0  };
/*
 * needs_vector(name) things connect to this device need to supply a vector
 */
needs_vector(name)
register char *name;
{
    register char **ptr = tbl_needs_vector;
    
    while(*ptr)
	{
	    if(eq(*ptr,name))return(1);
	    ptr++;
	}
    return(0);
}

char *tbl_needs_csr[] = { "uba", "uda", 0};


/*
 * needs_csr(name)
 * things connect to this device need to specify a csr.
 */
needs_csr(name)
register char *name;
{
    register char **ptr = tbl_needs_csr;
    
    while(*ptr)
	{
	    if(eq(*ptr,name))return(1);
	    ptr++;
	}
    return(0);
}

/*
 * print_tree(dp, fpb)
 *
 */
print_tree(dp, fpb)
register  struct device *dp;
register  FILE *fpb;
{
    register	struct device *mp = dp->d_conn;
    
    if(mp == TO_NEXUS || eq(dp->d_name,"uba") || mp == 0)
	return;
    
    print_tree(dp->d_conn,fpb);
    
    fprintf(fpb,"\t{\"%s\", %s, C \"%s\", %d, 'A' },\n",
	 mp->d_name, qu(mp->d_unit), dp->d_name, dp->d_unit);
}

/*
 *dump out a adapter line for the named device.
 */
dump_adapt(str,fpb)
register	char	*str;
register	FILE	*fpb;
{
    register	struct device *dp, *mp;
    
    for (dp = dtab; dp != 0; dp = dp->d_next) {
	if(eq(dp->d_name,str)) 
	    {
		print_tree(dp,fpb);
		
		if ((mp = dp->d_conn) == TO_NEXUS) 
		{
			fprintf(fpb,"\t{\"%s\", '%s', C \"%s\", %d, 'A' },\n",
				"nexus", "?", dp->d_name, dp->d_unit);
		} else {
		    fprintf(fpb,"\t{\"%s\", %s, C \"%s\", %d, 'A' },\n",
			  mp->d_name, qu(mp->d_unit), dp->d_name, dp->d_unit);
		}
		
			    
		
	    }
    }
}

uba_num(dp)
struct device *dp;
{
	if(eq(dp->d_name ,"uba")) return(dp->d_unit);
	if(dp->d_conn == TO_NEXUS || dp->d_conn == 0) return(UNKNOWN);
	return(uba_num(dp->d_conn));
}

