# @(#)Makefile	1.7	ULTRIX 	2/8/85
#
#	@(#)Makefile	4.2	(Berkeley)	83/02/11
#
DESTROOT=
CFLAGS=-O -DWORD32
SRCS = Makefile dextern files yaccpar \
	y1.c y2.c y3.c y4.c \
	yaccdiffs yaccnews

all: yacc

yacc: y1.o y2.o y3.o y4.o
	$(CC) -o yacc y?.o 
y1.o: dextern files yaccpar.h
y2.o y3.o y4.o: dextern files 
install: all
	install -c -s yacc $(DESTROOT)/usr/bin
	install -c yaccpar $(DESTROOT)/usr/lib

tools1: all
	install -c -s yacc $(DESTROOT)/usr/bin
	install -c yaccpar $(DESTROOT)/usr/lib

tools2: all
	install -c -s yacc $(DESTROOT)/usr/bin
	install -c yaccpar $(DESTROOT)/usr/lib

clean :
	-rm -f *.o yacc

clobber: clean
	rm -f *.[ch] yacc* dextern files Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

srcs: $(SRCS)
$(SRCS):
	sccs get $@
