#	makefile for ctrace, ctc, ctcr
#	@(#)Makefile	1.3  	ULTRIX	3/24/86
#	(System V)  ctrace.mk  1.3

# installation directory
BIN = ${DESTROOT}/usr/bin

# directory for runtime.c source
LIB = ${DESTROOT}/usr/lib/ctrace

# preprocessor symbols
CC=cc
CFLAGS = -O -DLIB=\"${LIB}
YFLAGS = -d

SOURCE	 = constants.h global.h main.c parser.y \
	   scanner.l lookup.c trace.c runtime.c ctcr
OTHER	 = Makefile
OBJECTS	 = main.o parser.o scanner.o lookup.o trace.o

all:	ctrace

ctrace: ${OBJECTS}
	${CC} ${OBJECTS} -o $@

install: all
	install -c -m 755 -o bin -g bin -s ctrace ${BIN}

	rm -f ${BIN}/ctc
	install -c -m 755 -o bin -g bin ctcr ${BIN}

	ln ${BIN}/ctcr ${BIN}/ctc

	-(if [ ! -d ${LIB} ] ;\
	then \
	    mkdir ${LIB} ;\
	else \
	    true ;\
	fi)
	install -c -m 644 -o bin -g bin runtime.c ${LIB}

clean:
	rm -f ctrace *.o core log a.out y.tab.h y.output

clobber: clean
	sccs clean

tags:	${SOURCE}
	ctags ${SOURCE}

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
