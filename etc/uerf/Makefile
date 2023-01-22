#/*  sccsid  =  @(#)Makefile	1.17   (ULTRIX)   12/16/86  */
#

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

CCFLAGS =       -O -I/sys/vaxbi
CFLAGS  =       -O
LIB	=	-lerrlog

#
# Any shell scripts.
#

SCRIPTS=

#
# .c files which live in this directory
#

CFILES=		uerf.c getcmd.c errprn.c \
		esget.c esopen.c select.c ddtbl_ultrix.c \
		btt.c msgwrt.c  \
		eritio.c eribld.c erisel.c erierr.c ulfile.c eixfrm.c \
		dsd_access.c dsd_initld.c

#
# .h files which live in this directory
#

HFILES= 	ueliterals.h uestruct.h uerror.h \
		erms.h eims.h select.h \
		btliterals.h eiliterals.h ulliterals.h \
		generic_dsd.h std_dsd.h os_dsd.h dsd_switch.h

#
# Extra files which live in this directory
#

XFILES=		uerf.err uerf.hlp

#
# Source files which live in this directory
#

SOURCES= 	${CFILES} ${HFILES} ${XFILES} Makefile

#
# Binary files which live in this directory
#

STD=    	uerf

#
# Binary files which live in this directory but are non-standard
#

NSTD=

#
# Object files which live in this directory
#

OBJS=		uerf.o getcmd.o errprn.o \
		esget.o esopen.o select.o ddtbl_ultrix.o \
		btt.o msgwrt.o  \
		eritio.o eribld.o erisel.o erierr.o ulfile.o eixfrm.o \
		dsd_access.o dsd_initld.o

#
# Files which live in the this directory which get installed
#

FILES=		${STD} ${XFILES}

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


all:    ${OBJS}
	${CC} ${CFLAGS} -o ${STD} ${OBJS} ${LIB}

${OBJS}:
	${CC} ${CCFLAGS} -c $*.c

#
#${NSTD}:
#

#
# Any "special" make commands
#

# document:       uerf.8
# 	nroff -man uerf.8 > uerf.8.fmt


#
# Install relies on all having been done.
#

install: all

#
# Install the list of FILES (if present)
#

	for i in ${FILES}; do \
		install -c $$i ${DESTROOT}${DESTPATH}/$$i; done

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
			-e 's,</\(.*\)>,"/\1",' \
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
		rm -f *.out core ${OBJS} ${STD} ${NSTD}

#
# Rule to blow things away (FILES if present)
#

clobber: clean
	for i in ${SOURCES}; do rm -f $$i; done
	rm -f tags;

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
uerf.c: uerror.h
uerf.c: eiliterals.h
uerf.c: ueliterals.h
uerf.c: uestruct.h
uerf.c: btliterals.h
uerf.c: erms.h
uerf.c: generic_dsd.h
uerf.c: std_dsd.h
uerf.c: /usr/include/stdio.h
uerf.c: /usr/include/signal.h
uerf.c: /usr/include/ctype.h
uerf.c: /usr/include/sys/types.h
uerf.c: /usr/include/sys/stat.h
getcmd.c: ueliterals.h
getcmd.c: uestruct.h
getcmd.c: uerror.h
getcmd.c: eiliterals.h
getcmd.c: erms.h
getcmd.c: generic_dsd.h
getcmd.c: std_dsd.h
getcmd.c: /usr/include/stdio.h
getcmd.c: /usr/include/sys/time.h
errprn.c: ueliterals.h
errprn.c: /usr/include/stdio.h
errprn.c: /usr/include/ctype.h
esget.c: /usr/include/stdio.h
esget.c: erms.h
esget.c: eiliterals.h
esopen.c: /usr/include/stdio.h
esopen.c: erms.h
esopen.c: select.h
esopen.c: eiliterals.h
select.c: /usr/include/stdio.h
select.c: /usr/include/ctype.h
select.c: /usr/include/varargs.h
select.c: eims.h
select.c: select.h
ddtbl_ultrix.c: eims.h
ddtbl_ultrix.c: generic_dsd.h
btt.c: /usr/include/stdio.h
btt.c: /usr/include/sys/time.h
btt.c: ueliterals.h
btt.c: uestruct.h
btt.c: btliterals.h
btt.c: erms.h
btt.c: generic_dsd.h
btt.c: std_dsd.h
msgwrt.c: /usr/include/stdio.h
msgwrt.c: /usr/include/sys/time.h
msgwrt.c: ueliterals.h
msgwrt.c: uestruct.h
msgwrt.c: uerror.h
msgwrt.c: btliterals.h
msgwrt.c: generic_dsd.h
msgwrt.c: std_dsd.h
eritio.c: /usr/include/stdio.h
eritio.c: erms.h
eritio.c: eims.h
eritio.c: eiliterals.h
eritio.c: select.h
eribld.c: /usr/include/stdio.h
eribld.c: /usr/include/sys/types.h
eribld.c: /usr/include/sys/time.h
eribld.c: /usr/include/sys/errlog.h
eribld.c: /sys/vax/cpu.h
eribld.c: eims.h
eribld.c: erms.h
eribld.c: select.h
eribld.c: eiliterals.h
eribld.c: ulliterals.h
eribld.c: generic_dsd.h
eribld.c: std_dsd.h
eribld.c: os_dsd.h
erisel.c: /usr/include/stdio.h
erisel.c: /usr/include/sys/types.h
erisel.c: /usr/include/sys/time.h
erisel.c: /usr/include/sys/errlog.h
erisel.c: eiliterals.h
erisel.c: eims.h
erisel.c: select.h
erisel.c: generic_dsd.h
erisel.c: std_dsd.h
erierr.c: /usr/include/stdio.h
erierr.c: eiliterals.h
ulfile.c: eiliterals.h
ulfile.c: ulliterals.h
ulfile.c: eims.h
ulfile.c: erms.h
ulfile.c: select.h
ulfile.c: /usr/include/stdio.h
ulfile.c: /usr/include/signal.h
ulfile.c: /usr/include/sys/types.h
ulfile.c: /usr/include/sys/time.h
ulfile.c: /usr/include/sys/errlog.h
ulfile.c: /usr/include/sys/file.h
ulfile.c: /usr/include/sys/uio.h
ulfile.c: /usr/include/sys/socket.h
ulfile.c: /usr/include/sys/un.h
ulfile.c: /usr/include/errno.h
ulfile.c: /usr/include/elwindow.h
eixfrm.c: /usr/include/stdio.h
eixfrm.c: erms.h
eixfrm.c: eiliterals.h
eixfrm.c: generic_dsd.h
eixfrm.c: std_dsd.h
eixfrm.c: os_dsd.h
dsd_access.c: /usr/include/stdio.h
dsd_access.c: dsd_switch.h
dsd_access.c: generic_dsd.h
dsd_access.c: std_dsd.h
dsd_access.c: os_dsd.h
dsd_initld.c: generic_dsd.h
dsd_initld.c: std_dsd.h
dsd_initld.c: os_dsd.h



erms.h: eims.h
erms.h: select.h












# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
