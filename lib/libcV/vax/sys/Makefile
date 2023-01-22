#		@(#)Makefile	1.3	(ULTRIX)	5/22/86
#
# @(#)Makefile	4.19 (Berkeley) 7/31/83
#
# 	Modifications
#

OBJS=	_getpgrp.o _ioctl.o _kill.o _lseek.o _mount.o \
	_open.o _ptrace.o _read.o _setpgrp.o _sigvec.o _umount.o \
	_wait.o _write.o

.c.o:
	/lib/cpp -E -DPROF  $*.c | ${AS} -o $*.o
	-ld -x -r -o profiled/$*.o $*.o
	/lib/cpp -E  $*.c | ${AS} -o $*.o
	-ld -x -r -o gfloat/$*.o $*.o
	/lib/cpp -E  $*.c | ${AS} -o $*.o
	-ld -x -r $*.o
	mv a.out $*.o

syslib gfloat/syslib profiled/syslib: gfloat profiled ${OBJS}
	@for d in . profiled gfloat; \
	do ( \
		echo "Creating libcV/vax/sys/$$d/syslib" ; \
		cd $$d; ar cru syslib ${OBJS} \
	) ; done
#	@echo "building gfloat syslib"
#	@cd gfloat; ar cru syslib ${OBJS}
#	@echo "buiding normal syslib"
#	@ar cru syslib ${OBJS}

gfloat profiled:
	mkdir $@

clean:
	rm -rf syslib profiled gfloat *.o

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
