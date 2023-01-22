# @(#)Makefile	1.5	ULTRIX	11/21/84
#
#	@(#)Makefile	4.1 5/19/82
#
DESTROOT=
CFLAGS=-O
SRCS = msgs.c msgs.h Makefile
msgs: msgs.c msgs.h
	cc $(CFLAGS) -o msgs msgs.c -ltermlib
install: msgs
	install -c  -s msgs ${DESTROOT}/usr/ucb
	-if [ ! -d ${DESTROOT}/usr/msgs ]; \
	then \
		mkdir ${DESTROOT}/usr/msgs; \
		chmod 777 ${DESTROOT}/usr/msgs; \
	else \
		true; \
	fi
	install -c /dev/null ${DESTROOT}/usr/msgs/bounds
	/etc/chown root ${DESTROOT}/usr/msgs/bounds
	chmod 666 ${DESTROOT}/usr/msgs/bounds
	chgrp system ${DESTROOT}/usr/msgs/bounds

clean:
	-rm -f msgs *.o

clobber: clean
	rm -f Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

sources: $(SRCS)
$(SRCS):
	sccs get $@
