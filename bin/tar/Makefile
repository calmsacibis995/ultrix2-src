#
# tar/mdtar -  makefile - Make tar/mdtar
#
#	@(#)Makefile	1.1	(ULTRIX)	5/22/86
#
# 15-May-86	Jeff Fries
#
#	Commands to make tar -
#
#	Ultrix-32 --->	make tar
#
#	The above insures that the correct version and default devices
#	are compiled.
#
#
# To make "mdtar" available, a link to "tar" is created in
# ${DESTROOT}/bin 
#
DESTROOT=
FILES = command.c readtape.c tar.c tar.h writetape.c Makefile 

MLNo = tar.o
COV1o = command.o
ROV1o = readtape.o
WOV1o = writetape.o
LIBCo = fprintf.o fread.o fseek.o malloc.o printf.o cvt.o \
      atof.o ctime.o ecvt.o doscan.o fls#buf.o fwrite.o

DFLAGS =
CFLAGS = -O $(DFLAGS)

tar:	$(MLNo) $(COV1o) $(ROV1o) $(WOV1o)
	$(CC) $(CFLAGS) $(MLNo) $(COV1o) $(ROV1o) $(WOV1o) -o tar

install: tar
	install -c -s tar ${DESTROOT}/bin
	rm -f ${DESTROOT}/bin/mdtar
	ln ${DESTROOT}/bin/tar ${DESTROOT}/bin/mdtar

clean:
	rm -f *.o tar

clobber: clean
	rm -f *.[ch] Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
