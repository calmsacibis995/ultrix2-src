# @(#)Makefile	1.4	ULTRIX	12/4/84
#
#	@(#)Makefile	1.4 (Berkeley) 5/4/82
#
#	error.h			error header file
#	errorinput.c		does all of the input, and canonicalizes errors
#	errorpi.c		canonicalizes pi errors
#	errormain.c		main
#	errorsubr.c		does grotty subroutine work
#	errorfilter.c		"greps" out unwanted messages
#	errortouch.c		touches all mentioned files
#

DESTROOT=
HDRS = error.h
PHDRS = $(HDRS)
SRCS = errormain.c errorinput.c errorpi.c errorsubr.c errorfilter.c errortouch.c
PSRCS = $(SRCS)
OBJS = errormain.o errorinput.o errorpi.o errorsubr.o errorfilter.o errortouch.o

DFLAGS = -DDEBUG
CFLAGS =  -O $(DFLAGS)

LINTFLAGS = $(DFLAGS)

error:	$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o error

$(OBJS): $(HDRS)

install: error
	install -c -s error $(DESTROOT)/usr/ucb

clean:
	rm -f error $(OBJS)

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

lint:	
	lint $(LINTFLAGS) $(SRCS)

psrcs:
	echo $(PHDRS) $(PSRCS)

print:
	print $(PHDRS) $(PSRCS)
