#		@(#)Makefile	1.6	(ULTRIX)	2/15/85
#
#  Makefile for INGRES support routines
#
#	David L Ballenger, 16-Jan-1985
# 001	Integrate into ULTRIX build procedures
#

INGRES=	../..
H=	../h
LIB=	$(INGRES)/lib
BIN=	$(INGRES)/bin

ALL=	copydb creatdb destroydb helpr printr purge sysmod \
	restore usersetup ingres printadmin demodb.sh \
	ingres_lock
SRCS=	creatdb.c destroydb.c error.c helpr.c ingres.y \
	printr.c purge.c subs.c sysmod.c usersetup.c \
	indconv.q restore.c copydb.q prvect.c printadmin.c \
	demodb.sh initsocket.c ildr.c

CTLMOD=	../ctlmod/ctlmod.a
IUTIL=	../iutil/iutil.a
GUTIL=	../gutil/gutil.a
DBULIB=	../dbu/dbulib.a
LIBS1=	$(GUTIL)
LIBS2=	$(IUTIL) $(GUTIL)
LIBS3=	$(DBULIB) $(IUTIL) $(GUTIL)
LIBQ=	../libq/libq.a

CFLAGS=	-O -I$H 
EQUEL=	../equel/equel
LDFLAGS=-z
BINMODE=751
SUMODE=	4751
SUIMAGES= creatdb destroydb helpr printr purge restore sysmod ingres
SHMODE=	755
PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Support Programs
INSTALL=install

.SUFFIXES: .q

.q.c:
	$(EQUEL) $<

all: ${ALL}

install: all
	${INSTALL} -c -m ${SUMODE}  -o ingres printadmin  $(BIN)
	${INSTALL} -c -m 700        -o ingres usersetup   $(BIN)
	${INSTALL} -c -m ${SHMODE}  -o ingres demodb.sh   $(BIN)/demodb
	for i in copydb ingres_lock ; do\
		(${INSTALL} -c -m ${BINMODE} -o ingres $$i ${BIN};\
		) ; done
	for i in ${SUIMAGES} ; do\
		(${INSTALL} -c -m ${SUMODE} -o ingres $$i ${BIN};\
		) ; done

copydb.c:  copydb.q
indconv.c: indconv.q

copydb:	copydb.o $(LIBS1) ${LIBQ}
	$(CC) $(LDFLAGS) copydb.o $(LIBS1) $(LIBQ) -o copydb

copydb5.c: copydb5.q
	equel5 copydb5.q

copydb5: copydb5.o $(LIBS2)
	$(CC) $(LDFLAGS) copydb5.o $(LIBS2) -lq5 -o copydb5

creatdb: creatdb.o error.o prvect.o $(LIBS3)
	$(CC) $(LDFLAGS) creatdb.o error.o prvect.o $(LIBS3) -o creatdb

destroydb: destroydb.o $(LIBS3)
	$(CC) $(LDFLAGS) destroydb.o $(LIBS3) -o destroydb

helpr:	helpr.o error.o prvect.o $(LIBS3)
	$(CC) $(LDFLAGS) helpr.o error.o prvect.o $(LIBS3) -o helpr

indconv: indconv.o $(LIBS2) ${EQUEL} ${LIBQ}
	$(CC) indconv.o ${EQUEL} $(LIBS2) ${LIBQ} -o indconv

ingres: ingres.o $(LIBS2)
	$(CC) ingres.o $(LDFLAGS) $(LIBS2) -o ingres

ingres.o: ingres.c
	$(CC) $(CFLAGS) -I../ctlmod -c ingres.c

ingres_lock: ildr.o initsocket.o
	$(CC) $(CFLAGS) -o ingres_lock ildr.o initsocket.o

printadmin: printadmin.o $(LIBS2)
	$(CC) $(LDFLAGS) printadmin.o $(LIBS2) -o printadmin

printr: printr.o error.o prvect.o $(LIBS3)
	$(CC) $(LDFLAGS) printr.o error.o prvect.o $(LIBS3) -o printr

purge: purge.o subs.o error.o $(LIBS3)
	$(CC) $(LDFLAGS) purge.o subs.o error.o $(LIBS3) -o purge

restore: restore.o subs.o error.o $(LIBS3)
	$(CC) $(LDFLAGS) restore.o subs.o error.o $(LIBS3) -o restore

sysmod:	sysmod.o error.o prvect.o $(LIBS3) 
	$(CC) $(LDFLAGS) sysmod.o error.o prvect.o $(LIBS3) -o sysmod

usersetup: usersetup.o $(LIBS1)
	$(CC) $(LDFLAGS) usersetup.o $(LIBS1) -o usersetup

new:
	rm -f $(ALL) *.q

clean:	new
	rm -f *.o
	rm -f $(ALL) copydb.c copydb5.c indconv.c ingres.c a.out core y.tab.c

clobber: clean
	rm -f *.[chsy] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

#.DEFAULT:
#	sccs get $<

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

creatdb.o:$H/ingres.h
creatdb.o:$H/aux.h
creatdb.o:$H/version.h
creatdb.o:$H/access.h
creatdb.o:$H/symbol.h
creatdb.o:$H/pv.h
destroydb.o:$H/ingres.h
destroydb.o:$H/aux.h
destroydb.o:$H/access.h
error.o:$H/ingres.h
error.o:$H/aux.h
helpr.o:$H/ingres.h
helpr.o:$H/aux.h
helpr.o:$H/access.h
helpr.o:$H/lock.h
helpr.o:$H/pv.h
ingres.y:$H/ingres.h
ingres.y:$H/aux.h
ingres.y:$H/version.h
ingres.y:$H/access.h
ingres.y:$H/lock.h
ingres.y:$H/opsys.h
printr.o:$H/ingres.h
printr.o:$H/aux.h
printr.o:$H/access.h
printr.o:$H/lock.h
printr.o:$H/pv.h
purge.o:$H/ingres.h
purge.o:$H/aux.h
purge.o:$H/lock.h
subs.o:$H/ingres.h
subs.o:$H/aux.h
subs.o:$H/access.h
subs.o:$H/lock.h
sysmod.o:$H/ingres.h
sysmod.o:$H/aux.h
sysmod.o:$H/lock.h
sysmod.o:$H/access.h
sysmod.o:$H/pv.h
usersetup.o:$H/ingres.h
usersetup.o:$H/aux.h
usersetup.o:$H/opsys.h
restore.o:$H/ingres.h
restore.o:$H/aux.h
restore.o:$H/catalog.h
restore.o:$H/access.h
restore.o:$H/opsys.h
restore.o:$H/lock.h
restore.o:$H/symbol.h
restore.o:$H/resp.h
prvect.o:$H/tree.h
prvect.o:$H/pv.h
printadmin.o:$H/ingres.h
printadmin.o:$H/access.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
