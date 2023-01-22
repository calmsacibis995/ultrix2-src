#	1.2	Makefile	84/10/11
DESTROOT=
CFLAGS = -O -w

ulink:	        builtin.o code.o glob.o lex.o mem.o opcode.o sym.o ulink.o
	cc -n -o ulink *.o

builtin.o:      ulink.h
code.o:         ulink.h opcode.h datatype.h code.h
glob.o:         ulink.h opcode.h
lex.o:          ulink.h opcode.h
mem.o:          ulink.h
opcode.o:       ulink.h opcode.h
sym.o:          ulink.h
ulink.o:        ulink.h

Listall:
	@pr *.h *.c
	@date >List

List:		code.h datatype.h opcode.h ulink.h \
	        builtin.c code.c glob.c lex.c mem.c opcode.c sym.c ulink.c
	@pr $?
	@date >List
clean:
	@rm -f *.o ulink
clobber: clean
	@rm -f *.[ch] *.h.gen Makefile
sccsinfo:
	@sccs info
sccsget:
	@sccs get -s SCCS
all:
install: all

dist-clean: clean
	rm -f `gcomp Makefile *.h *.c *.gen`
