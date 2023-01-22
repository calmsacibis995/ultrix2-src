# @(#)Makefile	1.19	ULTRIX	9/23/85
#
#	Makefile for sendmail base directory
#
#		@(#)Makefile	4.2		9/12/83
#

ALL=	src/sendmail aux/syslog
DIRS=	$(DESTROOT)/usr/spool/mqueue
SRCDIR=	$(DESTROOT)/usr/src/usr.lib/sendmail
ALLDIRS=include adm src aux md cf lib doc adb test

DIRWMAKE = aux cf doc lib src
DIRWOMAKE = adb adm include md 

BACKUP=	calder
SENDMAILNAME = /usr/lib/sendmail
SENDMAIL=$(DESTROOT)$(SENDMAILNAME)
SYSLOG=	/usr/spool/mqueue/syslog
INSTHOST= exampleether

LN=	ln -s
CP=	cp

all:
	cd lib; make ${MFLAGS}
#	cd src; m4 Makefile.m4 > Makefile; make ${MFLAGS}
#	cd aux; m4 Makefile.m4 > Makefile; make ${MFLAGS}
	cd src; make ${MFLAGS}
	cd aux; make ${MFLAGS}
	cd cf;  make ${MFLAGS}
#	cd doc; make ${MFLAGS}

install: all $(ALL) $(DIRS)
	@-if [ ! -d ${DESTROOT}/usr/adm ]; then \
		mkdir ${DESTROOT}/usr/adm; \
		chmod 755 ${DESTROOT}/usr/adm; \
		/etc/chown root ${DESTROOT}/usr/adm; \
		chgrp system ${DESTROOT}/usr/adm; \
	else true; \
	fi
	@-if [ ! -d ${DESTROOT}/usr/ucb ]; then \
		mkdir ${DESTROOT}/usr/ucb; \
		chmod 755 ${DESTROOT}/usr/ucb; \
		/etc/chown root ${DESTROOT}/usr/ucb; \
		chgrp system ${DESTROOT}/usr/ucb; \
	else true; \
	fi
	install -o 1		aux/syslog	$(DESTROOT)/etc/syslog
	install -c -o 1 -m 644	aux/syslog.conf	$(DESTROOT)/etc/syslog.conf
	install -c -o 1 -m 644	/dev/null	$(DESTROOT)/etc/syslog.pid
	install -c -o 1 -m 644	/dev/null	$(DESTROOT)$(SYSLOG)
	for i in 0 1 2 3 4 5 6 7; do install -c -o 1 -m 644 /dev/null $(DESTROOT)$(SYSLOG).$$i; done
	install -c -o 1		lib/newsyslog.sh $(DESTROOT)/usr/adm/newsyslog
#	echo	'5 4 * * * sh /usr/adm/newsyslog' >> $(DESTROOT)/usr/lib/crontab
	install -c lib/aliases $(DESTROOT)/usr/lib/aliases
	install -c lib/aliases.dir $(DESTROOT)/usr/lib/aliases.dir
	install -c lib/aliases.pag $(DESTROOT)/usr/lib/aliases.pag	
	install -c -s -m 4755	src/sendmail	$(SENDMAIL)
	install -c -m 644	lib/sendmail.hf	$(SENDMAIL).hf
	install -c -m 666	/dev/null	$(SENDMAIL).st
	install -c -m 644	/dev/null	$(SENDMAIL).fc
	install -c -m 644	cf/$(INSTHOST).cf	$(SENDMAIL).cf
	rm -f					$(DESTROOT)/usr/ucb/newaliases
	$(LN)		$(SENDMAILNAME)		$(DESTROOT)/usr/ucb/newaliases
	rm -f					$(DESTROOT)/usr/ucb/mailq
	$(LN)		$(SENDMAILNAME)		$(DESTROOT)/usr/ucb/mailq

	(cd cf; make ${MFLAGS} DESTROOT=${DESTROOT} install)

src/sendmail:
	cd src; make ${MFLAGS} sendmail

aux/syslog:
	cd aux; make ${MFLAGS} syslog

$(DESTROOT)/usr/spool/mqueue:
	mkdir			$(DESTROOT)/usr/spool/mqueue
	chmod	755		$(DESTROOT)/usr/spool/mqueue

clean:
	for i in ${DIRWMAKE}; do (cd $$i; make ${MFLAGS} clean); done
	for i in ${DIRWOMAKE}; do (cd $$i; rm -f *.o errs core a.out); done

clobber: clean
	for i in ${DIRWMAKE}; do (cd $$i; make ${MFLAGS} clobber); done
	for i in ${DIRWOMAKE}; do (cd $$i; rm -f *.[ch] eve*); done
	rm -f READ_ME FILES Makefile

sccsinfo:
	sccs info
	for i in ${DIRWMAKE}; do (cd $$i;echo $$i; make ${MFLAGS} sccsinfo);done
	for i in ${DIRWOMAKE}; do (cd $$i;echo $$i;sccs info);done

sccsget:
	sccs get SCCS
	for i in ${DIRWMAKE}; do (cd $$i;sccs get Makefile; make ${MFLAGS} sccsget); done
	for i in ${DIRWOMAKE}; do (cd $$i; sccs get SCCS); done

dirs:
	mkdir $(SRCROOT) 
	cd $(SRCROOT); mkdir $(ALLDIRS)

rcp:
	rsh $(BACKUP) mkdir sendmail
	rsh $(BACKUP) 'cd sendmail; mkdir $(ALLDIRS)'
	for i in `cat FILES`; do rcp $$i $(BACKUP):sendmail/$$i; done

srcinstall:
	for i in `cat FILES`; do rm -f $(SRCDIR)/$$i; cp $$i $(SRCDIR)/$$i; done

tar:
	rm -f sendmail.tar
	tar rcfh sendmail.tar `cat FILES`
