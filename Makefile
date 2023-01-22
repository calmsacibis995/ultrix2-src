# @(#)Makefile	1.35	ULTRIX	3/18/87
#	@(#)Makefile	4.3	(Berkeley)	8/20/83
#
DESTROOT=/Ultrix
NEWROOT=/
CFLAGS= -O

#	Modification History
#	--------------------
#
# 001 - Ray Glaser, xx-Jun-86
#	Add the sub-directory  usr.etc  for  Yellow pages
#
#-------

# Programs that live in subdirectories, and have makefiles of their own.
#
SUBDIR= lib usr.lib include bin usr.bin etc usr.etc ucb sys games field usr root new

# New definition to facilitate parallel builds
SUBDIRP= START
#lib usr.lib include bin usr.bin etc usr.etc ucb sys games field usr root new

# Vax specific directories
VAX=root


all:	${SUBDIRP}
	for i in ${VAX}; do \
		(cd $$i; make ${MFLAGS}); done

${SUBDIRP}: FRC
	cd lib;make ${MFLAGS} >/tmp/lib.out 2>&1 & \
	cd ../usr.lib;make ${MFLAGS} >/tmp/usr.lib.out 2>&1 & \
	cd ../include;make ${MFLAGS} >/tmp/include.out 2>&1 & \
	cd ../bin;make ${MFLAGS} >/tmp/bin.out 2>&1 & \
	cd ../usr.bin;make ${MFLAGS} >/tmp/usr.bin.out 2>&1 & \
	cd ../etc;make ${MFLAGS} >/tmp/etc.out 2>&1 & \
	cd ../usr.etc;make ${MFLAGS} >/tmp/usr.etc.out 2>&1 & \
	cd ../ucb;make ${MFLAGS} >/tmp/ucb.out 2>&1 & \
	cd ../sys;make ${MFLAGS} >/tmp/sys.out 2>&1 & \
	cd ../games;make ${MFLAGS} >/tmp/games.out 2>&1 & \
	cd ../field;make ${MFLAGS} >/tmp/field.out 2>&1 & \
	cd ../usr;make ${MFLAGS} >/tmp/usr.out 2>&1 & \
	cd ../root;make ${MFLAGS} >/tmp/root.out 2>&1 & \
	cd ../new;make ${MFLAGS} >/tmp/new.out 2>&1 & \
	cd ../obj;make ${MFLAGS} >/tmp/obj.out 2>&1 & wait
	cat /tmp/lib.out /tmp/usr.lib.out /tmp/include.out \
	/tmp/bin.out /tmp/usr.bin.out /tmp/etc.out /tmp/usr.etc.out \
	/tmp/ucb.out \
	/tmp/sys.out /tmp/games.out /tmp/field.out /tmp/usr.out \
	/tmp/root.out /tmp/new.out /tmp/obj.out
	rm -f /tmp/lib.out /tmp/usr.lib.out /tmp/include.out \
	/tmp/bin.out /tmp/usr.bin.out /tmp/etc.out /tmp/usr.etc.out \
	/tmp/ucb.out \
	/tmp/sys.out /tmp/games.out /tmp/field.out /tmp/usr.out \
	/tmp/root.out /tmp/new.out /tmp/obj.out

FRC:

install:
	for i in ${SUBDIR}; do \
		(cd $$i; make ${MFLAGS} DESTROOT=${DESTROOT} install); done
	@-if [ ! -d ${DESTROOT}/tmp ]; \
	then \
		mkdir ${DESTROOT}/tmp; \
		chmod 777 ${DESTROOT}/tmp; \
	else \
		true; \
	fi
	(cd obj; make ${MFLAGS} DESTROOT=${DESTROOT} install)

clean:
	rm -f a.out core *.s *.o
	cd lib;make ${MFLAGS} clean >/tmp/lib.out 2>&1 & \
	cd ../usr.lib;make ${MFLAGS} clean >/tmp/usr.lib.out 2>&1 & \
	cd ../include;make ${MFLAGS} clean >/tmp/include.out 2>&1 & \
	cd ../bin;make ${MFLAGS} clean >/tmp/bin.out 2>&1 & \
	cd ../usr.bin;make ${MFLAGS} clean >/tmp/usr.bin.out 2>&1 & \
	cd ../etc;make ${MFLAGS} clean >/tmp/etc.out 2>&1 & \
	cd ../usr.etc;make ${MFLAGS} clean >/tmp/usr.etc.out 2>&1 & \
	cd ../ucb;make ${MFLAGS} clean >/tmp/ucb.out 2>&1 & \
	cd ../sys;make ${MFLAGS} clean >/tmp/sys.out 2>&1 & \
	cd ../games;make ${MFLAGS} clean >/tmp/games.out 2>&1 & \
	cd ../field;make ${MFLAGS} clean >/tmp/field.out 2>&1 & \
	cd ../usr;make ${MFLAGS} clean >/tmp/usr.out 2>&1 & \
	cd ../root;make ${MFLAGS} clean >/tmp/root.out 2>&1 & \
	cd ../new;make ${MFLAGS} clean >/tmp/new.out 2>&1 & wait
	cat /tmp/lib.out /tmp/usr.lib.out /tmp/include.out \
	/tmp/bin.out /tmp/usr.bin.out /tmp/etc.out /tmp/usr.etc.out \
	/tmp/ucb.out \
	/tmp/sys.out /tmp/games.out /tmp/field.out /tmp/usr.out \
	/tmp/root.out /tmp/new.out
	for i in ${VAX}; do \
		(cd $$i; make ${MFLAGS} clean); done

clobber:
	rm -f a.out core *.s *.o
	cd lib;make ${MFLAGS} clobber >/tmp/lib.out 2>&1 & \
	cd ../usr.lib;make ${MFLAGS} clobber >/tmp/usr.lib.out 2>&1 & \
	cd ../include;make ${MFLAGS} clobber >/tmp/include.out 2>&1 & \
	cd ../bin;make ${MFLAGS} clobber >/tmp/bin.out 2>&1 & \
	cd ../usr.bin;make ${MFLAGS} clobber >/tmp/usr.bin.out 2>&1 & \
	cd ../etc;make ${MFLAGS} clobber >/tmp/etc.out 2>&1 & \
	cd ../usr.etc;make ${MFLAGS} clobber >/tmp/usr.etc.out 2>&1 & \
	cd ../ucb;make ${MFLAGS} clobber >/tmp/ucb.out 2>&1 & \
	cd ../sys;make ${MFLAGS} clobber >/tmp/sys.out 2>&1 & \
	cd ../games;make ${MFLAGS} clobber >/tmp/games.out 2>&1 & \
	cd ../field;make ${MFLAGS} clobber >/tmp/field.out 2>&1 & \
	cd ../usr;make ${MFLAGS} clobber >/tmp/usr.out 2>&1 & \
	cd ../root;make ${MFLAGS} clobber >/tmp/root.out 2>&1 & \
	cd ../obj; make ${MFLAGS} clobber >/tmp/obj.out 2>&1 & \
	cd ../new;make ${MFLAGS} clobber >/tmp/new.out 2>&1 & wait
	cat /tmp/lib.out /tmp/usr.lib.out /tmp/include.out \
	/tmp/bin.out /tmp/usr.bin.out /tmp/etc.out /tmp/usr.etc.out \
	/tmp/ucb.out \
	/tmp/sys.out /tmp/games.out /tmp/field.out /tmp/usr.out \
	/tmp/root.out  /tmp/obj.out /tmp/new.out

sccsinfo:
	for i in ${SUBDIR}; do (cd $$i; make ${MFLAGS} sccsinfo ); done
	(cd obj; make ${MFLAGS} sccsinfo)
	for i in ${VAX} ; do \
		(cd $$i; make ${MFLAGS} sccsinfo); done

sccsget:
	cd lib;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/lib.out 2>&1 & \
	cd ../usr.lib;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/usr.lib.out 2>&1 & \
	cd ../include;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/include.out 2>&1 & \
	cd ../bin;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/bin.out 2>&1 &\
	cd ../usr.bin;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/usr.bin.out 2>&1 & \
	cd ../etc;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/etc.out 2>&1 &\
	cd ../usr.etc;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/usr.etc.out 2>&1 &\
	cd ../ucb;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/ucb.out 2>&1 &\
	cd ../sys;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/sys.out 2>&1 &\
	cd ../games;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/games.out 2>&1 & \
	cd ../field;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/field.out 2>&1 & \
	cd ../usr;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/usr.out 2>&1 &\
	cd ../root;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/root.out 2>&1 & \
	cd ../obj;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/obj.out 2>&1 & \
	cd ../new;sccs get Makefile;make ${MFLAGS} sccsget >/tmp/new.out 2>&1 &\
	wait
	cat /tmp/lib.out /tmp/usr.lib.out /tmp/include.out \
	/tmp/bin.out /tmp/usr.bin.out /tmp/etc.out /tmp/usr.etc.out \
	/tmp/ucb.out \
	/tmp/sys.out /tmp/games.out /tmp/field.out /tmp/usr.out \
	/tmp/root.out /tmp/obj.out /tmp/new.out
	rm -f /tmp/lib.out /tmp/usr.lib.out /tmp/include.out \
	/tmp/bin.out /tmp/usr.bin.out /tmp/etc.out /tmp/usr.etc.out \
	/tmp/ucb.out \
	/tmp/sys.out /tmp/games.out /tmp/field.out /tmp/usr.out \
	/tmp/root.out /tmp/obj.out /tmp/new.out
	for i in ${VAX} ; do \
		(cd $$i; sccs get Makefile; make ${MFLAGS} sccsget); done

prebuild:
#  Lib is special as to insure compiler changes we compile it,
#  (the loader & the assembler), install them then chroot so that
#  only an uptodate compiler recompiles itself.
#  Perhaps builds of as & ld & other things necessary for a full
#  build could be done here - lp

	(cd lib; make ${MFLAGS} clean)
	@echo "(cd /library/lib;make ${MFLAGS} all;make ${MFLAGS} DESTROOT=${NEWROOT} install)" | ../chroot ${NEWROOT}
	@- if [ ${NEWROOT} != '/' ] ;\
	then \
	echo "(cd /library/bin; rm -f cc ld as/as; make -k cc ld; cd as; make -k as)" | ../chroot ${NEWROOT}; \
	echo "(cd /library/bin; cp -r cc ld /bin; cp as/as /bin)" | ../chroot ${NEWROOT}; \
	echo "(cd /library; make prebuild DESTROOT=/)" | ../chroot ${NEWROOT}; \
	fi

# Prebuild without a clean first
pre2:
	(cd lib; make ${MFLAGS} DESTROOT=${NEWROOT} install)
	@- if [ ${NEWROOT} != '/' ] ;\
	then \
	echo "(cd library/bin; rm -f cc ld as/as; make -k cc ld; cd as; make -k as)" | ../chroot ${NEWROOT}; \
	echo "(cd library/bin; cp -r cc ld /bin; cp as/as /bin)" | ../chroot ${NEWROOT}; \
	echo "(cd library; make prebuild DESTROOT=/)" | ../chroot ${NEWROOT}; \
	fi
test:
	@- if [ ${NEWROOT} != '/' ] ;\
	then \
	echo "cd library/bin; rm -f cc ld as/as; make -k cc ld; cd as; make -k as" | ../chroot ${NEWROOT}; \
	echo "cd library/bin; cp -r cc ld /bin; cp as/as /bin" | ../chroot ${NEWROOT}; \
	fi

pretools:

	@echo Tools in ${DESTROOT}/lib ; \
	echo lib in pretools started `date` >>/progress
	    cd /library/lib;make -k DESTROOT=/ tools ; \
	echo Tools in ${DESTROOT}/usr/lib ; \
	echo usr.lib in pretools started `date` >>/progress
	    cd /library/usr.lib;make -k DESTROOT=/ tools1 ; \
	echo Tools in ${DESTROOT}/usr/include ; \
	echo include in pretools started `date` >>/progress
	    cd /library/include;make -k DESTROOT=/ install ; \
	echo Tools in ${DESTROOT}/bin ; \
	echo bin in pretools started `date` >>/progress
	    cd /library/bin;make -k DESTROOT=/ tools ; \
	echo Tools in ${DESTROOT}/usr/bin ; \
	echo usr.bin in pretools started `date` >>/progress
	    cd /library/usr.bin;make -k DESTROOT=/ tools1 ; \
	echo Tools in ${DESTROOT}/usr/ucb ; \
	echo ucb in pretools started `date` >>/progress
	    cd /library/ucb;make -k DESTROOT=/ tools1 ; \
	echo Tools in ${DESTROOT}/etc ; \
	echo etc in pretools started `date` >>/progress
	    cd /library/etc;make -k DESTROOT=/ tools

tools1:
# Clean all the areas silently in parallel and wait before doing any compiles ...
	@cd /library/lib;make -k -s clean >/dev/null 2>&1 & \
	cd /library/usr.lib;make -k -s clean >/dev/null 2>&1 & \
	cd /library/include;make -k -s clean >/dev/null 2>&1 & \
	cd /library/bin;make -k -s clean >/dev/null 2>&1 & \
	cd /library/usr.bin;make -k -s clean >/dev/null 2>&1 & \
	cd /library/ucb;make -k -s clean >/dev/null 2>&1 & \
	cd /library/etc;make -k -s clean >/dev/null 2>&1 & \
	wait

# now build all of tools1 sequentially
	@echo Tools in ${DESTROOT}/lib ; \
	echo lib in tools1 started `date` >>/progress
	    cd /library/lib;make -k DESTROOT=/ tools ; \
	echo Tools in ${DESTROOT}/usr/lib ; \
	echo usr.lib in tools1 started `date` >>/progress
	    cd /library/usr.lib;make -k DESTROOT=/ tools1 ; \
	echo Tools in ${DESTROOT}/usr/include ; \
	echo include in tools1 started `date` >>/progress
	    cd /library/include;make -k DESTROOT=/ install ; \
	echo Tools in ${DESTROOT}/bin ; \
	echo bin in tools1 started `date` >>/progress
	    cd /library/bin;make -k DESTROOT=/ tools ; \
	echo Tools in ${DESTROOT}/usr/bin ; \
	echo usr.bin in tools1 started `date` >>/progress
	    cd /library/usr.bin;make -k DESTROOT=/ tools1 ; \
	echo Tools in ${DESTROOT}/usr/ucb ; \
	echo ucb in tools1 started `date` >>/progress
	    cd /library/ucb;make -k DESTROOT=/ tools1 ; \
	echo Tools in ${DESTROOT}/etc ; \
	echo etc in tools1 started `date` >>/progress
	    cd /library/etc;make -k DESTROOT=/ tools

tools2:
# Clean all the areas silently in parallel and wait before doing any compiles 
	@cd /library/lib;make -k -s clean >/dev/null 2>&1 & \
	cd /library/usr.lib;make -k -s clean >/dev/null 2>&1 & \
	cd /library/include;make -k -s clean >/dev/null 2>&1 & \
	cd /library/bin;make -k -s clean >/dev/null 2>&1 & \
	cd /library/usr.bin;make -k -s clean >/dev/null 2>&1 & \
	cd /library/ucb;make -k -s clean >/dev/null 2>&1 & \
	cd /library/etc;make -k -s clean >/dev/null 2>&1 & \
	wait

# now make the lib, usr/lib, include, and bin areas sequentially ...
	@echo Tools in ${DESTROOT}/lib ; \
	echo lib in tools2 started `date` >>/progress
	    cd /library/lib;make -k DESTROOT=/ tools ; \
	echo Tools in ${DESTROOT}/usr/lib ; \
	echo usr.lib in tools2 started `date` >>/progress
	    cd /library/usr.lib;make -k DESTROOT=/ tools2 ; \
	echo Tools in ${DESTROOT}/usr/include ; \
	echo include in tools2 started `date` >>/progress
	    cd /library/include;make -k DESTROOT=/ install ; \
	echo Tools in ${DESTROOT}/bin ; \
	echo bin in tools2 started `date` >>/progress
	    cd /library/bin;make -k DESTROOT=/ tools

# finish off the usr/bin, ucb, and etc areas in parallel and wait
	@echo Tools in ${DESTROOT}/usr/bin >/tmp/usr.bin.out ; \
	    cd /library/usr.bin ; \
	    make -k DESTROOT=/ tools2 >>/tmp/usr.bin.out 2>&1 & \
	echo Tools in ${DESTROOT}/usr/ucb >/tmp/ucb.out ; \
	    cd /library/ucb ; \
	    make -k DESTROOT=/ tools2 >>/tmp/ucb.out 2>&1 & \
	echo Tools in ${DESTROOT}/etc >/tmp/etc.out ; \
	    cd /library/etc ; \
	    make -k DESTROOT=/ tools >>/tmp/etc.out 2>&1 & \
	wait
	cat /tmp/usr.bin.out /tmp/ucb.out /tmp/etc.out
	rm -f /tmp/usr.bin.out /tmp/ucb.out /tmp/etc.out

