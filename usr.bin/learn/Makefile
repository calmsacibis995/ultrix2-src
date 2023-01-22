# @(#)Makefile	1.6	ULTRIX	10/2/84
#
#	@(#)Makefile	4.3	(Berkeley)	83/06/19
#
DESTROOT=

LESSONS = files editor morefiles macros eqn C

FILES = lrnref.h \
	copy.c dounit.c getlesson.c learn.c list.c \
	makpipe.c maktee.c mem.c mysys.c selsub.c selunit.c \
	start.c whatnow.c wrapup.c \
	lcount.c lrntee.c \
	makefile

OBJECTS = copy.o dounit.o getlesson.o learn.o list.o mem.o \
	makpipe.o maktee.o mysys.o selsub.o selunit.o \
	start.o whatnow.o wrapup.o

CFLAGS = -O
LIBRARY =
LLIB	= ${DESTROOT}/usr/lib/learn/bin

all:	learn lrntee lcount

install:	all
	-if [ ! -d ${DESTROOT}/usr/lib/learn ]; then \
		mkdir ${DESTROOT}/usr/lib/learn; \
		chmod 755 ${DESTROOT}/usr/lib/learn; \
		/etc/chown root ${DESTROOT}/usr/lib/learn; \
		chgrp system ${DESTROOT}/usr/lib/learn; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/lib/learn/bin ]; then \
		mkdir ${DESTROOT}/usr/lib/learn/bin; \
		chmod 755 ${DESTROOT}/usr/lib/learn/bin; \
		/etc/chown root ${DESTROOT}/usr/lib/learn/bin; \
		chgrp system ${DESTROOT}/usr/lib/learn/bin; \
	else true; \
	fi
	install -c -s learn ${DESTROOT}/usr/bin
	install -c -s learn ${LLIB}
	install -c -s lrntee ${LLIB}
	install -c -s lcount ${LLIB}

ccinstall:	all
	cp learn lrntee lcount $(LLIB)
	chmod 755 $(LLIB)/*
	strip $(LLIB)/*

clean:
	rm -f *.o learn lrntee lcount errs

clobber: clean
	rm -f *.[ch] README  *.orig

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

cmp:	all
	cmp learn $(DESTROOT)/usr/bin/learn
	cmp learn $(LLIB)/learn
	cmp lrntee $(LLIB)/lrntee
	cmp lcount $(LLIB)/lcount
	rm learn lrntee lcount *.o

learn: $(OBJECTS)
	cc -o learn $(CFLAGS) $(OBJECTS) $(LIBRARY)

$(OBJECTS): lrnref.h

lcount lrntee:
	cc $(CFLAGS) $@.c -o $@ $(LIBRARY)

log:
	-rm -r $(LLIB)/../log; mkdir $(LLIB)/../log; chmod +w $(LLIB)/../log

check:
	-@test -r $(LLIB)/lrntee || echo 'lrntee not present; make lrntee'
	-@test -r $(LLIB)/lcount || echo 'lcount not present; make lcount'
	-@test -r $(LLIB)/../log || echo 'log directory not present; make log if you want logging'
