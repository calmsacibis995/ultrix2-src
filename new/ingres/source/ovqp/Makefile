#		@(#)Makefile	1.4	(ULTRIX)	2/13/85
#
#  Makefile for INGRES One Variable Query Processor (OVQP)
#
#	David L Ballenger, 16-Jan-1984
# 001	Integrate into ULTRIX build procedures.
#

INGRES=	../..
H=	../h
LIB=	$(INGRES)/lib

CFLAGS=	-O -I$H
AR=	-ar
ARFLAGS=rvu
RANLIB=	ranlib
LDFLAGS=-z
LIBMODE=664
BINMODE=4740
PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	One Variable Query Processor
INSTALL=install

CTLMOD=	../ctlmod/ctlmod.a
IUTIL=	../iutil/iutil.a
GUTIL=	../gutil/gutil.a

OBJS=	scan.o interp.o startovqp.o\
	strategy.o findsimps.o key.o string.o typecheck.o interp1.o \
	equelpr.o prsym.o tidtest.o
SRCS=	scan.c interp.c startovqp.c\
	strategy.c findsimps.c key.c string.c typecheck.c interp1.c \
	equelpr.c prsym.c tidtest.c\
	ovqp.h strategy.h

LIBS=	$(CTLMOD) $(IUTIL) $(GUTIL)

all:	ovqp.a

install: all

ovqp.a: $(OBJS)
	$(AR) rvu ovqp.a $?
	${RANLIB} ovqp.a

ovqp:	$(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o ovqp
	$(CHMOD) $(BINMODE) ovqp

new:
	rm -f ovqp.a

clean:  new
	rm -f *.o
	rm -f a.out core ovqp

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

sources: $(SRCS)

#$(SRCS):
#	sccs get $@

save_version:
	@sccs get -s SCCS
	@sccs what ${FILES} | awk '$$2 != "" {print $$1, $$2}' > SCCS/version_${VERSION}

restore_version:
	@echo ${PATH}
	@for i in ${FILES}; do \
		echo "Restore" `grep $$i SCCS/version_${VERSION}`; \
		sccs get -s -r`grep $$i SCCS/version_${VERSION} | awk '{print $$2, $$1}'`; \
	done

show_current:
	@echo ${PATH}
	@sccs what ${FILES} | awk '$$2 != "" {printf "%s  %s\t", $$1, $$2}'
	@echo

print: sources
	$(PR) Makefile TraceFlags *.[hm] *.[csy]

vgrind: sources
	cp /dev/null index
	$(VGRIND) -h "$(HEADER) (Release 7)" -n Makefile TraceFlags
	$(VGRIND) -h "$(HEADER) (Release 7)" *.[hm] *.[csy]
	$(VGRIND) -h "$(HEADER) (Release 7)" -x index

ncheck: 
	ckdir -c

backup:  ncheck
	ar vu backup.a *.[chys]

check:
	ckdir

depend:
	grep '^#[ 	]*include' $(SRCS) | sed -n -f ../h/depend.sed > makedep
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it

scan.o:$H/ingres.h
scan.o:$H/symbol.h
scan.o:$H/tree.h
scan.o:../decomp/globs.h
interp.o:$H/ingres.h
interp.o:$H/aux.h
interp.o:$H/symbol.h
interp.o:$H/tree.h
interp.o:$H/access.h
interp.o:../decomp/globs.h
startovqp.o:$H/ingres.h
startovqp.o:$H/symbol.h
startovqp.o:$H/tree.h
startovqp.o:$H/aux.h
startovqp.o:../decomp/globs.h
startovqp.o:../ctlmod/pipes.h
strategy.o:$H/ingres.h
strategy.o:$H/aux.h
strategy.o:$H/catalog.h
strategy.o:$H/symbol.h
strategy.o:$H/tree.h
strategy.o:../decomp/globs.h
strategy.o:strategy.h
findsimps.o:$H/ingres.h
findsimps.o:$H/symbol.h
findsimps.o:$H/tree.h
findsimps.o:../decomp/globs.h
findsimps.o:strategy.h
key.o:$H/ingres.h
key.o:$H/aux.h
key.o:$H/symbol.h
key.o:$H/tree.h
key.o:../decomp/globs.h
key.o:strategy.h
string.o:$H/ingres.h
string.o:$H/aux.h
string.o:$H/symbol.h
string.o:$H/tree.h
string.o:../decomp/globs.h
typecheck.o:$H/ingres.h
typecheck.o:$H/aux.h
typecheck.o:$H/tree.h
typecheck.o:$H/symbol.h
typecheck.o:../decomp/globs.h
interp1.o:$H/ingres.h
interp1.o:$H/aux.h
interp1.o:$H/symbol.h
interp1.o:$H/tree.h
interp1.o:../decomp/globs.h
equelpr.o:$H/ingres.h
equelpr.o:$H/aux.h
equelpr.o:$H/symbol.h
equelpr.o:$H/tree.h
equelpr.o:../ctlmod/pipes.h
equelpr.o:../decomp/globs.h
prsym.o:$H/ingres.h
prsym.o:$H/aux.h
prsym.o:$H/symbol.h
prsym.o:$H/tree.h
prsym.o:../decomp/globs.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
