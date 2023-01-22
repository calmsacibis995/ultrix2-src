#
#       @(#)Makefile	1.1     Ultrix  4/4/85
#
#
# COMPILATION SYMBOLS:
#
#   RTDEVNAM - A quoted string that gives the path name of the "stand-
#       ard place" to find RT-11 volume.
#   ULTRIX32 - Defined only iff "m" (no interleave  mapping)  modifier
#       is NOT to be assumed, e.g., on ULTRIX-32 and 4.2bsd.
#


#
# Local definitions for arff
#
# Following for PRO/VENIX
# RTDEVNAM =    '"/dev/rf0.m11"'
# Following for ULTRIX-11
# RTDEVNAM =    '"/dev/rhx0"'
# Following for ULTRIX-32
RTDEVNAM =      '"/dev/bootdev"'

# Following for non ULTRIX-32
# OPSYSDEF =
# Following for ULTRIX-32
OPSYSDEF =      -DULTRIX32

#
# DESTROOT is the path to root while installing (passed)
#

DESTROOT=

#
# DESTPATH is the path from root to the directory where files are installed
# (passed)
#

DESTPATH=       /etc

#
# C Compiler flags (also available are [YFERPL]FLAGS for various compilers)
#

CFLAGS =        -O -DRTDEVNAM=${RTDEVNAM} ${OPSYSDEF}

#
# Any shell scripts.
#

SCRIPTS=

#
# Source files which live in this directory
#

SOURCES=        arcpad.c ardown.c arff.c   arnlen.c arpars.c \
		arrad5.c arvfyn.c arwcid.c rtcons.c rtctrd.c rtdate.c rtdele.c \
		rtdepm.c rtdire.c rtdsio.c rtextr.c rtinit.c \
		rtmkfi.c rtmkfn.c rtmvde.c rtopen.c rtpakn.c \
		rtpio.c  rtprot.c rtrepl.c rtscnd.c rtspds.c \
		rtwmat.c arff.h Makefile

#
# Binary files which live in this directory
#

STD=    arff

#
# Binary files which live in this directory but are non-standard
#

NSTD=

#
# Object files which live in this directory
#

OBJS=           arcpad.o ardown.o arff.o   arnlen.o arpars.o \
		arrad5.o arvfyn.o arwcid.o rtcons.o rtctrd.o rtdate.o rtdele.o \
		rtdepm.o rtdire.o rtdsio.o rtextr.o rtinit.o \
		rtmkfi.o rtmkfn.o rtmvde.o rtopen.o rtpakn.o \
		rtpio.o  rtprot.o rtrepl.o rtscnd.o rtspds.o \
		rtwmat.o


#
# Files which live in the this directory which get installed
#

FILES=${STD} ${NSTD}

#
# Stdtools1 are tools in this directory which need to be built
# and installed to build tools2 (If present)
#

STDTOOLS1=

#
# Stdtools2 are tools in this directory which need to be installed
# for a full system build (If present)
#

STDTOOLS2=

#
# Make all relies on the subdirectories being there as well as any
# pertinent files in the top level directory
#


all:    ${OBJS} ${FILES} ${STD}

${STD}: ${OBJS}
	${CC} ${CFLAGS} -o ${STD} ${STD}.o ${OBJS}

#
#${NSTD}:
#       ${CC} ${CFLAGS} -o $@ $@.o

#
# Any "special" make commands
#

document:       arff.8
	nroff -man arff.8 > arff.8.fmt


#
# Install relies on all having been done.
#

install: all

#
# Install the list of FILES (if present)
#

	for i in ${FILES}; do \
		install -c $$i ${DESTROOT}${DESTPATH}/$$i; done; \

#
# Any special install commands
#


#
# Rule to install tools1 phase of tool generation
#

#tools1: ${STDTOOLS1}
#
#        for i in ${STDTOOLS1}; do \
#                install -c $$i ${DESTROOT}${DESTPATH}/$$i; done

#
# Rule to install tools2 phase of tool generation
#

#tools2: ${STDTOOLS2}
#
#        for i in ${STDTOOLS2}; do \
#                install -c $$i ${DESTROOT}${DESTPATH}/$$i; done

#
# Depend rule (Make sure Makefile is checked out for edit before
#  make depend is issued)
#

depend:
	cat </dev/null >x.c
	for i in ${SOURCES}; do \
		(/bin/grep '^#[  ]*include' x.c $$i | sed \
			-e '/\.\.\/h/d' \
			-e 's,<\(.*\)>,"/usr/include/\1",' \
			-e 's/:[^"]*"\([^"]*\)".*/: \1/' >> makedep); done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep x.c
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

#
# Rule to clean things up (FILES if present)
#

clean:
		for i in ${FILES} ${OBJS}; do (rm -f $$i); done

#
# Rule to blow things away (FILES if present)
#

clobber: clean
	for i in ${SOURCES}; do rm -f $$i; done

#
# Rule to get sources back after a clobber
#

sccsget:
	@sccs get -s SCCS

# Change mode for any shell scripts.


#
# Rule to see if anything is checked out
#

sccsinfo:
	@sccs info

# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
ardown.c: /usr/include/ctype.h
ardown.c: arff.h
arff.c: arff.h
arff.c: /usr/include/stdio.h
arpars.c: arff.h
arpars.c: /usr/include/stdio.h
arrad5.c: arff.h
arrad5.c: /usr/include/stdio.h
arrad5.c: /usr/include/ctype.h
rtcons.c: arff.h
rtcons.c: /usr/include/stdio.h
rtctrd.c: arff.h
rtctrd.c: /usr/include/stdio.h
rtdate.c: arff.h
rtdate.c: /usr/include/time.h
rtdele.c: arff.h
rtdele.c: /usr/include/stdio.h
rtdepm.c: arff.h
rtdire.c: arff.h
rtdire.c: /usr/include/time.h
rtdire.c: /usr/include/stdio.h
rtdsio.c: arff.h
rtdsio.c: /usr/include/stdio.h
rtextr.c: arff.h
rtextr.c: /usr/include/stdio.h
rtinit.c: arff.h
rtinit.c: /usr/include/stdio.h
rtinit.c: /usr/include/ctype.h
rtmkfi.c: arff.h
rtmkfi.c: /usr/include/stdio.h
rtmkfn.c: /usr/include/stdio.h
rtmkfn.c: arff.h
rtmvde.c: arff.h
rtopen.c: arff.h
rtopen.c: /usr/include/stdio.h
rtopen.c: /usr/include/errno.h
rtpakn.c: /usr/include/stdio.h
rtpio.c: arff.h
rtpio.c: /usr/include/sys/types.h
rtpio.c: /usr/include/stdio.h
rtprot.c: arff.h
rtprot.c: /usr/include/stdio.h
rtrepl.c: arff.h
rtrepl.c: /usr/include/stdio.h
rtscnd.c: arff.h
rtscnd.c: /usr/include/stdio.h
rtspds.c: arff.h
rtspds.c: /usr/include/stdio.h
rtwmat.c: /usr/include/stdio.h
rtwmat.c: /usr/include/ctype.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
