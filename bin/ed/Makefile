# SCCSID:	@(#)Makefile	1.4	2/9/85
DESTROOT = 

INSDIR = ${DESTROOT}/bin
IFLAG = -n
CFLAGS = -O
LDFLAGS = -s ${IFLAG}
FIXSOURCE = edfun
SOURCES = ed.c

all:	ed

${SOURCES}:
	sccs get $@

ed:	ed.s
	${CC} ${LDFLAGS} -o ed ed.s

ed.s:	${SOURCES}
	${CC} -S ${CFLAGS} ed.c

install: ed
	cp ed ${INSDIR}/ed
	chmod 755 ${INSDIR}/ed
#	chog bin ${INSDIR}/ed
	rm -f ${INSDIR}/red ${INSDIR}/e
	ln ${INSDIR}/ed ${INSDIR}/red
	ln ${INSDIR}/ed ${INSDIR}/e
	make clean

clean:
	  rm -f ed.s *.o ed

clobber:  clean
	  sccs clean

sccsinfo:
	  sccs info

sccsget:
	  sccs get SCCS
