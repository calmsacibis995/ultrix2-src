#  @(#)Makefile	1.6	ULTRIX	1/26/87


LIBOBJS =fortrtl.a fortlib.a fortinitial.o fortrtl1 fortrtl2 fortmsgfile

BINOBJS = om

LIBINSTFLAGS = -m 644 -o root -g system

BININSTFLAGS = -m 755 -o root -g system

SCRIPTS =

SUBDIRS = omdir

RM = /bin/rm

#
# Any local variables that are specific to the Makefile should go after
# the above variables.  For example, you may want to use different
# variables for specifying directories where things get installed.
# Also, here is where any variables dealing with the tools rules 
# should live.

#
# For the first rule within the makefile, you must have the "all"
# rule.  This builds all the sources.
#

all:
	for i in ${SUBDIRS}; do (cd $$i; make); done
	cp omdir/om .

#
# Install will place the objects and executables onto the output pack.
#

install:
	dd if=fortrtl.a of=fortrtl1 ibs=10k count=38
	-dd if=fortrtl.a of=fortrtl2 ibs=10k skip=38

	for i in ${LIBOBJS}; do (install ${LIBINSTFLAGS} -c $$i ${DESTROOT}/usr/lib/$$i); done
	for i in ${BINOBJS}; do (install ${BININSTFLAGS} -c $$i ${DESTROOT}/usr/bin/$$i); done

clean:
	for i in ${SUBDIRS}; do (cd $$i; make clean); done

#
# Rule to remove all files within CWD and leave only the SCCS directory 
# (this assumes that all files needed within the directory live in
# sccs).  
#

clobber:
	${SCCS} clean
	for i in ${SUBDIRS}; do (cd $$i; ${SCCS} clean); done

#
# Rule to get all sources out of sccs after a clobber was done or 
# just to make sure that the lastest of all files are within directory.
#

sccsget:
	${SCCS} get ${DATE} SCCS
	for i in ${SUBDIRS}; do (cd $$i; ${SCCS} get ${DATE} SCCS); done

#
# Rule to see if anything is checked out of sccs for edit.
#

sccsinfo:
	@${SCCS} info
	for i in ${SUBDIRS}; do (cd $$i; ${SCCS} info); done

