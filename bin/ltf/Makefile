#  @(#)Makefile	1.3	ULTRIX	10/3/86
# ltf - makefile
# 
# 02-Apr-86
#

DESTROOT= 

HDRS=  filetypes.h ltfdefs.h ltferrs.h 

OBJS=	filenames.o filetype.o initvol.o ltf.o ltfvars.o mstrcmp.o \
	odm.o putdir.o putfile.o scantape.o statchk.o xtractf.o

INSTFLAGS= 

CFLAGS= -O 

CC= /bin/cc

all:	ltf

#	Make the LTF
#
ltf:	${OBJS}
	${CC} ${CFLAGS} ${OBJS} -o ltf

install: all
	install ${INSTFLAGS} -c -s ltf ${DESTROOT}/bin/ltf

clean:
	rm -f *.out ltf core ${OBJS}

clobber:
	-for i in `ls -a`; do \
		if test -f $$i; \
		then \
		rm -f $$i; \
	fi \
	done

sccsget:
	sccs get SCCS

sccsinfo:
	@sccs info

