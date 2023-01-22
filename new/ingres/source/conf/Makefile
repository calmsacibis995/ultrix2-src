#		@(#)Makefile	1.4	(ULTRIX)	2/13/85
#
#


INGRES=	../..
H=	../h
LIB=	$(INGRES)/lib
BIN=	$(INGRES)/bin
CPUTYPE=VAX
COMMON= monitor sysdump
PDPALL=	parser qrymod decomp alldbu 
VAXALL=	vaxingres 
SRCS=	monitor.c parser.c qrymod.c decomp.c alldbu.c vaxingres.c \
	sysdump.c version.c

#
#  Libraries
#
#	Libraries should be loaded in the order listed below.
#
CTLMOD=	../ctlmod/ctlmod.a
IUTIL=	../iutil/iutil.a
GUTIL=	../gutil/gutil.a
MONITOR=../monitor/monitor.a
PARSER=	../parser/parser.a
QRYMOD=	../qrymod/qrymod.a
DECOMP=	../decomp/decomp.a
OVQP=	../ovqp/ovqp.a
DBULIB=	../dbu/dbulib.a
MATH=	-lm

ALLLIB=	$(CTLMOD) $(IUTIL) $(GUTIL)
ALLLIBS=$(PARSER) $(QRYMOD) $(DECOMP) $(OVQP) $(DBULIB) $(ALLLIB)

#
#  Assorted commands & other garbage
#
CFLAGS=	-I$H -O
LDFLAGS=-z
CHMOD=	chmod
BINMODE=751
SUMODE=	4740
PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Configuration Tables
MFLAGS=	-k
INSTALL=install

#
#	Recompile absolutely everything
#
all:	${COMMON} all${CPUTYPE}
allVAX:	${VAXALL}
allPDP:	${PDPALL}

install: all install${CPUTYPE}
	${INSTALL} -c -m ${SUMODE}  -o ingres sysdump ${BIN}
	${INSTALL} -c -m ${BINMODE} -o ingres monitor ${BIN}

installVAX:
	${INSTALL} -c -m ${SUMODE} -o ingres vaxingres ${BIN}

installPDP:
	${INSTALL} -c -m ${SUMODE} -o ingres parser ${BIN}
	${INSTALL} -c -m ${SUMODE} -o ingres qrymod ${BIN}
	${INSTALL} -c -m ${SUMODE} -o ingres decomp ${BIN}
	${INSTALL} -c -m ${SUMODE} -o ingres alldbu ${BIN}
#
#  And the processes....
#
#	Proper loading order is:
#		conf table
#		library(s) for modules
#		ctlmod library
#		iutil library
#		gutil library
#

sysdump: version.o sysdump.o $(CTLMOD) $(ALLLIB)
	$(CC) $(LDFLAGS) version.o $@.o $(OTHERS) $(ALLLIB) -o $@

monitor: version.o monitor.o $(MONITOR) $(CTLMOD) $(IUTIL) $(GUTIL)
	$(CC) $(LDFLAGS) version.o $@.o $(OTHERS) $(MONITOR) $(CTLMOD) \
		$(IUTIL) $(GUTIL) -o $@

parser: version.o parser.o $(PARSER) $(ALLLIB)
	$(CC) $(LDFLAGS) version.o $@.o $(OTHERS) $(PARSER) $(ALLLIB) \
		$(MATH) -o $@

qrymod: version.o qrymod.o $(QRYMOD) $(ALLLIB)
	$(CC) $(LDFLAGS) version.o $@.o $(OTHERS) $(QRYMOD) $(ALLLIB) \
		$(MATH) -o $@

decomp: version.o decomp.o $(DECOMP) $(OVQP) $(ALLLIB)
	$(CC) $(LDFLAGS) version.o $@.o $(OTHERS) $(DECOMP) $(OVQP) \
		$(ALLLIB) $(MATH) -o $@

alldbu: version.o alldbu.o $(DBULIB) $(ALLLIB)
	$(CC) $(LDFLAGS) version.o $@.o $(OTHERS) $(DBULIB) $(ALLLIB) \
		$(MATH) -o $@

vaxingres: version.o vaxingres.o $(ALLLIBS)
	$(CC) $(LDFLAGS) version.o $@.o $(OTHERS) $(ALLLIBS) $(MATH) -o $@


new:	new${CPUTYPE} 
	rm -f ${COMMON}
newPDP:
	rm -f $(PDPALL)
newVAX:
	rm -f $(VAXALL)

print: sources
	$(PR) Makefile *.[csy]

vgrind: sources
	cp /dev/null index
	$(VGRIND) -h "$(HEADER) (Release 7)" -n Makefile
	$(VGRIND) -h "$(HEADER) (Release 7)" *.[csy]
	$(VGRIND) -h "$(HEADER) (Release 7)" -x index

clean: new
	rm -f *.o
	rm -f a.out core

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

monitor.o:$H/func.h
parser.o:$H/ingres.h
parser.o:$H/aux.h
parser.o:$H/access.h
parser.o:$H/func.h
qrymod.o:$H/func.h
decomp.o:$H/func.h
alldbu.o:$H/func.h
vaxingres.o:$H/ingres.h
vaxingres.o:$H/aux.h
vaxingres.o:$H/access.h
vaxingres.o:$H/func.h
sysdump.o:$H/func.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
