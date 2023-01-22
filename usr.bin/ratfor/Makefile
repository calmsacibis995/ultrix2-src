# @(#)Makefile	1.3	ULTRIX	12/4/84
#
#	@(#)Makefile	1.1 (Berkeley) 12/15/82

CFLAGS = -O
DESTROOT=

ratfor:	r0.o r1.o r2.o rio.o rlook.o rlex.o y.tab.o
	cc r*.o y.tab.o -o ratfor

r0.o:	r.h y.tab.h r0.c
r1.o:	r.h y.tab.h r1.c
r2.o:	r.h y.tab.h r2.c
rio.o:	r.h y.tab.h rio.c
rlook.o:	r.h y.tab.h rlook.c
rlex.o:	r.h y.tab.h rlex.c
y.tab.c:	r.g
	yacc -d r.g
y.tab.h:	r.g
	yacc -d r.g
install: ratfor
	install -c -s ratfor $(DESTROOT)/usr/bin
clean:
	rm -f y.tab.c *.o ratfor y.tab.h

clobber: clean
	rm -f *.[cohg] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
