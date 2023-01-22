#		@(#)Makefile	1.5	(ULTRIX)	5/22/86
#
#
#
# 	Modifications
#
# 002	David L Ballenger, 23-Sep-1985
#	Remove lockf, plock, and ustat, they are now part of the regular 
#	libc.
#
#	David L Ballenger, 31-May-1985
# 001	Remove ulimit and utime, they are now included in the regular libc.
#

CFLAGS=	-O -Y
DESTROOT=
LIBC=	../../libc
GEN=	${LIBC}/gen
COMPAT41= ${LIBC}/compat-4.1

COMMON=	alarm.c gtty.c nice.c stty.c time.c

OBJS=	alarm.o getpgrp.o gtty.o ioctl.o kill.o \
	lseek.o mount.o nice.o open.o ptrace.o read.o \
	setpgrp.o signal.o stty.o time.o times.o \
	umount.o wait.o write.o

.c.o:
	${CC} -p ${CFLAGS} -c $*.c
	-ld -X -r -o profiled/$*.o $*.o
	${CC} ${CFLAGS} -Mg -c $*.c
	-ld -x -r -o gfloat/$*.o $*.o
	${CC} ${CFLAGS} -c $*.c
	-ld -x -r $*.o
	mv a.out $*.o

syslib profiled/syslib gfloat/syslib: gfloat profiled ${OBJS}
	@for d in . profiled gfloat; \
	do ( \
		echo "Creating libcV/sys/$$d/syslib" ; \
		cd $$d; ar cru syslib ${OBJS} \
	) ; done

gfloat profiled:
	mkdir $@

clean:
	rm -rf syslib profiled gfloat *.o

clobber: clean
	rm -f Makefile :errfix *.c 

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

alarm.c time.c:
	ln -s ${GEN}/$@

gtty.c nice.c stty.c:
	ln -s ${COMPAT41}/$@

