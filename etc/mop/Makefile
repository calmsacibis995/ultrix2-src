#
#	@(#)Makefile	1.8	(ULTRIX)	5/19/86
#
 
DESTROOT= 
LIBR= -lmop
CFLAGS= -O
CFILES=	mop_mom.c mop_dumpload.c mop_parsrqst.c mop_fetchfile.c \
	mop_dlload.c mop_udump.c mop_gpsubr.c mop_syslog.c \
	addnode.c getnode.c remnode.c \
	mop_forceload.c store_parm.c dliconn.c \
	ccr.c ccr_dbaccess.c ccr_netwio.c ccr_subr.c ccr_process.c \
	ccr_io.c ccr_release.c ccr_reserve.c \
	mop_boot.c valid_line_name.c load_interface.c \
	print_nodeent.c print_usage.c

OBJECTS=	mop_mom.o mop_dumpload.o mop_parsrqst.o mop_fetchfile.o \
	mop_dlload.o mop_udump.o mop_gpsubr.o mop_syslog.o \
	addnode.o getnode.o remnode.o \
	mop_forceload.o store_parm.o dliconn.o \
	mop_boot.o valid_line_name.o load_interface.o \
	ccr.o ccr_dbaccess.o ccr_netwio.o ccr_subr.o ccr_process.o \
	ccr_io.o ccr_release.o ccr_reserve.o \
	print_nodeent.o print_usage.o

all:	mop_mom mop_dumpload load addnode getnode remnode ccr

mop_mom:	mop_mom.o mop_syslog.o 
	cc ${CFLAGS} mop_mom.o mop_syslog.o -o mop_mom ${LIBR}

mop_dumpload:	mop_dumpload.o mop_parsrqst.o mop_fetchfile.o \
		mop_dlload.o mop_udump.o mop_gpsubr.o mop_syslog.o 
	cc ${CFLAGS} mop_dumpload.o mop_parsrqst.o mop_fetchfile.o \
		mop_dlload.o mop_udump.o mop_gpsubr.o mop_syslog.o \
		-o mop_dumpload ${LIBR}

load:	load_interface.o mop_forceload.o mop_gpsubr.o store_parm.o \
		valid_line_name.o dliconn.o mop_syslog.o \
		mop_boot.o print_usage.o nmgtdb.h mopdb.h nmgtdb.h
	cc ${CFLAGS} load_interface.o mop_forceload.o mop_gpsubr.o \
		store_parm.o valid_line_name.o dliconn.o mop_boot.o \
		mop_syslog.o print_usage.o -o load ${LIBR}

addnode:	addnode.o store_parm.o print_usage.o valid_line_name.o \
		nmgtdb.h mopdb.h
	  cc ${CFLAGS} addnode.o store_parm.o print_usage.o valid_line_name.o \
		${LIBR} -o addnode 

remnode: remnode.o mopdb.h
	  cc ${CFLAGS} remnode.o ${LIBR} -o remnode 

getnode: getnode.o print_nodeent.o mopdb.h
	  cc ${CFLAGS} getnode.o print_nodeent.o ${LIBR} -o getnode 

ccr:	ccr_prs_sysid.o ccr.o ccr_dbaccess.o ccr_netwio.o \
		ccr_subr.o ccr_process.o \
		ccr_io.o ccr_release.o ccr_reserve.o \
		mop_forceload.o mop_gpsubr.o \
		dliconn.o mop_syslog.o mop_boot.o 
	cc $(CFLAGS) ccr_prs_sysid.o ccr.o ccr_dbaccess.o ccr_netwio.o \
		ccr_subr.o ccr_process.o \
		ccr_io.o ccr_release.o ccr_reserve.o \
		mop_forceload.o mop_gpsubr.o dliconn.o mop_syslog.o \
		mop_boot.o -o ccr $(LIBR) -lup

ccr_prs_sysid.o:	ccr_prs_sysid.u 
	/usr/bin/upars  ccr_prs_sysid.u -o ccr_prs_sysid.c
	cc $(CFLAGS) -c ccr_prs_sysid.c

${OBJECTS}:	mopdb.h mop_proto.h mop_var.h nmgtdb.h \
		ccr.h ccr_mesg_codes.h ccr_mesg_text.h

install:	mop_mom mop_dumpload load addnode getnode remnode ccr

	-if [ ! -d ${DESTROOT}/usr ] ; then \
		mkdir ${DESTROOT}/usr; \
		chmod 755 ${DESTROOT}/usr; \
		/etc/chown root ${DESTROOT}/usr; \
		chgrp system ${DESTROOT}/usr; \
		else true; \
	fi

	-if [ ! -d ${DESTROOT}/usr/lib ] ; then \
		mkdir ${DESTROOT}/usr/lib; \
		chmod 755 ${DESTROOT}/usr/lib; \
		/etc/chown root ${DESTROOT}/usr/lib; \
		chgrp system ${DESTROOT}/usr/lib; \
		else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/lib/dnet ] ; then \
		mkdir ${DESTROOT}/usr/lib/dnet; \
		chmod 755 ${DESTROOT}/usr/lib/dnet; \
		/etc/chown root ${DESTROOT}/usr/lib/dnet; \
		chgrp system ${DESTROOT}/usr/lib/dnet; \
		else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/lib/mop ] ; then \
		mkdir ${DESTROOT}/usr/lib/mop; \
		chmod 755 ${DESTROOT}/usr/lib/mop; \
		/etc/chown root ${DESTROOT}/usr/lib/mop; \
		chgrp system ${DESTROOT}/usr/lib/mop; \
		else true; \
	fi
	install -c -s mop_dumpload ${DESTROOT}/usr/lib/dnet/mop_dumpload
	install -c /dev/null ${DESTROOT}/usr/lib/dnet/nodes_v
	install -c -s mop_mom ${DESTROOT}/etc/mop_mom
	install -c -s load ${DESTROOT}/etc/load
	rm -f ${DESTROOT}/etc/trigger
	ln ${DESTROOT}/etc/load ${DESTROOT}/etc/trigger
	install -c -s addnode ${DESTROOT}/etc/addnode
	install -c -s getnode ${DESTROOT}/etc/getnode
	install -c -s remnode ${DESTROOT}/etc/remnode
	install -c -s -m 4755 ccr ${DESTROOT}/etc/ccr

clean:	
	rm -f a.out ccr core *.s *.o u_DEFINES.h u_STATES.h ccr_prs_sysid.c
	rm -f mop_dumpload mop_mom load addnode remnode getnode

clobber: clean
	rm -f *.[chu] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS
