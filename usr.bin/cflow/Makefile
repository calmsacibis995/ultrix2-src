#	@(#)cflow.mk	1.3
#	3.0 SID #	1.2
DESTROOT =
TESTDIR = .
CFLAGS = -O -DFLEXNAMES
FILES = Makefile README cflow.sh dag.c lpfx.c nmf.c flip.c
ALL = dag lpfx nmf flip
LINT = ../lint
MIP = ../../lib/mip
BIN = ${DESTROOT}/usr/bin
LIB = ${DESTROOT}/usr/lib

all:	$(ALL)

dag:	dag.c getopt.o
	$(CC) $(CFLAGS) dag.c getopt.o -o $(TESTDIR)/dag

lpfx:	lpfx.c hash.c $(LINT)/lmanifest $(MIP)/manifest getopt.o
	$(CC) $(CFLAGS) -I$(LINT) -I$(MIP) lpfx.c hash.c getopt.o -o $(TESTDIR)/lpfx

nmf:	nmf.c getopt.o
	$(CC) $(CFLAGS) nmf.c getopt.o -o $(TESTDIR)/nmf

flip:	flip.c getopt.o
	$(CC) $(CFLAGS) flip.c getopt.o -o  $(TESTDIR)/flip

install:	$(ALL)
	cp cflow.sh $(BIN)/cflow
	cp dag $(LIB)/dag
	cp lpfx $(LIB)/lpfx
	cp nmf $(LIB)/nmf
	cp flip $(LIB)/flip
	chgrp staff $(BIN)/cflow $(LIB)/dag $(LIB)/lpfx $(LIB)/nmf $(LIB)/flip
	/etc/chown root $(BIN)/cflow $(LIB)/dag $(LIB)/lpfx $(LIB)/nmf $(LIB)/flip
	chmod 755 $(BIN)/cflow $(LIB)/dag $(LIB)/lpfx $(LIB)/nmf $(LIB)/flip

clean:
	-rm -f *.o *.out core dag lpfx nmf flip

clobber:	clean
	-rm -f $(ALL)

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

