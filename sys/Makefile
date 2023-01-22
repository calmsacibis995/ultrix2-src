#
#	3/18/87	(ULTRIX)	@(#)Makefile	1.111
#
#

CLOBBERDIR= ./780cons ./8600cons ./730cons ./sas \
	./cassette ./conf ./data ./dist ./stand ./floppy ./h ./mdec ./net \
	./netbsc ./netimp ./netinet ./netpup ./rl02 ./sys ./vaxbi ./vaxif \
	./vaxmba ./vaxuba ./emul ./netdnet ./lat ./dli ./gfs ./ufs ./nfs\
	./rpc ./rpcsvc ./vaxci

TARDIRS= ./780cons ./8600cons ./730cons ./sas \
	./cassette ./conf ./data ./dist ./stand ./floppy ./h ./mdec ./net \
	./netbsc ./netimp ./netinet ./netpup ./rl02 ./sys ./vax ./vaxbi \
	./vaxif ./vaxmba ./vaxuba ./emul ./netdnet ./lat ./dli ./gfs \
	./ufs ./nfs ./rpc ./rpcsvc ./vaxci

SAS_KERNELS= SAS.net SAS.gen SAS.rx01 SAS.tu58
MAKEDIRS= BINARY.vax GENERIC ${SAS_KERNELS}
SUBDIR= ./decnet
MACHINES=vax 
FIRST= sas stand mdec
MCLEANDIRS= 780cons 8600cons 730cons sas cassette floppy stand mdec rl02
#SRCDIR=/sys
SRCSYS=vacuum
DESTROOT=/Ultrix
DESTDIR=${DESTROOT}/usr/sys
#
#
# Files in the conf dir that go on the kit.
#
#
CONF-FILES=GENERIC BINARY.vax devices.vax \
	LINT files MY730 makefile.vax ERNIE MY780 newvers.sh \
	param.c KIM defines touch.c \
	files.vax MONET MY750

#
#
# Files in the floppy dir that go on the kit.
#
#

#FLOPPY-FILES= 3tracks Makefile anyboo.cmd boot boot_justask copy defboo.cmd \
#	defboo.hk defboo.hp defboo.ra defboo.up drtest floppy format \
#	hkmboo.cmd hksboo.cmd hpmboo.cmd hpsboo.cmd ramboo.cmd rasboo.cmd \
#	restar.cmd upmboo.cmd upsboo.cmd utboot.cmd

#
#
# Files in the mdec dir that go on the kit.
#
#
MDEC-FILES=Makefile a.out b.out hkboot hpboot htboot httoggle mtboot mttoggle \
	noboot raboot rlboot tmboot tmtoggle tsboot tstoggle tuboot \
	upboot utboot uttoggle 

#
# Files in the netinet and netbsc dirs that go on the kit.
#
#
NETINET-FILES=icmp_var.h in_proto.c ip_var.h tcp_seq.h udp.h if_ether.h \
	in_systm.h tcp.h tcp_timer.h udp_var.h in.h ip.h tcp_debug.h \
	tcp_var.h in_pcb.h ip_icmp.h tcp_fsm.h tcpip.h in_var.h

NETBSC-FILES=bsc.h bsc_messages.h bsc_proto.c bsc_proto.h \
	bsc_states.h bsc_timer.h  \
	bsc_var.h

NET-FILES= *.h conf_net.c

#
#
# Files in the vax dir that go on the kit.
#
#
VAX-FILES=crt0.ex mtpr.h pte.h trap.h b_params.h nexus.h \
    reg.h vmparam.h clock.h flp.h param.h rpb.h conf.c frame.h pcb.h rsp.h \
    cons.h mem.h pcb.m scb.h cpu.h mscp.h psl.h symbols.raw symbols.sort \
    spt.s tmscp.h ioa.h crl.h rpb.s scb.s cpuconf.c swap.c ka8200.h ka8800.h

#
#
# Files in the vaxbi dir that go on the kit.
#
#
VAXBI-FILES= bireg.h buareg.h bdareg.h dmbreg.h bvp_ni.h nireg.h


all: first makes

tags: 
	/usr/ucb/ctags -w `cat conf/files conf/files.vax | awk '{print $$1}'` 

clean:
	-for i in ${MCLEANDIRS} ${MAKEDIRS} ;do \
		if [ -d $$i/. ]; then \
			cd $$i;make clean;cd ..;\
		fi; \
	done
	cd vax/inline; make clean

clobber: clean
	rm -fr ${MAKEDIRS}
	for i in ${CLOBBERDIR} ${SUBDIR}; do \
	cd $$i;sccs clean;cd ..;done
	cd vax; rm -f [a-hj-z]* i*.*
	cd vax/inline; make clobber


sccsinfo:
	@for i in ${TARDIRS} ${SUBDIR} ; do cd $$i;echo -n "$$i:";sccs info;cd .. ; done
	@sccs info
	cd vax/inline; make sccsinfo

sccsget:
	@-for i in ${TARDIRS} ${SUBDIR};do cd $$i;echo "$$i";sccs get -s SCCS;cd ..;done
	cd vax/inline; sccs get Makefile; make sccsget
	cd dist/setld; sccs get Makefile; make sccsget
	cd dist/tools; sccs get Makefile; make sccsget

BINARY.vax:
	-rm -fr BINARY.vax
	-for i in ${MACHINES}; do \
		if [ ! -d BINARY.$$i ]; \
		then \
			mkdir BINARY.$$i; \
		else \
			true; \
		fi; \
	(cd conf;/etc/config -s BINARY.$$i;cd ../BINARY.$$i;make depend); done

GENERIC:
	-if [ ! -d GENERIC ]; \
	then \
		mkdir GENERIC; \
	else \
		true; \
	fi
	(cd conf;/etc/config GENERIC;cd ../GENERIC;make depend)

SAS.net:
	-if [ ! -d SAS.net ]; \
	then \
		mkdir SAS.net; \
	else \
		true; \
	fi
	(cd conf;/etc/config SAS.net;cd ../SAS.net;make depend)

SAS.gen:
	-if [ ! -d SAS.gen ]; \
	then \
		mkdir SAS.gen; \
	else \
		true; \
	fi
	(cd conf;/etc/config SAS.gen;cd ../SAS.gen;make depend)

SAS.rx01:
	-if [ ! -d SAS.rx01 ]; \
	then \
		mkdir SAS.rx01; \
	else \
		true; \
	fi
	(cd conf;/etc/config SAS.rx01;cd ../SAS.rx01;make depend)

SAS.tu58:
	-if [ ! -d SAS.tu58 ]; \
	then \
		mkdir SAS.tu58; \
	else \
		true; \
	fi
	(cd conf;/etc/config SAS.tu58;cd ../SAS.tu58;make depend)

install: all instdirs inst-780cons inst-8600cons inst-730cons inst-sas \
	inst-BINARY inst-GENERIC inst-stand inst-cassette \
	inst-conf inst-data inst-dist inst-SAS inst-floppy \
	inst-h inst-mdec inst-netbsc \
	inst-net inst-netimp inst-netinet inst-netpup inst-rl02 inst-sys \
	inst-vax inst-vaxinline inst-vaxbi inst-vaxif inst-vaxmba inst-vaxuba \
	inst-netdnet inst-gfs inst-ufs inst-nfs inst-rpc inst-rpcsvc \
	inst-vaxci

	-if [ ! -d ${DESTROOT}/usr/lib ]; \
	then \
		mkdir ${DESTROOT}/usr/lib; \
		chmod 755 ${DESTROOT}/usr/lib; \
	else \
		true; \
	fi
	-if [ ! -d ${DESTROOT}/tp ]; \
	then \
		mkdir ${DESTROOT}/tp; \
		chmod 755 ${DESTROOT}/tp; \
	else \
		true; \
	fi
	-if [ ! -d ${DESTROOT}/usr/mdec ]; \
	then \
		mkdir ${DESTROOT}/usr/mdec; \
		chmod 755 ${DESTROOT}/usr/mdec; \
	else \
		true; \
	fi
	(cd mdec;make  "DESTROOT=${DESTROOT}" install)

instdirs:
	-if [ ! -d ${DESTDIR} ]; \
	then \
		mkdir ${DESTDIR}; \
		chmod 0755 ${DESTDIR}; \
	else \
		true; \
	fi
	@rm -f ${DESTROOT}/sys
	@cd ${DESTROOT}; ln -s usr/sys ${DESTROOT}/sys
	@-for i in  ${TARDIRS} BINARY.vax ; do \
		if [ ! -d ${DESTROOT}/usr/sys/$$i ] ; \
		then \
			echo makeing $$i; \
			mkdir ${DESTROOT}/usr/sys/$$i; \
			/etc/chown root ${DESTROOT}/usr/sys/$$i; \
			chgrp system ${DESTROOT}/usr/sys/$$i; \
			chmod 0755 ${DESTROOT}/usr/sys/$$i; \
		else \
			true; \
		fi ; \
	done


makes: ${MAKEDIRS}
	for i in ${MAKEDIRS}; do  cd $$i;make ${MFLAGS} all; cd ..  ; done

first:
	cd sas; make ${MFLAGS}
	cd stand;make ${MFLAGS}
	cd stand; make ${MFLAGS} setup
	cd mdec; make ${MFLAGS}

inst-BINARY: 
	@echo 'installing BINARIES for '${MACHINES}
	@-for i in ${MACHINES}; do \
	if [ ! -d ${DESTDIR}/BINARY.$$i ]; \
	then \
		mkdir ${DESTDIR}/BINARY.$$i; \
	else \
		true; \
	fi; \
	(cd BINARY.$$i;tar cf - . | ( cd ${DESTDIR}/BINARY.$$i;tar xpf - \
	; rm -f locore.c .[a-zA-Z]* log; chmod ugo+r *)); \
	done

inst-GENERIC: GENERIC
	@echo Installing Generic system
	@cd GENERIC; install -c -m 755 vmunix ${DESTROOT}/genvmunix; \
		ln ${DESTROOT}/genvmunix ${DESTROOT}/vmunix

inst-SAS: 
	@echo 'installing standalone kernels'
	@-for i in net gen rx01 tu58; do \
	if [ ! -d ${DESTDIR}/SAS.$$i ]; \
	then \
		mkdir ${DESTDIR}/SAS.$$i; \
	else \
		true; \
	fi; \
	(cd SAS.$$i;install -c -m 755 vmunix ${DESTDIR}/SAS.$$i/vmunix);done

	
inst-8600cons:
	@echo Installing 8600cons
	@cd 8600cons;chmod ugo+r *;tar cFf - . | ( cd ${DESTDIR}/8600cons;tar xpf - )

inst-rl02:
	@echo Installing RL02
	@cd rl02;chmod ugo+r *;tar cFf - . | ( cd ${DESTDIR}/rl02;tar xpf - )

inst-730cons: 
	@echo Installing 730cons
	@cd 730cons;chmod ugo+r *;tar cFf - . | ( cd ${DESTDIR}/730cons;tar xpf - )

inst-cassette:
	@echo Installing Cassette
	@cd cassette;chmod ugo+r *;tar cFf - . | ( cd ${DESTDIR}/cassette;tar xpf - )

inst-conf:
	@echo Installing Config files.
	@cd conf; tar cFf - ${CONF-FILES} | ( cd ${DESTDIR}/conf;tar xpf - )

inst-data:
	@echo Installing data files
	@cd data;chmod ugo+r *;tar cFf - *.c | ( cd ${DESTDIR}/data;tar xpf - )

inst-dist:
	@echo installing dist files
	@cd dist;make ${MFLAGS} DESTROOT=${DESTROOT} install

inst-780cons:
	@echo Installing 780cons files.
	@cd 780cons;tar cFf - . |(cd ${DESTDIR}/780cons;tar xpf -;chmod ugo+r *)

inst-floppy:
	@echo Installing Floppy files.
	@cd floppy;tar cFf - . |(cd ${DESTDIR}/floppy;tar xpf -;chmod ugo+r *)

inst-h:
	@echo Installing Header files
	@cd h;chmod ugo+r *;tar cFf - *.h | ( cd ${DESTDIR}/h;tar xpf - )

inst-mdec:
	@echo Installing mdec
	@(cd mdec;tar cFf - ${MDEC-FILES}|(cd ${DESTDIR}/mdec;tar xpf -;chmod ugo+r *))

inst-netbsc:
	@echo Installing netbsc files.
	@cd netbsc;tar cFf - ${NETBSC-FILES}|( cd ${DESTDIR}/netbsc; tar xpf - ; chmod ugo+r *)

inst-net:
	@echo Installing net files.
	@cd net;tar cFf - ${NET-FILES}|(cd ${DESTDIR}/net;tar xpf - ; chmod ugo+r * )

inst-netimp:
	@echo Installing netimp files.
	@cd netimp;tar cFf - *.h|(cd ${DESTDIR}/netimp;tar xpf -;chmod ugo+r *)

inst-netinet:
	@echo Installing netinet files.
	@cd netinet;tar cFf - ${NETINET-FILES}|( cd ${DESTDIR}/netinet; tar xpf - ; chmod ugo+r *)

inst-netpup:
	@echo Installing netpup files.
	@cd netpup;tar cFf - *.h | (cd ${DESTDIR}/netpup;tar xpf -;chmod ugo+r *)

inst-sas:
	@echo Installing sas files
	(cd sas; make ${MFLAGS} DESTROOT=${DESTROOT} install)
	@cd sas;tar cFf - *.h|(cd ${DESTDIR}/sas;tar xpf -; chmod ugo+r *)

inst-stand:
	@echo Stand, that is easy.
	(cd stand; make ${MFLAGS} DESTDIR=${DESTROOT} install)

inst-sys:
	@echo Sys, that is easy to!

inst-vax:
	@echo Installing vax files.
	cd vax;tar cFf - ${VAX-FILES}|(cd ${DESTDIR}/vax;tar xpf -; chmod ugo+r * ; cd ${DESTDIR}; rm -f machine; ln -s vax machine)

inst-vaxinline:
	@echo Installing vax/inline/inline.
	cd vax/inline; make DESTROOT=${DESTROOT} install

inst-vaxbi:
	@echo Installing vaxbi files.
	cd vaxbi;tar cFf - ${VAXBI-FILES}|(cd ${DESTDIR}/vaxbi;tar xpf -; chmod ugo+r *)

inst-vaxci:
	@echo Installing vaxci files- Nothing to install !!!!

inst-vaxif:
	@echo Installing vaxif files.
	@cd vaxif;tar cFf - *.h|(cd ${DESTDIR}/vaxif;tar xpf -; chmod ugo+r *)

inst-vaxmba:
	@echo Installing vaxmba files.
	@cd vaxmba;tar cFf - *.h|(cd ${DESTDIR}/vaxmba;tar xpf -; chmod ugo+r *)

inst-vaxuba:
	@echo Installing vaxuba files.
	@cd vaxuba;tar cFf - *.h|(cd ${DESTDIR}/vaxuba;tar xpf -; chmod ugo+r *)

inst-netdnet:
	@echo Installing netdnet files.
	@cd netdnet;tar cFf - *.[ch]|(cd ${DESTDIR}/netdnet;tar xpf -; chmod ugo+r *)

inst-lat:
	@echo Installing lat files.
	@cd lat;tar cFf - *.h|(cd ${DESTDIR}/lat;tar xpf -; chmod ugo+r *)

inst-gfs:
	@echo Gfs, thats easy too!

inst-ufs:
	@echo Installing ufs files.
	@cd ufs;tar cFf - *.h|(cd ${DESTDIR}/ufs;tar xpf -; chmod ugo+r *)

inst-nfs:
	@echo Installing nfs files.
	@cd nfs;tar cFf - *.h|(cd ${DESTDIR}/nfs;tar xpf -; chmod ugo+r *)

inst-rpc:
	@echo Installing rpc files.
	@cd rpc;tar cFf - *.h|(cd ${DESTDIR}/rpc;tar xpf -; chmod ugo+r *)

inst-rpcsvc:
	@echo Installing rpcsvc files.
	@cd rpcsvc;tar cFf - *.h|(cd ${DESTDIR}/rpcsvc;tar xpf -; chmod ugo+r *)

