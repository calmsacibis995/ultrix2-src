#	1.2	Makefile	84/10/11
DESTROOT=
CFLAGS = -w -g
LDFLAGS=

iconx:		init.o \
		../functions/Lib ../operators/Lib ../lib/Lib ../rt/Lib
	ld $(LDFLAGS) -X -o iconx \
	   ../functions/Lib ../operators/Lib ../lib/Lib ../rt/Lib -lc

init.o:		../h/rt.h ../h/err.h
interp.o:	../h/defs.s
start.o:	../h/defs.s
	cat ../h/defs.s start.s >t.c
	cc -E t.c | grep -v "^#" >t.s
	as -o start.o t.s
	rm -f t.c t.s
.s.o:
	cat ../h/defs.s $< >t.c
	cc -E t.c | grep -v "^#" >t.s
	as -o $@ t.s
	rm -f t.c t.s

Listall:
	@pr *.[cs]
	@date >List

List:		init.c interp.s pstart.s pstop.s start.s
	@pr $?
	@date >List

clean:
	@rm -f iconx *.o
clobber: clean
	@rm -f *.c Makefile
sccsinfo:
	@sccs info
sccsget:
	@sccs get -s SCCS
all:
install: all


dist-clean:
	rm -f `gcomp Makefile *.s *.c *.gen`

