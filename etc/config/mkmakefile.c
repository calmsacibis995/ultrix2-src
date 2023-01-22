#ifndef lint
static	char	*sccsid = "@(#)mkmakefile.c	1.25	(ULTRIX)	8/10/86";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1985,86 by			*
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
 *	Modification History
 *
 * 06-Aug-86 -- prs
 *	Added changes to support the merging of the swapboot and
 *	swapgeneric files.
 *
 * 15-Apr-86 -- afd
 *	Removed references to MACHINE_MVAX.
 *
 * 01 Apr 86 -- depp
 *	Added in shared memory configurable items
 *
 * 05-Mar-86 -- bjg
 *	Removed msgbuf.h dependency (for assym.s);
 *	msgbuf removed from kernel
 *
 * 25-Feb-86 -- jrs
 *	Changed to allow multiple "needs" files per files.* line
 *	Changed to support "swap on boot" similar to "swap on generic"
 *	Also changed maxusers check to warning rather than hard bound of 8
 *
 * 05-Feb-86 -- jrs
 *	Moved cpu aliasing out to parser for earlier resolution
 *
 * 08 Aug 85 -- darrell
 *	Added rules for making genassym.c.
 *
 * 19-Jul-85 -- tresvik
 *	Increased maxusers limit from 128 to 256 for larger systems.
 *	Also, force on -DVAX8600 whenever VAX8650 is defined as a cpu.
 *
 * 18 Jun 85 -- reilly
 *      Fix the emulation problem when doing the -s option in
 *      the target directory. 
 *
 * 24 Mar 85 -- depp
 *	Added new config specs "release" and "version".  Since this is
 *	a System V convention, release indicates 1.0, 1.1, 1.2, 2.0, etc.
 *	Version indicates a subrelease 0, 1, 2, ...
 *
 * 11 Apr 85 -- depp
 *	Modified do_cfiles to restrict the size of CFILES.  Additional
 *	files are now placed in CFILES1.
 *
 * 22 Mar 85 -- reilly
 *	Added code for the EMULFLT option ( float point emulation )
 *
 * 06 Mar 85 -- reilly
 *	Modified so that the option BINARY is done correctly.
 *
 * 25 Oct 84 -- rjl
 *	Added support for  MicroVAX binary kits. In line emulation code
 *	and name change to BINARY.machinename format for BINARY directory.
 */

/*
 * Build the makefile for the system, from
 * the information in the files files and the
 * additional files for the machine being compiled to.
 */

#include <stdio.h>
#include <ctype.h>
#include "y.tab.h"
#include "config.h"
#include <machine/param.h>
#include <machine/vmparam.h>

int	source = 0;
static	struct file_list *fcur;
char *tail();

/*
 * Lookup a file, by make.
 */
struct file_list *
fl_lookup(file)
	register char *file;
{
	register struct file_list *fp;

	for (fp = ftab ; fp != 0; fp = fp->f_next) {
		if (eq(fp->f_fn, file))
			return (fp);
	}
	return (0);
}

/*
 * Lookup a file, by final component name.
 */
struct file_list *
fltail_lookup(file)
	register char *file;
{
	register struct file_list *fp;

	for (fp = ftab ; fp != 0; fp = fp->f_next) {
		if (eq(tail(fp->f_fn), tail(file)))
			return (fp);
	}
	return (0);
}

/*
 * Make a new file list entry
 */
struct file_list *
new_fent()
{
	register struct file_list *fp;
	int i;

	fp = (struct file_list *) malloc(sizeof *fp);
	for (i = 0; i < NNEEDS; i++) {
		fp->f_needs[i] = 0;
	}
	fp->f_next = 0;
	fp->f_flags = 0;
	fp->f_type = 0;
	if (fcur == 0)
		fcur = ftab = fp;
	else
		fcur->f_next = fp;
	fcur = fp;
	return (fp);
}

char	*COPTS;
/*
 *  Dont't load if the BINARY option is specified
 */
int dontload = 0;

/*
 * Emulation flag
 */
int emulation_float = 0;

/*
 * Build the makefile from the skeleton
 */
makefile()
{
	FILE *ifp, *ofp;
	char line[BUFSIZ];
	struct opt *op;
	int min_dmmin; /* min size of dmmin - min vmem allocation chunk */

	read_files();
	strcpy(line, "../conf/makefile.");
	(void) strcat(line, machinename);
	ifp = fopen(line, "r");
	if (ifp == 0) {
		perror(line);
		exit(1);
	}
	ofp = fopen(path("makefile"), "w");
	if (ofp == 0) {
		perror(path("makefile"));
		exit(1);
	}
	fprintf(ofp, "IDENT=-D%s", raise(ident));
	if (profiling)
		fprintf(ofp, " -DGPROF");
	if (cputype == 0) {
		printf("cpu type must be specified\n");
		exit(1);
	}

	{ struct cputype *cp, *prevp;
	  for (cp = cputype; cp; cp = cp->cpu_next) {
		for (prevp = cputype; prevp != cp; prevp = prevp->cpu_next) {
			if (strcmp(cp->cpu_name, prevp->cpu_name) == 0) {
				break;
			}
		}
		if (prevp == cp) {
			fprintf(ofp, " -D%s", cp->cpu_name);
		}
	  }
	}

	if (source)
		fprintf(ofp," -DRELEASE='\"'%3.1f'\"' -DVERSION='\"'%d'\"'",
				release,version);

	for (op = opt; op; op = op->op_next)
		if (op->op_value)
			fprintf(ofp, " -D%s=\"%s\"", op->op_name, op->op_value);
		else {
			if (eq(op->op_name,"GFLOAT"))
				fprintf(ofp, " -Mg");

			else if (eq(op->op_name,"BINARY")) 
				/*
			 	 *  BINARY option specified then set
			 	 *  dontload flag
			 	 */
				dontload++;

			else if (eq(op->op_name,"EMULFLT"))
				/* For the floating emulation */
				emulation_float++;

			fprintf(ofp, " -D%s", op->op_name);
		}

	fprintf(ofp, "\n");
	if (hadtz == 0)
		printf("timezone not specified; gmt assumed\n");
#ifdef vax
	if (maxusers == 0) {
		printf("maxusers not specified; 24 assumed\n");
		maxusers = 24;
	} else if (maxusers < 8) {
		printf("maxusers less than 8 not recommended\n");
	} else if (maxusers > 256) {
		printf("maxusers truncated to 256\n");
		maxusers = 256;
	}
	min_dmmin = CLSIZE * KLMAX; /* min value of dmmin */
	if (dmmin == 0)
		dmmin = min_dmmin;
	else if (min_dmmin > dmmin) {
		printf("dmmin (%d) is too small, set to %d\n", dmmin, min_dmmin);
		dmmin = min_dmmin;
	}
	else if (dmmin % min_dmmin != 0) {
		printf("dmmin (%d) must be a multiple of CLSIZE*KLMAX (%d), dmmin set to %d\n", dmmin, min_dmmin);
		dmmin = min_dmmin;
	}
	if (dmmax == 0)
		dmmax = 1024; /* the old value in init_main.c */
	
		
	if (maxuprc == 0 ) {
		printf("maxuprc not specified; 25 assumed\n");
		maxuprc = 25;
	}
	if (physmem == 0 ) {
		printf("physmem not specified; 2 megabytes assumed\n");
		physmem = 2;
	}
	if (processors == 0 ) {
		printf("processors not specified; 1 assumed\n");
		processors = 1;
	}
#endif
#ifdef sun
	if (maxusers == 0) {
		printf("maxusers not specified; 8 assumed\n");
		maxusers = 8;
	} else if (maxusers < 2) {
		printf("minimum of 2 maxusers assumed\n");
		maxusers = 2;
	} else if (maxusers > 32) {
		printf("maxusers truncated to 32\n");
		maxusers = 32;
	}
#endif
	fprintf(ofp, "PARAM=-DTIMEZONE=%d -DDST=%d -DMAXUSERS=%d -DMAXUPRC=%d -DPHYSMEM=%d -DNCPU=%d -DDMMIN=%d -DDMMAX=%d ",
		timezone, dst, maxusers, maxuprc, physmem,
		processors, dmmin, dmmax);
	if (smmin)
		fprintf(ofp,"-DSMMIN=%d ",smmin);
	if (smmax)
		fprintf(ofp,"-DSMMAX=%d ",smmax);
	if (smbrk)
		fprintf(ofp,"-DSMBRK=%d ",smbrk);
	fprintf(ofp,"\n");
	while (fgets(line, BUFSIZ, ifp) != 0) {
		if (*line == '%')
			goto percent;
		if (profiling && strncmp(line, "COPTS=", 6) == 0) {
			register char *cp;

			fprintf(ofp, 
			    "GPROF.EX=/usr/src/lib/libc/%s/csu/gmon.ex\n",
			    machinename);
			cp = index(line, '\n');
			if (cp)
				*cp = 0;
			cp = line + 6;
			while (*cp && (*cp == ' ' || *cp == '\t'))
				cp++;
			COPTS = malloc((unsigned)(strlen(cp) + 1));
			if (COPTS == 0) {
				printf("config: out of memory\n");
				exit(1);
			}
			strcpy(COPTS, cp);
			fprintf(ofp, "%s -pg\n", line);
			continue;
		}
		fprintf(ofp, "%s", line);
		continue;
	percent:
		if (eq(line, "%OBJS\n"))
			do_objs(ofp);
		else if (eq(line, "%EMULO\n"))
			do_emulo(ofp);
		else if (eq(line, "%EMULS\n"))
			do_emuls(ofp);
		else if (eq(line, "%CFILES\n"))
			do_cfiles(ofp);
		else if (eq(line, "%EMRULES\n"))
			do_emrules(ofp);
		else if (eq(line, "%RULES\n"))
			do_rules(ofp);
		else if (eq(line, "%LOAD\n"))
			do_load(ofp);
		else
			fprintf(stderr,
			    "Unknown %% construct in generic makefile: %s",
			    line);
	}
	(void) fclose(ifp);
	(void) fclose(ofp);
}

/*
 * get next word from stream and error out if EOF
 */

char *
next_word(fp, fname)
FILE *fp;
char *fname;
{
	register char *wd;

	if ((wd = get_word(fp)) == (char *) EOF) {
		printf("%s: Unexpected end of file\n", fname);
		exit(1);
	}
	return(wd);
}

/*
 * Read in the information about files used in making the system.
 * Store it in the ftab linked list.
 */
read_files()
{
	FILE *fp;
	register struct file_list *tp;
	register struct device *dp;
	int sysfile, required, doopts, override, skipopts, negate;
	int noneed, nexneed, failing;
	char fname[32], *wd, *module;

	/*
	 * filename	[ standard | optional ] [ config-dependent ]
	 *	[ dev* [ or dev* ] | profiling-routine ] [ device-driver] 
	 *	[Binary | Notbinary] [ Unsupported ] 
	 */

	ftab = NULL;
	required = 1;
	override = 0;

	/* loop for all file lists we have to process */

	for (sysfile = 0; sysfile < 3; sysfile++) {

		/* select appropriate file */

		switch (sysfile) {

			case 0:
				strcpy(fname, "files");
				break;

			case 1:
				(void) sprintf(fname, "files.%s", machinename);
				break;

			case 2:
				(void) sprintf(fname, "files.%s",
						raise(ident));
				required = 0;
				override = 1;
				break;
		}
		fp = fopen(fname, "r");
		if (fp == NULL) {
			if (required == 0) {
				continue;
			} else {
				perror(fname);
				exit(1);
			}
		}

		/* process each line */

		while ((wd = get_word(fp)) != (char *)EOF) {
			if (wd == NULL) {
				continue;
			}

			/* record and check module name */

			module = ns(wd);
			if (fl_lookup(module)) {
				printf("%s: Duplicate file %s.\n", fname,
					module);
				exit(1);
			}
			if (override != 0) {
				if ((tp = fltail_lookup(module)) != NULL) {
					printf("%s: Local file %s overrides %s.\n",
						fname, module, tp->f_fn);
				}
			} else {
				tp = new_fent();
			}
			tp->f_fn = module;
			tp->f_type = 0;
			tp->f_flags = 0;
			for (nexneed = 0; nexneed < NNEEDS; nexneed++) {
				tp->f_needs[nexneed] = NULL;
			}
			nexneed = -1;

			/* process optional or standard specification */

			wd = get_word(fp);
			if (eq(wd, "optional")) {
				doopts = 1;
			} else if (eq(wd, "standard")) {
				doopts = 0;
			} else {
				printf("%s: %s must be optional or standard\n",
					fname, module);
			}
			wd = next_word(fp, fname);

			/* process config-dependent specification */

			if (eq(wd, "config-dependent")) {
				tp->f_flags |= CONFIGDEP;
				wd = next_word(fp, fname);
			}

			/* process optional specifications */

			failing = 0;
			if (doopts == 1) {
				noneed = 0;
				skipopts = 0;
				negate = 0;
				while (wd != NULL) {
					if (eq(wd, "device-driver")
						|| eq(wd, "profiling-routine")
						|| eq(wd, "Binary")
						|| eq(wd, "Unsupported")
						|| eq(wd, "Notbinary")) {
						break;
					}

					/* if this is or, dependency is met */

					if (eq(wd, "or")) {
						if (negate) {
							printf("%s: %s has unspecified negate\n",
								fname, module);
							exit(1);
						}
						if (failing == 0) {
							skipopts = 1;
						} else {
							skipopts = 0;
							failing = 0;
						}
						wd = next_word(fp, fname);
						continue;
					}

					/* if this is not, next dependency
					   is negated */

					if (eq(wd, "not")) {
						negate = 1;
						wd = next_word(fp, fname);
						continue;
					}

					/* if dependent on cpu, do
					   not build header file */

					if (eq(wd, "cpu")) {
						noneed = 1;
						wd = next_word(fp, fname);
						continue;
					}

					/* if dependent on bus recognize
					   keyword, does nothing now */

					if (eq(wd, "bus")) {
						wd = next_word(fp, fname);
						continue;
					}

					/* process normal dependency */

					if (nexneed < 0) {
						nexneed = 0;
					}
					if (noneed == 0) {
						if (nexneed >= NNEEDS) {
							printf("%s: %s has too many dependencies",
								fname, module);
							exit(1);
						}
						tp->f_needs[nexneed++] =
							ns(wd);
					}
					noneed = 0;

					/* if dependency met, wait for field
					   terminator */

					if (skipopts != 0) {
						wd = next_word(fp, fname);
						continue;
					}

					/* see if dependency is satisfied */

					dp = dtab;
					while (dp != NULL && !eq(wd, dp->d_name)) {
						dp = dp->d_next;
					}
					if ((dp == NULL && negate == 0) ||
						(dp != NULL && negate != 0)) {

						/* flush rest of this line
						   or until we find an "or" */

						failing = 1;
						skipopts = 1;
					}
					wd = next_word(fp, fname);
					negate = 0;
				}

				/* finished with dependencies, error if none
				   or hanging not */

				if (negate) {
					printf("%s: %s has unspecified negate\n",
						fname, module);
					exit(1);
				}
				if (nexneed < 0
					&& !eq(wd, "profiling-routine")) {
					printf("%s: what is %s optional on?\n",
						fname, module);
					exit(1);
				}
			}

			/* is this module to be included? */

			if (failing != 0) {
				tp->f_flags |= INVISIBLE;
			}

			/* handle profiling or device driver spec */

			if (eq(wd, "profiling-routine")) {
				tp->f_type = PROFILING;
				if (profiling == 0) {
					tp->f_flags |= INVISIBLE;
				}
				wd = next_word(fp, fname);
			} else if (eq(wd, "device-driver")) {
				tp->f_type = DRIVER;
				wd = next_word(fp, fname);
			} else {
				tp->f_type = NORMAL;
			}

			/* handle binary or not binary spec */

			if (eq(wd, "Binary")) {
				if (!source) {
					tp->f_flags |= OBJS_ONLY;
				}
				wd = next_word(fp, fname);
			} else if (eq(wd, "Notbinary")) {
				if (source) {
					tp->f_flags |= NOTBINARY;
				}
				wd = next_word(fp, fname);
			}

			/* handle unsupported spec */

			if (eq(wd, "Unsupported")) {
				if (!source) {
					tp->f_flags |= UNSUPPORTED;
				}
				wd = next_word(fp, fname);
			}

			/* if anything left, its a syntax error */

			if (wd != NULL) {
				printf("%s: syntax error describing %s found %s type %d flags 0x%x\n",
					fname, module, wd, tp->f_type,
					tp->f_flags);
				exit(1);
			}
		}
	}
}

/*
 * This routine handles the MicroVAX emulation code object modules. These 
 * routines must be processed in a particular order. This is necessary so
 * that the space they occupy can be mapped as user read.
 */
do_emulo(fp)
	FILE *fp;
{
	static char *objects[]={ 
	" vaxarith.o  vaxcvtpl.o vaxeditpc.o vaxhandlr.o \\\n",
	"	vaxashp.o vaxcvtlp.o vaxdeciml.o vaxemulat.o vaxstring.o \\\n",
	"	vaxconvrt.o ",
	0 };

	char **cptr;
	if ( emulation_instr || emulation_float )
		fprintf(fp, "EMULO= vaxbegin.o ");
	else
		return;

	if ( emulation_instr ) {
		for( cptr = objects ; *cptr ; cptr++ )
			fputs(*cptr, fp);
	}

	if ( emulation_float )
		fprintf(fp, "vaxfloat.o ");

	fprintf(fp,"vaxexception.o vaxend.o\n");
}
do_objs(fp)
	FILE *fp;
{
	register struct file_list *tp, *fl;
	register int lpos, len;
	register char *cp, och, *sp;
	char swapname[32];

	fprintf(fp, "OBJS=");
	lpos = 6;
	for (tp = ftab; tp != 0; tp = tp->f_next) {
		if ((tp->f_flags & INVISIBLE) != 0)
			continue;
		if ((tp->f_flags & NOTBINARY) && source)
		/* do not load object in the BINARY directory */
			continue;
		sp = tail(tp->f_fn);
		for (fl = conf_list; fl; fl = fl->f_next) {
			if (fl->f_type != SWAPSPEC)
				continue;
			if (eq(fl->f_fn, "generic") || eq(fl->f_fn, "boot"))
				sprintf(swapname, "swap.c", fl->f_fn);
			else
				sprintf(swapname, "swap%s.c", fl->f_fn);
			if (eq(sp, swapname))
				goto cont;
		}
		cp = sp + (len = strlen(sp)) - 1;
		och = *cp;
		*cp = 'o';
		if (len + lpos > 72) {
			lpos = 8;
			fprintf(fp, "\\\n\t");
		}
		fprintf(fp, "%s ", sp);
		if (tp->f_flags & UNSUPPORTED)
			printf("Warning this device is not supported by DIGITAL %s\n",sp);
		lpos += len + 1;
		*cp = och;
cont:
		;
	}
	if (lpos != 8)
		putc('\n', fp);
}

/*
 *	This routine outputs the emulation source file names
 */
do_emuls(fp)
	FILE *fp;
{
	static char *sources[]={
	"../emul/vaxarith.s ../emul/vaxashp.s \\\n",
	"	../emul/vaxconvrt.s ../emul/vaxcvtlp.s ../emul/vaxcvtpl.s \\\n",
	"	../emul/vaxdeciml.s ../emul/vaxeditpc.s ../emul/vaxemulat.s \\\n",
	"	../emul/vaxhandlr.s ../emul/vaxstring.s \\\n",
	0};

	char **cptr;

	if( !source ) 
		return;

	if (emulation_instr || emulation_float)
		fprintf(fp,"EMULS= ../emul/vaxbegin.s ");
	else
		return;

	if (emulation_instr) {
		for( cptr=sources ; *cptr ; cptr++ )
			fputs(*cptr, fp);
	}

	if (emulation_float)
		fprintf(fp,"../emul/vaxfloat.s ");

	fprintf(fp,"	../emul/vaxexception.s ../emul/vaxend.s \n");
}

#define MAXFILES	150	/* maximum number of files in CFILES */
do_cfiles(fp)
	FILE *fp;
{
	register struct file_list *tp;
	register int lpos, len, count;

	fprintf(fp, "CFILES=");
	lpos = 8;
	count = 0;
	for (tp = ftab; tp != 0; tp = tp->f_next) {
		if ((tp->f_flags & INVISIBLE) != 0)
			continue;
		if (tp->f_flags & OBJS_ONLY)
			continue;
		if ((tp->f_flags & NOTBINARY) && source)
		/* do not compile in the BINARY directory */
			continue;
		if (tp->f_fn[strlen(tp->f_fn)-1] != 'c')
			continue;
		if ((len = 3 + strlen(tp->f_fn)) + lpos > 72) {
			lpos = 8;
			fprintf(fp, "\\\n\t");
		}
		fprintf(fp, "../%s ", tp->f_fn);
		lpos += len + 1;
		if (++count == MAXFILES) {
			putc('\n\n', fp);
			fprintf(fp,"CFILES1=");
			lpos = 8;
			count = 0;
		}
	}
	if (source)
		fprintf(fp, " ../vax/genassym.c");
	if (lpos != 8)
		putc('\n', fp);
}

char *
tail(fn)
	char *fn;
{
	register char *cp;

	cp = rindex(fn, '/');
	if (cp == 0)
		return (fn);
	return (cp+1);
}

/*
 * Output the emulation code rules. This is really a hack but considering
 * the fact that the code and rules are not optional for microvax and that
 * the rules are special and do not fit with the general case we do it this
 * way.
 */
do_emrules( fp )
	FILE *fp;
{
	char **cptr;
	/*
	 * This is the list of files that make up the emulation code.
	 * Some of the code is dependant on the placement and order
	 * of the first three names.
	 */
	static char *emuls[]={
		"vaxbegin", "vaxend",  "vaxfloat", "vaxarith", "vaxcvtpl",
		"vaxeditpc","vaxhandlr", "vaxashp", "vaxcvtlp", "vaxdeciml",
		"vaxemulat", "vaxstring", "vaxconvrt", "vaxexception",0
		};

	cptr = emuls;
	if( !source ){
		for( ; *cptr ; cptr++ ){
			fprintf(fp, "%s.o : ../BINARY.%s/%s.o\n",
				*cptr, machinename, *cptr);
			fprintf(fp, "	@ln -s ../BINARY.%s/%s.o %s.o\n\n",
				machinename, *cptr, *cptr);
		}
	} else {
		/*
		 * do begin and end
		 */
		fprintf(fp, "%s.o : ../emul/%s.s\n\tcc -c ../emul/%s.s\n\n",
			*cptr, *cptr, *cptr);
		cptr++;
		fprintf(fp, "%s.o : ../emul/%s.s\n\tcc -c ../emul/%s.s\n\n",
			*cptr, *cptr, *cptr);
		cptr++;
		/*
		 * do float
		 */
		fprintf(fp, "%s.o : ../emul/%s.s ../emul/%s.awk\n",
			*cptr, *cptr, *cptr);
		fprintf(fp, "\tawk -f ../emul/%s.awk ../emul/%s.s | /lib/cpp | as -o %s.o -\n",
			*cptr, *cptr, *cptr);
		cptr++;
		/*
		 * Do the rest
		 */
		for( ; *cptr ; cptr++ ) {
			fprintf(fp, "%s.o : ../emul/%s.s\n", *cptr, *cptr);
			fprintf(fp, "\t/lib/cpp ");
			fprintf(fp,"../emul/%s.s | as -o %s.o -\n\n",
				*cptr, *cptr);
		}
	}
}

/*
 * Create the makerules for each file
 * which is part of the system.
 * Devices are processed with the special c2 option -i
 * which avoids any problem areas with i/o addressing
 * (e.g. for the VAX); assembler files are processed by as.
 */
do_rules(f)
	FILE *f;
{
	register char *cp, *np, och, *tp;
	register struct file_list *ftp;
	char *extras;

		
	if(source){
		fprintf(f,"locore.o: assym.s ${AHEADS} ../vax/rpb.s ../vax/scb.s ../vax/locore.s \\\n");
		fprintf(f,"\t../vax/mtpr.h ../vax/trap.h ../machine/psl.h \\\n");
		fprintf(f,"\t../machine/pte.h ../vax/cpu.h mba.h\n");
		fprintf(f,"\tcat assym.s ../vax/locore.s > locore.c\n");
		fprintf(f,"\tcc -E -I. -DLOCORE ${COPTS} locore.c > locore.i\n");
		fprintf(f,"\t@echo 'as -o locore_bin.o $${AHEADS} locore.i'\n");
		fprintf(f,"\t@as -o locore_bin.o ${AHEADS} locore.i\n");
		fprintf(f,"\t@rm locore.i locore.c\n\n");
		fprintf(f,"\t@cat assym.s ../vax/rpb.s ../vax/scb.s > scb.c\n");
		fprintf(f,"\tcc -E -I. -DLOCORE ${COPTS} scb.c > scb.i\n");
		fprintf(f,"\t@echo 'as -o scb.o $${AHEADS} scb.i'\n");
		fprintf(f,"\t@as -o scb.o ${AHEADS} scb.i\n");
		fprintf(f,"\t@rm scb.i scb.c\n\n");
		fprintf(f,"\t@cat assym.s ../vax/spt.s > spt.c\n");
		fprintf(f,"\tcc -E -I. -DLOCORE ${COPTS} spt.c > spt.i\n");
		fprintf(f,"\t@echo 'as -o spt.o $${AHEADS} spt.i'\n");
		fprintf(f,"\t@as -o spt.o ${AHEADS} spt.i\n");
		fprintf(f,"\t@rm spt.i spt.c\n\n");
		fprintf(f,"\t@echo ld -r scb.o locore_bin.o spt.o\n");
		fprintf(f,"\t@ld -r scb.o locore_bin.o spt.o\n");
		fprintf(f,"\t@mv a.out locore.o\n\n");
		fprintf(f,"genassym.o: ../vax/genassym.c\n");
		fprintf(f,"\tcc -c ${CFLAGS} ${PARAM} ../vax/genassym.c\n");
		fprintf(f,"genassym: genassym.o\n");
		fprintf(f,"\tcc -o genassym ${CFLAGS} ${PARAM} genassym.o\n");
		fprintf(f,"assym.s: ../h/param.h ../machine/pte.h ../h/buf.h ../h/vmparam.h \\\n");
		fprintf(f,"\t../h/vmmeter.h ../h/dir.h ../h/cmap.h ../h/map.h ../vaxuba/ubavar.h \\\n");
		fprintf(f,"\t../h/proc.h makefile genassym\n");
		fprintf(f,"\t./genassym %d %d >assym.s\n\n",
			maxusers, physmem);  /*RR - needed -DKERNEL*/
					     /*RR - IDENT changed to CFLAGS*/
		fprintf(f,"../%s/inline/inline: ../%s/inline/inline.h \\\n",
			machinename, machinename);
		fprintf(f,"\t../%s/inline/langpats.c ../%s/inline/libcpats.c \\\n",
			machinename, machinename);
		fprintf(f,"\t../%s/inline/machdep.c ../%s/inline/machpats.c \\\n",
			machinename, machinename);
		fprintf(f,"\t../%s/inline/main.c\n", machinename);
		fprintf(f,"\tcd ../%s/inline; make\n\n", machinename);
	} else {
		fprintf(f,"locore.o: assym.s  ../vax/spt.s ../vax/rpb.s ../vax/scb.s locore_bin.o\n");
		fprintf(f,"\t@cat assym.s ../vax/rpb.s ../vax/scb.s > scb.c\n");
		fprintf(f,"\tcc -E -I. -DLOCORE ${COPTS} scb.c > scb.i\n");
		fprintf(f,"\t@echo 'as -o scb.o $${AHEADS} scb.i'\n");
		fprintf(f,"\t@as -o scb.o ${AHEADS} scb.i\n");
		fprintf(f,"\t@rm scb.i scb.c\n\n");
		fprintf(f,"\tcat assym.s ../vax/spt.s > spt.c\n");
		fprintf(f,"\tcc -E -I. -DLOCORE ${COPTS} spt.c > spt.i\n");
		fprintf(f,"\t@echo 'as -o spt.o $${AHEADS} spt.i'\n");
		fprintf(f,"\t@as -o spt.o ${AHEADS} spt.i\n");
		fprintf(f,"\t@rm spt.i spt.c\n\n");
		fprintf(f,"\t@echo ld -r scb.o locore_bin.o spt.o\n");
		fprintf(f,"\t@ld -r scb.o locore_bin.o spt.o\n");
		fprintf(f,"\t@mv a.out locore.o\n");
		fprintf(f,"locore_bin.o:\n");
		fprintf(f,"\t@ln -s ../BINARY.%s/locore_bin.o locore_bin.o\n\n",
			machinename);
		fprintf(f,"assym.s: makefile\n\t@../BINARY.%s/genassym %d %d > assym.s\n\n", machinename, maxusers, physmem);
	}
		

for (ftp = ftab; ftp != 0; ftp = ftp->f_next) {
	if ((ftp->f_flags & INVISIBLE) != 0)
		continue;
	if ((ftp->f_flags & NOTBINARY) && source)
		continue;
	cp = (np = ftp->f_fn) + strlen(ftp->f_fn) - 1;
	och = *cp;
	*cp = '\0';
	if (ftp->f_flags & OBJS_ONLY) {
	  fprintf(f,"%so: ../BINARY.%s/%so\n\t@ln -s ../BINARY.%s/%so %so\n\n"
			,tail(np),machinename,tail(np),machinename,tail(np),tail(np));
		continue;
	}
	fprintf(f, "%so: ../%s%c\n", tail(np), np, och);
	tp = tail(np);
	if (och == 's') {
		fprintf(f, "\t${AS} -o %so ../%ss\n\n", tp, np);
		continue;
	}
	if (ftp->f_flags & CONFIGDEP)
		extras = "${PARAM} ";
	else
		extras = "";
	switch (ftp->f_type) {

	case NORMAL:
		switch (machine) {

		case MACHINE_VAX:
			fprintf(f, "\t${CC} -I. -c -S ${COPTS} %s../%sc\n",
				extras, np);
			fprintf(f, "\t${C2} %ss | ../%s/inline/inline |",
				tp, machinename);
			fprintf(f, " ${AS} -o %so\n", tp);
			fprintf(f, "\trm -f %ss\n\n", tp);
			break;

		case MACHINE_SUN:
			fprintf(f, "\t${CC} -I. -c -O ${COPTS} %s../%sc\n\n",
				extras, np);
			break;
		}
		break;

	case DRIVER:
		switch (machine) {

		case MACHINE_VAX:
			fprintf(f, "\t${CC} -I. -c -S ${COPTS} %s../%sc\n",
				extras, np);
			fprintf(f,"\t${C2} -i %ss | ../%s/inline/inline |",
				tp, machinename);
			fprintf(f, " ${AS} -o %so\n", tp);
			fprintf(f, "\trm -f %ss\n\n", tp);
			break;

		case MACHINE_SUN:
			fprintf(f, "\t${CC} -I. -c -O ${COPTS} %s../%sc\n\n",
				extras, np);
		}
		break;

	case PROFILING:
		if (!profiling)
			continue;
		if (COPTS == 0) {
			fprintf(stderr,
			    "config: COPTS undefined in generic makefile");
			COPTS = "";
		}
		switch (machine) {

		case MACHINE_VAX:
			fprintf(f, "\t${CC} -I. -c -S %s %s../%sc\n",
				COPTS, extras, np);
			fprintf(f, "\tex - %ss < ${GPROF.EX}\n", tp);
			fprintf(f, "\t../%s/inline/inline %ss | ${AS} -o %so\n",
				machinename, tp, tp);
			fprintf(f, "\trm -f %ss\n\n", tp);
			break;

		case MACHINE_SUN:
			fprintf(stderr,
			    "config: don't know how to profile kernel on sun\n");
			break;
		}
		break;

	default:
		printf("Don't know rules for %s\n", np);
		break;
	}
	*cp = och;
}
}

/*
 * Create the load strings
 */
do_load(f)
	register FILE *f;
{
	register struct file_list *fl;
	int first = 1;
	struct file_list *do_systemspec();

	fl = conf_list;
	while (fl) {
		if (fl->f_type != SYSTEMSPEC) {
			fl = fl->f_next;
			continue;
		}
		fl = do_systemspec(f, fl, first);
		if (first)
			first = 0;
	}
	fprintf(f, "all:");
	for (fl = conf_list; fl != 0; fl = fl->f_next)
		if (fl->f_type == SYSTEMSPEC)
			fprintf(f, " %s", fl->f_needs[0]);
	fprintf(f, "\n");
}

struct file_list *
do_systemspec(f, fl, first)
	FILE *f;
	register struct file_list *fl;
	int first;
{
	int swaptype;
	char *depends;

	if ( emulation_instr || emulation_float )
		depends = "${OBJS} ${EMULO}";
	else
		depends = "${OBJS}";

	fprintf(f, "%s: makefile", fl->f_needs[0]);
	if (machine == MACHINE_VAX) {
		if (source) {
			fprintf(f, " ../%s/inline/inline", machinename);
		}
	}
	fprintf(f, " locore.o ubglue.o \\\n\t%s param.o", depends);
	if (source)
		fprintf(f, " ioconf.o swap.o");
	else
		fprintf(f, " ioconf.o swapgen%s.o", fl->f_needs[0]);
	if (!eq(fl->f_fn, "generic") && !eq(fl->f_fn, "boot"))
		fprintf(f, " swap%s.o", fl->f_fn);  /*RR*/
	if (dontload) fprintf(f, "\n");   /*RR*/
	/*
	 *	Don't load if the BINARY option has been specified
	 */
	if (!dontload ) { /* we are loading */
		/* first define a load tag on the end of vmunix */
		/* and define that symbol as the actual load commands */
		/* now we can say "make vmunix" to get the old style loads */
		/* with all the dependancy checking */
		/* or we can say "make load" and we execute the load commands */
		fprintf(f, " load%s\n\nload%s:\n",
			fl->f_needs[0], fl->f_needs[0]);   /*RR*/
		fprintf(f, "\t@echo loading %s\n\t@rm -f %s\n",
			fl->f_needs[0], fl->f_needs[0]);
		if (first) {
			fprintf(f, "\t@/bin/sh ../conf/newvers.sh\n"); /*RR*/
			fprintf(f, "\t@${CC} $(CFLAGS) -c vers.c\n");
		}
		switch (machine) {

		case MACHINE_VAX:
			fprintf(f,"\t@${LD} -n -o %s -e start -x -T 80000000 ",
				fl->f_needs[0]);
			break;

		case MACHINE_SUN:
			fprintf(f, "\t@${LD} -o %s -e start -x -T 4000 ",
				fl->f_needs[0]);
			break;
		}
		fprintf(f, "locore.o ubglue.o \\\n\t\t%s vers.o ioconf.o param.o swap.o ", depends);
		if (!eq(fl->f_fn, "generic") && !eq(fl->f_fn, "boot"))
			fprintf(f, "swap%s.o\n", fl->f_fn);
		else
			fprintf(f, "\n");
		fprintf(f, "\t@echo rearranging symbols\n");
		fprintf(f, "\t@-symorder ../%s/symbols.sort %s\n",
			machinename, fl->f_needs[0]);
		fprintf(f, "\t@size %s\n", fl->f_needs[0]);
		fprintf(f, "\t@chmod 755 %s\n\n", fl->f_needs[0]);
	} else 
		fprintf(f, "\t@echo Binary build done\n\n");

	swaptype = 0;
	if (eq(fl->f_fn, "generic"))
		swaptype = 1;
	else if (eq(fl->f_fn, "boot"))
			swaptype = 2;
	if (!source) {
		fprintf(f, "swapgen%s.o:\n", fl->f_needs[0]);
		fprintf(f, "\t${CC} -I. -c -S ${COPTS} -DSWAPTYPE=%d ../%s/swap.c\n", swaptype, machinename);
		fprintf(f, "\t${C2} swap.s | ../%s/inline/inline", machinename);
		fprintf(f, " | ${AS} -o swap.o\n");
		fprintf(f, "\trm -f swap.s\n\n");
	}
	do_swapspec(f, fl->f_fn);
	for (fl = fl->f_next; fl->f_type == SWAPSPEC; fl = fl->f_next)
		;
	return (fl);
}

do_swapspec(f, name)
	FILE *f;
	register char *name;
{
	if ((eq(name, "generic") || eq(name, "boot")) && source) {
		fprintf(f, "swap.o: ../%s/swap.c\n", machinename);
		switch (machine) {

		case MACHINE_VAX:
			fprintf(f, "\t${CC} -I. -c -S ${COPTS} ");
			fprintf(f, "../%s/swap.c\n", machinename);
			fprintf(f, "\t${C2} swap.s | ../%s/inline/inline", machinename);
			fprintf(f, " | ${AS} -o swap.o\n");
			fprintf(f, "\trm -f swap.s\n\n");
			break;

		case MACHINE_SUN:
			fprintf(f, "\t${CC} -I. -c -O ${COPTS} ");
			fprintf(f, "../%s/swap.c\n\n", machinename);
			break;
		}
	} else
	  if (!eq(name, "generic") && !eq(name, "boot")) {
			fprintf(f, "swap%s.o: swap%s.c\n", name, name);
			fprintf(f, "\t${CC} -I. -c -O ${COPTS} swap%s.c\n\n", name);
		}
}

char *
raise(str)
	register char *str;
{
	register char *cp = str;

	while (*str) {
		if (islower(*str))
			*str = toupper(*str);
		str++;
	}
	return (cp);
}
