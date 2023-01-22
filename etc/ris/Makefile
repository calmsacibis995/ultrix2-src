# @(#)Makefile	1.4	Ultrix	1/29/87
# ris Makefile  -- jhw
CFLAGS = -O
DESTROOT= 

SRCS=ris makpkt.c licheck.c 
CSRCS=makpkt.c licheck.c
OBJS=makpkt.o licheck.o
BINS=makpkt licheck 

all: $(BINS) ris

$(BINS): $(CSRCS) 
	 $(CC) $(CFLAGS) -c $(@).c
	 $(CC) $(CFLAGS) -o $(@) -s $(@).o

clean : 
	rm -f $(OBJS)

clobber: clean
	rm -f   Makefile [a-z]*

sccsget:
	sccs get $(SRCS)

sccsinfo:
	sccs info

print :
	pr Makefile $(SRCS) | lpr -J lcp

calls :
	cat $(SRCS) | calls -v | lpr -J callgraph

install: makpkt licheck ris 

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
	-if [ ! -d ${DESTROOT}/usr/adm ] ; then \
		mkdir ${DESTROOT}/usr/adm; \
		chmod 755 ${DESTROOT}/usr/adm; \
		/etc/chown root ${DESTROOT}/usr/adm; \
		chgrp system ${DESTROOT}/usr/adm; \
		else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/adm/ris ] ; then \
		mkdir ${DESTROOT}/usr/adm/ris; \
		chmod 755 ${DESTROOT}/usr/adm/ris; \
		/etc/chown ris ${DESTROOT}/usr/adm/ris; \
		chgrp ris ${DESTROOT}/usr/adm/ris; \
		else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/adm/ris/bin ] ; then \
		mkdir ${DESTROOT}/usr/adm/ris/bin; \
		chmod 755 ${DESTROOT}/usr/adm/ris/bin; \
		/etc/chown ris ${DESTROOT}/usr/adm/ris/bin; \
		chgrp ris ${DESTROOT}/usr/adm/ris/bin; \
		else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/adm/ris/clients ] ; then \
		mkdir ${DESTROOT}/usr/adm/ris/clients; \
		chmod 755 ${DESTROOT}/usr/adm/ris/clients; \
		/etc/chown ris ${DESTROOT}/usr/adm/ris/clients; \
		chgrp ris ${DESTROOT}/usr/adm/ris/clients; \
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
	-if [ ! -d ${DESTROOT}/usr/lib/mop/clients ] ; then \
		mkdir ${DESTROOT}/usr/lib/mop/clients; \
		chmod 755 ${DESTROOT}/usr/lib/mop/clients; \
		/etc/chown root ${DESTROOT}/usr/lib/mop/clients; \
		chgrp system ${DESTROOT}/usr/lib/mop/clients; \
		else true; \
	fi
	install -c ris ${DESTROOT}/etc/ris
	chmod 700 ${DESTROOT}/etc/ris
	/etc/chown root ${DESTROOT}/etc/ris
	chgrp system ${DESTROOT}/etc/ris
	install -c -s makpkt ${DESTROOT}/usr/adm/ris/bin/makpkt
	chmod 700 ${DESTROOT}/usr/adm/ris/bin/makpkt
	/etc/chown root ${DESTROOT}/usr/adm/ris/bin/makpkt
	chgrp system ${DESTROOT}/usr/adm/ris/bin/makpkt
	install -c -s licheck ${DESTROOT}/usr/adm/ris/bin/licheck
	chmod 700 ${DESTROOT}/usr/adm/ris/bin/licheck
	/etc/chown root ${DESTROOT}/usr/adm/ris/bin/licheck
	chgrp system ${DESTROOT}/usr/adm/ris/bin/licheck
