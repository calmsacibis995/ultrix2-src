#  @(#)Makefile	1.1	ULTRIX	10/3/86
#/****************************************************************
#*								*
#*  Licensed to Digital Equipment Corporation, Maynard, MA	*
#*		Copyright 1985 Sun Microsystems, Inc.		*
#*			All rights reserved.			*
#*								*
#****************************************************************/
#
#
CFLAGS= -O
CWD=`pwd'
DESTROOT=
ECHO=/bin/echo
SCCS=/usr/ucb/sccs

HDRS = getgroup.h table.h util.h
OBJS = revnetgroup.o getgroup.o table.o util.o
SRCS = revnetgroup.c getgroup.c table.c util.c

revnetgroup: $(OBJS)
	cc $(CFLAGS) $(OBJS) -o revnetgroup

lint:
	lint $(SRCS)

clean: 
	rm -f revnetgroup  $(OBJS)

clobber:
	${SCCS} clean

sccsget:
	${SCCS} get ${DATE} ${HDRS} ${SRCS};

sccsinfo:
	@${SCCS} info;
	
install: revnetgroup
#	Directories made by makefile(s) in upper level(s) of tree
	install -s -c revnetgroup $(DESTROOT)/usr/etc/yp

depend:
	cp /dev/null x.c
	for i in $(SRCS) ; do \
		(/bin/grep '^#[     ]*include' x.c $$i | sed \
			-e 's,<\(.*\)>,"/usr/include/\1",' \
			-e 's/:[^"]*"\([^"]*\)".*/: \1/' \
			-e 's/\.c/\.o/' >>makedep); done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep x.c
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile
 
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above

