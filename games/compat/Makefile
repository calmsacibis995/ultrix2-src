# @(#)Makefile	1.5	ULTRIX	9/13/84
#
# 	Makefile	4.2	82/10/24
#
DESTROOT=
MISCSRCS = Readme v6run.1
SSRCS = compat.s memsiz.s
HSRCS = defs.h rt11.h unix6sys.h unix6sysn.h unix7sys.h unix7sysn.h unixhdr.h
CSRCS = bcopy.c dofloat.c dosig.c runcompat.c unixemts.c unixstart.c unixtraps.c

SRCS = $(MISCSRCS) $(SSRCS) $(HSRCS) $(CSRCS)

v7run:	compat.o v7run.o v7start.o v7traps.o unixemts.o rdosig.o\
	dofloat.o bcopy.o
	ld -e start -N -o v7run compat.o v7run.o v7start.o v7traps.o bcopy.o \
	unixemts.o rdosig.o dofloat.o /lib/crt0.o -lc -s

v6run:	compat.o v6run.o v6start.o v6traps.o unixemts.o rdosig.o \
	bcopy.o dofloat.o
	ld -e start -N -o v6run compat.o v6run.o v6start.o v6traps.o\
	unixemts.o rdosig.o dofloat.o bcopy.o /lib/crt0.o -lc -s

v7trc:	compat.o v7run.o v7start.o v7ttraps.o unixemts.o tdosig.o dofloat.o
	ld -e start -N -o v7trc compat.o v7run.o v7start.o v7ttraps.o\
	unixemts.o tdosig.o dofloat.o /lib/crt0.o -lc

v6trc:	compat.o v6run.o v6start.o v6ttraps.o unixemts.o tdosig.o dofloat.o
	ld -e start -N -o v6trc compat.o v6run.o v6start.o v6ttraps.o\
	unixemts.o tdosig.o dofloat.o /lib/crt0.o -lc

rtrun:	compat.o rtrun.o rtstart.o rttraps.o rtemts.o rdosig.o dofloat.o
	ld -e start -N -o rtrun compat.o rtrun.o rtstart.o rttraps.o\
	rtemts.o rdosig.o dofloat.o /lib/crt0.o -lc

rttrc:	compat.o rtrun.o rtstart.o rtttraps.o rtemts.o rdosig.o dofloat.o
	ld -e start -N -o rttrc compat.o rtrun.o rtstart.o rttraps.o\
	rtemts.o rdosig.o dofloat.o /lib/crt0.o -lc

all:	v7run v6run # v7trc v6trc

compat.o: memsiz.s compat.s
	cat memsiz.s compat.s | as -o compat.o

v7run.o: defs.h unixhdr.h runcompat.c
	cc -c -O -DV7UNIX -DUNIX runcompat.c
	mv runcompat.o v7run.o

v6run.o: defs.h unixhdr.h runcompat.c
	cc -c -O -DV6UNIX -DUNIX runcompat.c
	mv runcompat.o v6run.o

rtrun.o: defs.h rt11.h runcompat.c
	cc -c -O -DRT11 runcompat.c
	mv runcompat.o rtrun.o

v7start.o: defs.h unixstart.c
	cc -c -O -DV7UNIX unixstart.c
	mv unixstart.o v7start.o

v6start.o: defs.h unixstart.c
	cc -c -O -DV6UNIX unixstart.c
	mv unixstart.o v6start.o

v7traps.o: defs.h unix7sys.h unixtraps.c
	cc -c -O -DV7UNIX unixtraps.c
	mv unixtraps.o v7traps.o

v6traps.o: defs.h unix6sys.h unixtraps.c
	cc -c -O -DV6UNIX unixtraps.c
	mv unixtraps.o v6traps.o

v7ttraps.o: defs.h unix7sys.h unixtraps.c
	cc -c -O -DV7UNIX -DTRACE unixtraps.c
	mv unixtraps.o v7ttraps.o

v6ttraps.o: defs.h unix6sys.h unixtraps.c
	cc -c -O -DV6UNIX -DTRACE unixtraps.c
	mv unixtraps.o v6ttraps.o

unixemts.o: unixemts.c
	cc -c -O unixemts.c

rdosig.o: dosig.c
	cc -c -O dosig.c
	mv dosig.o rdosig.o

tdosig.o: dosig.c
	cc -c -O -DTRACE dosig.c
	mv dosig.o tdosig.o

dofloat.o: defs.h dofloat.c
	cc -c -O dofloat.c

install: v7run # v6run v6trc v7trc
	-if [ ! -d ${DESTROOT}/usr/games/lib ]; then \
		mkdir ${DESTROOT}/usr/games/lib; \
		chmod 755 ${DESTROOT}/usr/games/lib; \
		/etc/chown root ${DESTROOT}/usr/games/lib; \
		chgrp system ${DESTROOT}/usr/games/lib; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/local ]; then \
		mkdir ${DESTROOT}/usr/local; \
		chmod 755 ${DESTROOT}/usr/local; \
		/etc/chown root ${DESTROOT}/usr/local; \
		chgrp system ${DESTROOT}/usr/local; \
	else true; \
	fi
	install v7run $(DESTROOT)/usr/games/lib/compat
	: install v6run $(DESTROOT)/usr/local
	: install v6trc $(DESTROOT)/usr/local
	: install v7trc $(DESTROOT)/usr/local
	: chmod 4755 $(DESTROOT)/usr/local/v?run $(DESTROOT)/usr/local/v?trc

clean:
	rm -f *.o v6run v7run v7trc v6trc
	(cd modshell; make ${MFLAGS} clean)

sources: $(SRCS)

$(SRCS):
	sccs get $@


clobber: clean
	rm -f *.[chs] Makefile Readme v6run.1
	(cd modshell; make ${MFLAGS} clobber)

sccsget:
	sccs get SCCS 
	( cd modshell; sccs get SCCS)

sccsinfo:
	sccs info 
	( cd modshell; make ${MFLAGS} sccsinfo)

.DEFAULT:
	sccs get $<
