#ifndef lint
static	char	*sccsid = "@(#)rules.c	1.1	(ULTRIX)	3/20/86";
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
/*
 *
 *   Modification History:
 *
 *
 */

#include "defs"
/* DEFAULT RULES FOR UNIX */

/*
 *	These are the internal rules that "make" trucks around with it at
 *	all times. One could completely delete this entire list and just
 *	conventionally define a global "include" makefile which had these
 *	rules in it. That would make the rules dynamically changeable
 *	without recompiling make. This file may be modified to local
 *	needs. There are currently two versions of this file with the
 *	source; namely, rules.c (which is the version running in Columbus)
 *	and pwbrules.c which is my attempt at satisfying the requirements
 *	of PWB systems.
 *	The makefile for make (make.mk) is parameterized for a different
 *	rules file. The macro $(RULES) defined in "make.mk" can be set
 *	to another file and when "make" is "made" the procedure will
 *	use the new file. The recommended way to do this is on the
 *	command line as follows:
 *		"make -f make.mk RULES=pwbrules"
 */

CHARSTAR builtin[] =
	{
/* orig sys V
	".SUFFIXES: .o .c .c~ .y .y~ .l .l~ .s .s~ .sh .sh~ .h .h~",
*/
/* these suffixes handle 4.3 and sys V */
".SUFFIXES: .out .o .c .c~ .F .F~ .f .f~ .e .e~ .r .r~ .y .y~ .l .l~ .s .s~ .sh .sh~ .h .h~ .p .p~",

/* PRESET VARIABLES */
	"MAKE=make",
	"YACC=yacc",
	"YFLAGS=",
	"LEX=lex",
	"LFLAGS=",
	"LD=ld",
	"LDFLAGS=",
	"CC=cc",
	"CFLAGS=-O",
	"AS=as",
	"ASFLAGS=",
	"GET=get",
	"GFLAGS=",
/* these added for 4.3 compatibility */
	"PC=pc",
	"PFLAGS=-O",
	"RC=f77",
	"RFLAGS=-O",
	"EC=f77",
	"EFLAGS=-O",
	"FC=f77",
	"FFLAGS=-O",
	"LOADLIBES=",
/* end of 4.3 additions */

/* SINGLE SUFFIX RULES */
	".c:",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@",
	".c~:",
		"\t$(GET) $(GFLAGS) -p $< > $*.c",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) $*.c -o $*",
		"\t-rm -f $*.c",
	".sh:",
		"\tcp $< $@; chmod 0777 $@",
	".sh~:",
		"\t$(GET) $(GFLAGS) -p $< > $*.sh",
		"\tcp $*.sh $*; chmod 0777 $@",
		"\t-rm -f $*.sh",
/* begin additions for 4.3 compat */
	".F:",
		"\t$(FC) $(FFLAGS) $(LDFLAGS) $< -o $@",
	".F~:",
		"\t$(GET) $(GFLAGS) -p $< > $*.F",
		"\t$(FC) $(FFLAGS) $(LDFLAGS) $*.F -o $*",
		"\t-rm -f $*.F $*.o",
	".f:",
		"\t$(FC) $(FFLAGS) $(LDFLAGS) $< -o $@",
	".f~:",
		"\t$(GET) $(GFLAGS) -p $< > $*.f",
		"\t$(FC) $(FFLAGS) $(LDFLAGS) $*.f -o $*",
		"\t-rm -f $*.f $*.o",
	".r:",
		"\t$(RC) $(RFLAGS) $(LDFLAGS) $< -o $@",
	".r~:",
		"\t$(GET) $(GFLAGS) -p $< > $*.r",
		"\t$(RC) $(RFLAGS) $(LDFLAGS) $*.r -o $*",
		"\t-rm -f $*.r $*.o",
	".e:",
		"\t$(EC) $(EFLAGS) $(LDFLAGS) $< -o $@",
	".e~:",
		"\t$(GET) $(GFLAGS) -p $< > $*.e",
		"\t$(EC) $(EFLAGS) $(LDFLAGS) $*.e -o $*",
		"\t-rm -f $*.e $*.o",
	".p:",
		"\t$(PC) $(PFLAGS) $(LDFLAGS) $< -o $@",
	".p~:",
		"\t$(GET) $(GFLAGS) -p $< > $*.p",
		"\t$(PC) $(PFLAGS) $(LDFLAGS) $*.p -o $*",
		"\t-rm -f $*.p $*.o",
	".l:",
		"\t$(LEX) $(LFLAGS) $<",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) lex.yy.c -ll -o $@",
		"\t-rm lex.yy.c",
	".l~:",
		"\t$(GET) $(GFLAGS) -p $< > $*.l",
		"\t$(LEX) $(LFLAGS) $*.l",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) lex.yy.c -ll -o $*",
		"\t-rm lex.yy.c $*.l",
	".y:",
		"\t$(YACC) $(YFLAGS) $<",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) y.tab.c -ly -o $@",
		"\t-rm y.tab.c",
	".y~:",
		"\t$(GET) $(GFLAGS) -p $< > $*.y",
		"\t$(YACC) $(YFLAGS) $*.y",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) y.tab.c -ly -o $*",
		"\t-rm y.tab.c $*.y",
	".s:",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@",
	".s~:",
		"\t$(GET) $(GFLAGS) -p $< > $*.s",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) $*.s -o $*",
		"\t-rm $*.s",
/* end additions for 4.3 compat */

/* DOUBLE SUFFIX RULES */
	".c.o:",
		"\t$(CC) $(CFLAGS) -c $<",
	".c~.o:",
		"\t$(GET) $(GFLAGS) -p $< > $*.c",
		"\t$(CC) $(CFLAGS) -c $*.c",
		"\t-rm -f $*.c",
	".c~.c:",
		"\t$(GET) $(GFLAGS) -p $< > $*.c",
	".s.o:",
		"\t$(AS) $(ASFLAGS) -o $@ $<",
	".s~.o:",
		"\t$(GET) $(GFLAGS) -p $< > $*.s",
		"\t$(AS) $(ASFLAGS) -o $*.o $*.s",
		"\t-rm -f $*.s",
	".y.o:",
		"\t$(YACC) $(YFLAGS) $<",
		"\t$(CC) $(CFLAGS) -c y.tab.c",
		"\trm y.tab.c",
		"\tmv y.tab.o $@",
	".y~.o:",
		"\t$(GET) $(GFLAGS) -p $< > $*.y",
		"\t$(YACC) $(YFLAGS) $*.y",
		"\t$(CC) $(CFLAGS) -c y.tab.c",
		"\trm -f y.tab.c $*.y",
		"\tmv y.tab.o $*.o",
	".l.o:",
		"\t$(LEX) $(LFLAGS) $<",
		"\t$(CC) $(CFLAGS) -c lex.yy.c",
		"\trm lex.yy.c",
		"\tmv lex.yy.o $@",
	".l~.o:",
		"\t$(GET) $(GFLAGS) -p $< > $*.l",
		"\t$(LEX) $(LFLAGS) $*.l",
		"\t$(CC) $(CFLAGS) -c lex.yy.c",
		"\trm -f lex.yy.c $*.l",
		"\tmv lex.yy.o $*.o",
	".y.c :",
		"\t$(YACC) $(YFLAGS) $<",
		"\tmv y.tab.c $@",
	".y~.c :",
		"\t$(GET) $(GFLAGS) -p $< > $*.y",
		"\t$(YACC) $(YFLAGS) $*.y",
		"\tmv y.tab.c $*.c",
		"\t-rm -f $*.y",
	".l.c :",
		"\t$(LEX) $(LFLAGS) $<",
		"\tmv lex.yy.c $@",
	".c.a:",
		"\t$(CC) -c $(CFLAGS) $<",
		"\tar rv $@ $*.o",
		"\trm -f $*.o",
	".c~.a:",
		"\t$(GET) $(GFLAGS) -p $< > $*.c",
		"\t$(CC) -c $(CFLAGS) $*.c",
		"\tar rv $@ $*.o",
		"\trm -f $*.[co]",
	".s~.a:",
		"\t$(GET) $(GFLAGS) -p $< > $*.s",
		"\t$(AS) $(ASFLAGS) -o $*.o $*.s",
		"\tar rv $@ $*.o",
		"\t-rm -f $*.[so]",
	".h~.h:",
		"\t$(GET) $(GFLAGS) -p $< > $*.h",

/* begin additions for 4.3 compat */
	".s.a:",
		"\t$(AS) $(ASFLAGS) -o $*.o $<",
		"\tar rv $@ $*.o",
		"\t-rm -f $*.o",
	".s~.s:",
		"\t$(GET) $(GFLAGS) -p $< > $*.s",
	".y~.y:",
		"\t$(GET) $(GFLAGS) -p $< > $*.y",
	".l~.l:",
		"\t$(GET) $(GFLAGS) -p $< > $*.l",
	".l~.c:",
		"\t$(GET) $(GFLAGS) -p $< > $*.l",
		"\t$(LEX) $(LFLAGS) $*.l",
		"\tmv lex.yy.c $@",
		"\t-rm $*.l",
	".p~.p:",
		"\t$(GET) $(GFLAGS) -p $< > $*.p",
	".p.o:",
		"\t$(PC) $(PFLAGS) -c $<",
	".p~.o:",
		"\t$(GET) $(GFLAGS) -p $< > $*.p",
		"\t$(PC) $(PFLAGS) -c $*.p",
		"\t-rm $*.p",
	".F~.F:",
		"\t$(GET) $(GFLAGS) -p $< > $*.F",
	".F.o:",
		"\t$(FC) $(FFLAGS) -c $<",
	".F~.o:",
		"\t$(GET) $(GFLAGS) -p $< > $*.F",
		"\t$(FC) $(FFLAGS) -c $*.F",
		"\t-rm $*.F",
	".f~.f:",
		"\t$(GET) $(GFLAGS) -p $< > $*.f",
	".f.o:",
		"\t$(FC) $(FFLAGS) -c $<",
	".f~.o:",
		"\t$(GET) $(GFLAGS) -p $< > $*.f",
		"\t$(FC) $(FFLAGS) -c $*.f",
		"\t-rm $*.f",
	".e~.e:",
		"\t$(GET) $(GFLAGS) -p $< > $*.e",
	".e.o:",
		"\t$(EC) $(EFLAGS) -c $<",
	".e~.o:",
		"\t$(GET) $(GFLAGS) -p $< > $*.e",
		"\t$(EC) $(EFLAGS) -c $*.e",
		"\t-rm $*.e",
	".r~.r:",
		"\t$(GET) $(GFLAGS) -p $< > $*.r",
	".r.o:",
		"\t$(RC) $(RFLAGS) -c $<",
	".r~.o:",
		"\t$(GET) $(GFLAGS) -p $< > $*.r",
		"\t$(RC) $(RFLAGS) -c $*.r",
		"\t-rm $*.r",
/* this junk following with the ".out" is superceded by the new make */
/* single suffix rules: from file.f we can say "make file" now */
	".o.out:",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) $< $(LOADLIBES) -o $@",
	".c.out:",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) $< $(LOADLIBES) -o $@",
	".c~.out:",
		"\t$(GET) $(GFLAGS) -p $< > $*.c",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) $*.c $(LOADLIBES) -o $@",
		"\t-rm $*.c",
	".s.out:",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) $< $(LOADLIBES) -o $@",
	".s~.out:",
		"\t$(GET) $(GFLAGS) -p $< > $*.s",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) $*.s $(LOADLIBES) -o $@",
		"\t-rm $*.s",
	".f.out:",
		"\t$(FC) $(FFLAGS) $(LDFLAGS) $< $(LOADLIBES) -o $@",
		"\t-rm $*.o",
	".f~.out:",
		"\t$(GET) $(GFLAGS) -p $< > $*.f",
		"\t$(FC) $(FFLAGS) $(LDFLAGS) $*.f $(LOADLIBES) -o $@",
		"\t-rm $*.[fo]",
	".F.out:",
		"\t$(FC) $(FFLAGS) $(LDFLAGS) $< $(LOADLIBES) -o $@",
		"\t-rm $*.o",
	".F~.out:",
		"\t$(GET) $(GFLAGS) -p $< > $*.F",
		"\t$(FC) $(FFLAGS) $(LDFLAGS) $*.F $(LOADLIBES) -o $@",
		"\t-rm $*.[Fo]",
	".r.out:",
		"\t$(RC) $(RFLAGS) $(LDFLAGS) $< $(LOADLIBES) -o $@",
		"\t-rm $*.o",
	".r~.out:",
		"\t$(GET) $(GFLAGS) -p $< > $*.r",
		"\t$(RC) $(RFLAGS) $(LDFLAGS) $*.r $(LOADLIBES) -o $@",
		"\t-rm $*.[ro]",
	".e.out:",
		"\t$(EC) $(EFLAGS) $(LDFLAGS) $< $(LOADLIBES) -o $@",
		"\t-rm $*.o",
	".e~.out:",
		"\t$(GET) $(GFLAGS) -p $< > $*.e",
		"\t$(EC) $(EFLAGS) $(LDFLAGS) $*.e $(LOADLIBES) -o $@",
		"\t-rm $*.[eo]",
	".y.out:",
		"\t$(YACC) $(YFLAGS) $<",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) y.tab.c $(LOADLIBES) -ly -o $@",
		"\t-rm y.tab.c",
	".y~.out:",
		"\t$(GET) $(GFLAGS) -p $< > $*.y",
		"\t$(YACC) $(YFLAGS) $*.y",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) y.tab.c $(LOADLIBES) -ly -o $@",
		"\t-rm $*.y y.tab.c",
	".l.out:",
		"\t$(LEX) $(LFLAGS) $<",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) lex.yy.c $(LOADLIBES) -ll -o $@",
		"\t-rm lex.yy.c",
	".l~.out:",
		"\t$(GET) $(GFLAGS) -p $< > $*.l",
		"\t$(LEX) $(LFLAGS) $*.l",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) lex.yy.c $(LOADLIBES) -ll -o $@",
		"\t-rm $*.l lex.yy.c",
/* end of compat junk */
	"Rules:",
		"\t-@sh -c \"$(MAKE) -fp - 2\>/dev/null \</dev/null\";exit 0",
/* end additions for 4.3 compat */

	"markfile.o:	markfile",
		"\tA=@;echo \"static char _sccsid[] = \\042`grep $$A'(#)' markfile`\\042;\" > markfile.c",
		"\tcc -c markfile.c",
		"\trm -f markfile.c",
	0 };
