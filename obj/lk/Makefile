#  @(#)Makefile	1.3	ULTRIX	10/3/86


OBJS = lk

INSTFLAGS = -m 755 -o root -g system 

SCRIPTS =

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

#
# Install will place the objects and executables onto the output pack.
#

install:
	for i in ${OBJS}; do \
		(install ${INSTFLAGS} -c $$i ${DESTROOT}/usr/bin/$$i); done

#
# Rule to remove all files within CWD and leave only the SCCS directory 
# (this assumes that all files needed within the directory live in
# sccs).  
#

clobber:
	${SCCS} clean

#
# Rule to get all sources out of sccs after a clobber was done or 
# just to make sure that the lastest of all files are within directory.
#

sccsget:
	${SCCS} get ${DATE} SCCS

#
# Rule to see if anything is checked out of sccs for edit.
#

sccsinfo:
	@${SCCS} info

