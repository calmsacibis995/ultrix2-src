#  @(#)Makefile	1.3	ULTRIX	2/19/87

#
# CWD is the current working directory 
#

CWD= `pwd`

#
# Subdirectories that have makefiles of their own.
#

SUBDIRS=lk fort


#
# Any local variables that are specific to the Makefile may go here. 
# You may need to have a variable to pass different cflags to the lower
# directories.  Also, here is where variables dealing with tools should
# live. 


# 
# The all rule should look the same for branching into subdirectories
# to build binaries.
#

all: ${SUBDIRS}

${SUBDIRS}: FRC
	(cd $@; ${ECHO} Entering ${CWD}/$@; ${MAKE}; \
	 ${ECHO} Exiting ${CWD}/$@)

FRC:

#
# Install  should look the same for branching into subdirectories.
#

install:
	for i in ${SUBDIRS}; do \
		(cd $$i; ${ECHO} Entering ${CWD}; \
		${MAKE} DESTROOT=${DESTROOT} install; \
		 ${ECHO} Exiting ${CWD}); done

#
# Rule to remove all files and leave only the directories. It should
# first go into all subdirectories and run the clobber rule and then
# remove all files from within the CWD.  
#

clobber: 
	for i in ${SUBDIRS}; do \
		(cd $$i; ${ECHO} Entering ${CWD}; ${MAKE} clobber; \
		 ${ECHO} Exiting ${CWD}); done
	${SCCS} clean

#
# Rule to get all sources out of sccs after a clobber 
#

sccsget:
	${SCCS} get ${DATE} SCCS
	for i in ${SUBDIRS}; do \
		(cd $$i; ${ECHO} Entering ${CWD}; \
		 ${SCCS} get ${DATE} Makefile; ${MAKE} sccsget; \
		 ${ECHO} Exiting ${CWD}); done

#
# Rule to see if anything is checked out of sccs for edit.
#

sccsinfo:
	@${SCCS} info
	for i in ${SUBDIRS}; do \
		(cd $$i; ${ECHO} Entering ${CWD}; \
		 ${MAKE} sccsinfo; ${ECHO} Exiting ${CWD}); done
