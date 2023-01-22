#  @(#)Makefile	1.4	ULTRIX	10/3/86

# Makefile to build 2780d and install into /etc

# All makefiles we have changed considerably must contain out copyright
# notice.  Those makefiles we haven't touched i.e., user contributed
# are still undecided which notice to put in them.  Jim D. will get
# back to me.

#    COPYRIGHT NOTICE

DESTROOT = 

HDRS = atoe.h etoa.h rje.local.h rje.h

OBJS = 2780d.o nmatch.o

INSTFLAGS = -c -s -m 700 -o root
CFLAGS = -O

SCRIPTS =

STD = 2780d

NSTD =

#
# Any local variables that are specific to the Makefile
#

CC = /bin/cc
SOURCES = 2780d.c nmatch.c

all: ${STD} ${HDRS}

${STD}: ${OBJS}
	${CC} ${CFLAGS} -o ${STD} ${OBJS}


#
# Install will place the sources onto the output pack.
#

install: all
	for i in ${STD}; do \
	     install ${INSTFLAGS} ${STD} ${DESTROOT}/etc/$$i; done;

#
# Removes things that have gotten built and error files
#

clean:
	rm -f *.out core ${STD} ${NSTD} ${OBJS}

#
# Removes all files from the directory ( assumes that everything needed
# lives in SCCS ).
#

clobber:
	-for i in `ls -a`; do \
		if test -f $$i; \
		then \
		rm -f $$i; \
	fi \
	done

#
# Bring everything out of sccs
#

sccsget:
	sccs get -s SCCS

#
# Check to see if sources are out of sccs for edit.
#

sccsinfo:
	@sccs info


