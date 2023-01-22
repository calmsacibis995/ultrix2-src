# @(#)Makefile	1.9	ULTRIX	6/28/85
#
#	Makefile	4.2	83/07/17
#
# makefile for line printer filters and associated programs
#

DESTROOT=
CFLAGS=	-O
LIBDIR=	/usr/lib
CXREF=	/usr/ucb/ctags -x
SRCS=	lpf.c necf.c lqf.c ln01of.c ln03of.c \
	chrtab.c rvsort.c rvcat.c vsort.c vcat.c vpf.c vpsf.c vdmp.c vpltdmp.c \
	vplotf.c vfontinfo.c vwidth.c vfw.c fcvt.c rotate.c rotprt.c
PRINTER=lpf necf lqf ln01of ln03of ln01pp
VARIAN=	rvsort rvcat vsort vcat vpf vpsf vdmp vpltdmp vplotf vfontinfo vwidth \
	vfw fcvt rotate rotprt

all:	printer varian

printer: ${PRINTER}

varian:	${VARIAN}

lpf:	lpf.c
	${CC} -o lpf ${CFLAGS} lpf.c

necf:	necf.c
	${CC} -o necf ${CFLAGS} necf.c

rvsort:	rvsort.c
	${CC} -o rvsort ${CFLAGS} rvsort.c

rvcat:	rvcat.c
	${CC} -o rvcat ${CFLAGS} rvcat.c

vsort:	vsort.c
	${CC} -o vsort ${CFLAGS} vsort.c

vcat:	vcat.c
	${CC} -o vcat ${CFLAGS} vcat.c

vpf:	vpf.c
	${CC} -o vpf ${CFLAGS} vpf.c

vpsf:	vpsf.c
	${CC} -o vpsf ${CFLAGS} vpsf.c

vdmp: vdmp.c
	${CC} -o vdmp ${CFLAGS} vdmp.c

vpltdmp: vpltdmp.c
	${CC} -o vpltdmp ${CFLAGS} vpltdmp.c

vplotf: vplotf.c chrtab.o
	${CC} -o vplotf ${CFLAGS} vplotf.c chrtab.o

vfontinfo: vfontinfo.c
	${CC} -o vfontinfo ${CFLAGS} vfontinfo.c

vwidth: vwidth.c
	${CC} -o vwidth ${CFLAGS} vwidth.c

vfw: vfw.c
	${CC} -o vfw ${CFLAGS} vfw.c

fcvt: fcvt.c
	${CC} -o fcvt ${CFLAGS} fcvt.c

rotate: rotate.c
	${CC} -o rotate ${CFLAGS} rotate.c

rotprt: rotprt.c
	${CC} -o rotprt ${CFLAGS} rotprt.c

ln01of: ln01of.c
	${CC} -o $@ ${CFLAGS} $@.c
ln01pp: ln01pp.c
	${CC} -o $@ ${CFLAGS} $@.c
ln03of: ln03of.c
	${CC} -o $@ ${CFLAGS} $@.c
lqf: lqf.c
	${CC} -o $@ ${CFLAGS} $@.c

install: printer installv
	for i in ${PRINTER}; do \
		install -c -s $$i ${DESTROOT}${LIBDIR}/$$i; done

installv: varian
	install -s vfontinfo ${DESTROOT}/${LIBDIR}/vfontinfo
	install -s vwidth ${DESTROOT}/${LIBDIR}/vwidth
	install -s vfw ${DESTROOT}/${LIBDIR}/vfw
	install -s fcvt ${DESTROOT}/${LIBDIR}/fcvt
	install -s rotate ${DESTROOT}/${LIBDIR}/rotate
	install -s rotprt ${DESTROOT}/${LIBDIR}/rotprt
	install -s rvsort ${DESTROOT}/${LIBDIR}/rvsort
	install -s rvcat ${DESTROOT}/${LIBDIR}/rvcat
	install -s vsort ${DESTROOT}/${LIBDIR}/vsort
	install -s vcat ${DESTROOT}/${LIBDIR}/vcat
	install -s vpf ${DESTROOT}/${LIBDIR}/vpf
	ln -f ${DESTROOT}/${LIBDIR}/vpf ${DESTROOT}/${LIBDIR}/vpfW
	install -s vpsf ${DESTROOT}/${LIBDIR}/vpsf
	ln -f ${DESTROOT}/${LIBDIR}/vpsf ${DESTROOT}/${LIBDIR}/vpsfW
	install -s vdmp ${DESTROOT}/${LIBDIR}/vdmp
	install -s vpltdmp ${DESTROOT}/${LIBDIR}/vpltdmp
	install -s vplotf ${DESTROOT}/${LIBDIR}/vplotf

clean:
	rm -f ${PRINTER} ${VARIAN} *.o

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

print:
	@pr makefile
	@${CXREF} *.c | pr -h XREF
	@pr *.h *.c
