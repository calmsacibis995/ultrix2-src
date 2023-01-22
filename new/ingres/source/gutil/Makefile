#		@(#)Makefile	1.6	(ULTRIX)	2/13/85
#
#  Makefile for INGRES gutil library
#
#
# 	David L Ballenger, 15-Jan-1985
# 001	Modify Makefile for use in ULTRIX-32/32m build procedures.
#

INGRES=	../..
H=	../h
LIB=	${INGRES}/lib
CPUTYPE=VAX

OBJS=	append.o atof.o bequal.o bitcnt.o bitpos.o cat.o \
	concat.o freebuf.o ftoa.o fullwait.o \
	getufield.o iocv.o itoa.o length.o \
	xalloc.o need.o oatoi.o pad.o pmove.o \
	prargs.o scompare.o sequal.o set_si_buf.o set_so_buf.o \
	setflag.o trace.o atol.o smove.o \
	lprintf.o clrmem.o xputchar.o ztack.o syserr.o \
	min.o xfree.o bmove.${CPUTYPE}.o perror.o locv.o

SRCS=	append.c atof.c bequal.c bitcnt.c bitpos.c cat.c \
	concat.c freebuf.c ftoa.c fullwait.c \
	bmove.PDP.s bmove.VAX.s \
	getufield.c iocv.c itoa.c length.c \
	xalloc.c need.c oatoi.c pad.c pmove.c \
	prargs.c scompare.c sequal.c set_si_buf.c set_so_buf.c \
	setflag.c trace.c atol.c smove.c \
	xputchar.c ztack.c syserr.c \
	min.c xfree.c lprintf.c clrmem.c perror.c locv.s

FILES=	Makefile ${SRCS}

CFLAGS=	-I${H} -O
AR=	-ar
ARFLAGS=urv
RANLIB=	ranlib
PR=	print
VGRIND=	csh /usr/ucb/vgrind
HEADER=	Gutil Library
CP=	cp
CHMOD=	chmod
LIBMODE=664
INSTALL=install

all: gutil.a

gutil.a: ${OBJS}
	${AR} ${ARFLAGS} gutil.a $?
	${RANLIB} gutil.a

install: all

new:
	rm -f gutil.a

clean:  new
	rm -f [0-m]*.o
	rm -f [n-z]*.o
	rm -f core 

clobber: clean
	rm -f *.[chmsy] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<

sources: ${SRCS}

${ASDIR}/instrs:
	(cd ${ASDIR}; sccs get instrs)

#${SRCS}: 
#	sccs get $@

save_version:
	@sccs get -s SCCS
	@sccs what ${FILES} | awk '$$2 != "" {print $$1, $$2}' > SCCS/version_${VERSION}

restore_version:
	@echo ${PATH}
	@for i in ${FILES}; do \
		echo "Restore" `grep $$i SCCS/version_${VERSION}`; \
		sccs get -s -r`grep $$i SCCS/version_${VERSION} | awk '{print $$2, $$1}'`; \
	done

show_current:
	@echo ${PATH}
	@sccs what ${FILES} | awk '$$2 != "" {printf "%s  %s\t", $$1, $$2}'
	@echo

print: sources
	${PR} Makefile *.[hm] *.[csy]

vgrind: sources
	cp /dev/null index
	${VGRIND} -h "${HEADER} (Release 7)" -n Makefile
	${VGRIND} -h "${HEADER} (Release 7)" *.[csy]
	${VGRIND} -h "${HEADER} (Release 7)" -x index

depend:
	grep '^#[ 	]*include' ${SRCS} | sed -n -f ../h/depend.sed > makedep
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it

bequal.o:$H/useful.h
xalloc.o:$H/useful.h
sequal.o:$H/useful.h
set_si_buf.o:$H/ingres.h
set_so_buf.o:$H/ingres.h
trace.o:$H/useful.h
lprintf.o:$H/useful.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
