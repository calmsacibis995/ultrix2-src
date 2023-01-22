######################################################################
#
#	Makefile for system generation handling of
#	sendmail configuration files.
#
#		@(#)Makefile	1.14	(ULTRIX)	1/29/87
#
######################################################################

DESTROOT=

MAILERS= localm.m4 uucpm.m4 tcpm.m4 xm.m4
BASES= version.m4 base.m4 localbase.m4 zerobase.m4

EXFILES= generic.mc

EXMAKE= Makefile.dist

GENSRCS= base.m4 generic.mc \
	localbase.m4 localm.m4 tcpm.m4 uucpm.m4 version.m4 xm.m4 \
	zerobase.m4

SRCS= ${EXFILES} ${EXMAKE} ${GENSRCS}

EXAMPLE= generic.cf exampleuucp.cf exampleether.cf

COPY= ${EXAMPLE} ${EXFILES} ${GENSRCS}

GET=	sccs get

.SUFFIXES: .mc .cf

.mc.cf:
	rm -f $*.cf
	m4 $*.mc > $*.cf
	chmod 755 $*.cf
	cp $*.cf exampleether.cf
	cp $*.cf exampleuucp.cf

all: $(EXAMPLE)

generic.cf:	generic.mc $(BASES) $(MAILERS)

#####################
#   administrivia   #
#####################

$(SRCS):
#	$(GET) $(REL) SCCS/s.$@

install: ${SRCS} ${EXAMPLE}
	-if [ ! -d ${DESTROOT}/usr/src ]; then\
		mkdir ${DESTROOT}/usr/src; \
		/etc/chown root ${DESTROOT}/usr/src; \
		chgrp system ${DESTROOT}/usr/src; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/src/usr.lib ]; then\
		mkdir ${DESTROOT}/usr/src/usr.lib ; \
		/etc/chown root ${DESTROOT}/usr/src/usr.lib ; \
		chgrp system ${DESTROOT}/usr/src/usr.lib ; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/src/usr.lib/sendmail ]; then\
		mkdir ${DESTROOT}/usr/src/usr.lib/sendmail ; \
		/etc/chown root ${DESTROOT}/usr/src/usr.lib/sendmail ; \
		chgrp system ${DESTROOT}/usr/src/usr.lib/sendmail ; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/src/usr.lib/sendmail/cf ]; then\
		mkdir ${DESTROOT}/usr/src/usr.lib/sendmail/cf ; \
		/etc/chown root ${DESTROOT}/usr/src/usr.lib/sendmail/cf ; \
		chgrp system ${DESTROOT}/usr/src/usr.lib/sendmail/cf ; \
	else true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/spool/mqueue ]; then\
		mkdir ${DESTROOT}/usr/spool/mqueue ; \
	else true; \
	fi
	-/etc/chown daemon ${DESTROOT}/usr/spool/mqueue
	-chgrp system ${DESTROOT}/usr/spool/mqueue
	-chmod 755 ${DESTROOT}/usr/spool/mqueue 
	-if [ ! -d ${DESTROOT}/usr/spool/mail ]; then\
		mkdir ${DESTROOT}/usr/spool/mail ; \
	else true; \
	fi
	-/etc/chown root ${DESTROOT}/usr/spool/mail
	-chgrp system ${DESTROOT}/usr/spool/mail
	-chmod 777 ${DESTROOT}/usr/spool/mail 
	-chmod +t ${DESTROOT}/usr/spool/mail 
	-for i in ${COPY} ; do \
		install -c $$i ${DESTROOT}/usr/src/usr.lib/sendmail/cf/$$i; \
	done
	-install -c Makefile.dist ${DESTROOT}/usr/src/usr.lib/sendmail/cf/Makefile 
	cp generic.cf $(DESTROOT)/usr/lib/sendmail.cf

clean:
	rm -f $(EXAMPLE) a.out core make.out

clobber: clean
	rm -f $(SRCS) ${EXAMPLE} Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

