#  @(#)Makefile	1.16	ULTRIX	10/3/86
#
DESTROOT =
INSTALLDIR = ${DESTROOT}/usr/ucb
INSTALLIB = ${DESTROOT}/usr/lib
SRCDIR=	../src
PX_HEADER = px_header
NM = nm -go
RM = rm -f
CFLAGS = -O -I${SRCDIR}

#
WHOAMI = pi
VERSION = 3.1
DATE = 2/13/86
LIBDIR = ${DESTROOT}/usr/lib
ERRORSTRINGS = ${WHOAMI}${VERSION}strings
HOWFILE = how_${WHOAMI}
#

PROGS =	pc pix pmerge
FILES =	pcexterns.o how_pc how_pi how_pix how_pxp
TOOLS = pc pc2 pc3 pcexterns.o

all: pix px_header pc pc2 pc3 pmerge pcexterns.o

tools: ${TOOLS}
	install -c -m 755 pc ${INSTALLDIR}
	cd pc2; make DESTROOT=${DESTROOT} install
	install -c -m 755 -s pc3 ${INSTALLIB}
	install -c -m 644 pcexterns.o ${INSTALLIB} 

install: all pxref.p
	dd if=px_header of=temp conv=sync
	install -m 644 temp ${INSTALLIB}/${PX_HEADER}
	for i in ${PROGS}; do (install -s $$i ${INSTALLDIR}/$$i ); done
	for i in ${FILES}; do (install -m 644 -c $$i ${INSTALLIB}/$$i ); done
	install -s pc3 ${INSTALLIB}/pc3
	cd pc2; make DESTROOT=${DESTROOT} install
	-pc -w pxref.p
	install -s a.out ${INSTALLDIR}/pxref

config.c: ${SRCDIR}/CONFIG.c Makefile
	rm -f config.c
	sed -e "s?VERSION?${VERSION}?g" \
	    -e "s?DATE?${DATE}?g" \
	    -e "s?INSTALLDIR?${INSTALLDIR}?g" \
	    -e "s?LIBDIR?${LIBDIR}?g" \
	    -e "s?ERRORSTRINGS?${ERRORSTRINGS}?g" \
	    -e "s?HOWFILE?${HOWFILE}?g" \
	    < ${SRCDIR}/CONFIG.c >config.c

pix: pix.c config.o
	${CC} ${CFLAGS} -o pix pix.c config.o

pmerge: pmerge.c
	${CC} ${CFLAGS} -o pmerge pmerge.c

px_header: px_header.c ${SRCDIR}/objfmt.h config.o
	${CC} ${CFLAGS} -o px_header px_header.c config.o -N
	strip px_header

pc2: /tmp
	cd pc2; make

pc3: pc3.c pc3.h ${SRCDIR}/pstab.h
	${CC} ${CFLAGS} pc3.c -o pc3

pxref: pxref.p
	pc -w pxref.p -o pxref

pc: pc.c
	${CC} ${CFLAGS} -o pc pc.c

LIBPC 	= /usr/lib/libpc.a
LIBC	= /lib/libc.a
LIBM	= /usr/lib/libm.a
LIBG	= /usr/lib/libg.a
D_LIBPC	= $(DESTROOT)$(LIBPC)
D_LIBC	= $(DESTROOT)$(LIBC)
D_LIBM	= $(DESTROOT)$(LIBM)
D_LIBG	= $(DESTROOT)$(LIBG)
LIBRARIES = $(D_LIBPC) $(D_LIBC) $(D_LIBM) $(D_LIBG)

pcexterns.o: pcexterns.awk ${LIBRARIES}
	( echo -n $(LIBPC):; ${NM} $(D_LIBPC) ) \
		| awk -f pcexterns.awk \
		| sort -t\" +1 -2 -u >pcexterns.s
	( echo -n $(LIBC):; ${NM} $(D_LIBC) ) \
		| awk -f pcexterns.awk \
		| sort -t\" +1 -2 -u >>pcexterns.s
	( echo -n $(LIBM):; ${NM} $(D_LIBM) ) \
		| awk -f pcexterns.awk \
		| sort -t\" +1 -2 -u >>pcexterns.s
	( echo -n $(LIBG):; ${NM} $(D_LIBG) ) \
		| awk -f pcexterns.awk \
		| sort -t\" +1 -2 -u >>pcexterns.s
	as pcexterns.s -o pcexterns.o
	${RM} pcexterns.s

opcode.h: ${SRCDIR}/OPnames.h ${SRCDIR}/opc.c
	${RM} opcode.h
	cc ${SRCDIR}/opc.c -o opc
	opc >opcode.h
	${RM} opc

picture: ${SRCDIR}/OPnames.h ${SRCDIR}/pic.c
	${RM} picture
	cc ${SRCDIR}/pic.c -o pic
	pic >picture
	${RM} pic

clean:
	${RM} pix px_header pxref pmerge pc pc3 \
	pcexterns.s *.o config.c errs opc opc.c pic pic.c picture
	cd pc2; make clean

clobber: clean
	${RM} how* *.[chp] *.awk Makefile
	cd pc2; make clobber

sccsinfo:
	sccs info
	cd pc2; make sccsinfo

sccsget:
	sccs get SCCS
	cd pc2; sccs get Makefile; make sccsget

print: picture
	@pr READ_ME makefile picture
	@ls -l | pr
	@pr pix.c pmerge.c pxref.p pc.c pc3.h pc3.c
