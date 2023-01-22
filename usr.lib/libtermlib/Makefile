# @(#)Makefile	1.4	ULTRIX	11/22/84
#
#	Makefile	4.1	83/06/27
#
CFLAGS=	-O -DCM_N -DCM_GT -DCM_B -DCM_D
LD = ld
DESTROOT=
OBJS=	termcap.o tgoto.o tputs.o

.c.o:
	$(CC) $(CFLAGS) -c -p $*.c
	$(LD) $(LDFLAGS) -x -r -o profiled/$*.o $*.o
	$(CC) $(CFLAGS) -c $*.c
	$(LD) $(LDFLAGS) -X -r $*.o
	mv a.out $*.o

termcap.a termcap_p.a: termcap.o tgoto.o tputs.o
	ar cr termcap.a termcap.o tgoto.o tputs.o
	cd profiled; ar cr ../termcap_p.a termcap.o tgoto.o tputs.o

clean:
	-rm -f *.o profiled/*.o
	-rm -f termcap.a termcap_p.a

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

install: termcap.a termcap_p.a
	install -c termcap.a ${DESTROOT}/usr/lib/libtermcap.a
	-rm -f ${DESTROOT}/usr/lib/libtermlib.a
	ln ${DESTROOT}/usr/lib/libtermcap.a ${DESTROOT}/usr/lib/libtermlib.a
	ranlib ${DESTROOT}/usr/lib/libtermcap.a
	install -c termcap_p.a ${DESTROOT}/usr/lib/libtermcap_p.a
	-rm -f ${DESTROOT}/usr/lib/libtermlib_p.a
	ln ${DESTROOT}/usr/lib/libtermcap_p.a ${DESTROOT}/usr/lib/libtermlib_p.a
	ranlib ${DESTROOT}/usr/lib/libtermcap_p.a

tools:
	for i in ${OBJS}; do \
		echo $$i; \
		$(CC) $(CFLAGS) -c `basename $$i .o`.c; \
		$(LD) $(LDFLAGS) -X -r $$i; \
		mv a.out $$i; \
	done
	rm -f termcap.a
	ar cr termcap.a ${OBJS}
	rm -f ${OBJS}
	install -c termcap.a ${DESTROOT}/usr/lib/libtermcap.a
	-if [ -f ${DESTROOT}/usr/lib/libtermlib.a ]; \
	then \
		rm -f ${DESTROOT}/usr/lib/libtermlib.a; \
	else \
		true; \
	fi
	ln ${DESTROOT}/usr/lib/libtermcap.a ${DESTROOT}/usr/lib/libtermlib.a
	ranlib ${DESTROOT}/usr/lib/libtermcap.a

VGRIND=	csh /usr/ucb/vgrind
vgrind:
	cp /dev/null index
	${VGRIND} -h "Termcap library" termcap.c tputs.c tgoto.c
	${VGRIND} -h "Termcap library" -x index
