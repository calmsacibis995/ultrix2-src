# @(#)Makefile	1.8	ULTRIX	12/4/84
#
# sccsid = "@(#)Makefile	1.4 (Berkeley) 4/28/83"
#
#	dump.h			header file
#	dumpitime.c		reads /etc/ddate
#	dumpmain.c		driver
#	dumpoptr.c		operator interface
#	dumptape.c		handles the mag tape and opening/closing
#	dumptraverse.c		traverses the file system
#	unctime.c		undo ctime
#	l3tol.c			3-byte to 4-byte unpacking
#
#	DEBUG			use local directory to find ddate and dumpdates
#	TDEBUG			trace out the process forking
#
DESTROOT=

PSRCS = \
	dump.h dumpmain.c dumptraverse.c dumptape.c dumpoptr.c dumpitime.c

SRCS =  \
	dump.h dumpitime.c \
	dumpmain.c dumpoptr.c dumptape.c \
	dumptraverse.c unctime.c l3tol.c

OBJS = \
	dumpitime.o \
	dumpmain.o dumpoptr.o \
	dumptape.o dumptraverse.o unctime.o l3tol.o

DFLAGS =
CFLAGS = -O $(DFLAGS)

dump:	$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o dump
install: dump
	install -c -s dump $(DESTROOT)/etc/dump.4.1
clean:
	rm -f *.o dump

clobber: clean
	rm -f *.[hc] Makefile
	rm -f [a-z]*/*.h [a-z]*/[a-z]*/*.h 
sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
	(cd include.4.1; sccs get SCCS; cd sys; sccs get SCCS)

.DEFAULT:
	sccs get $<

lint:
	lint $(DFLAGS) $(SRCS)

psrcs:
	echo $(PSRCS)
