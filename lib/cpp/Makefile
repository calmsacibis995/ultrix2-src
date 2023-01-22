#  @(#)Makefile	1.11	ULTRIX	10/3/86
#
#	Makefile	1.2	83/07/02
#

CFLAGS=-O -Dunix=1 -Dbsd4_2=1 -Dultrix=1 -DFLEXNAMES
DESTROOT=

cpp :	cpp.o cpy.o rodata.o
	$(CC) $(CFLAGS) -o cpp rodata.o cpp.o cpy.o  

cpy.c : cpy.y yyfix
	yacc cpy.y
	/bin/sh ../pcc/:yyfix yyexca yyact yypact yypgo yyr1 yyr2 yychk yydef
	mv y.tab.c cpy.c
cpy.o :	cpy.c yylex.c
rodata.o : cpy.c
	$(CC) $(CFLAGS) -R -c rodata.c

yyfix : yyfix
	-(cd ../pcc;if test -f :yyfix -a -w :yyfix ; then chmod 555 :yyfix; fi; exit 0)
	touch yyfix
clean :
	rm -f *.s *.o cpy.c rodata.c errs yyfix

clobber: clean
	rm -f README Makefile *.[cy]

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

install : cpp
	install -s -c cpp $(DESTROOT)/lib

sources : cpp.c cpy.y yylex.c README
cpp.c cpy.y yylex.c README :
	sccs get $@
