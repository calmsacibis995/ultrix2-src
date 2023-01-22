#	1.2	Makefile	84/10/11
#   Make the user command and the bidding program
#
DESTROOT=
all: dbid dsh

dbid: getloadave.o getpath.o dbid.o error.o getrc.o ipc.o
	cc getloadave.o getpath.o dbid.o error.o getrc.o ipc.o -o dbid

dsh: dsh.o error.o getrc.o ipc.o getwd.o getbids.o
	cc -o dsh dsh.o error.o getrc.o ipc.o getwd.o getbids.o

dbid.o: dsh.h dbid.h
getbids.o: dsh.h dbid.h
dsh.o: dsh.h

#
#   install them
#
install: all
	@install -c -s dbid ${DESTROOT}/usr/new/dbid
	@install -c -s dsh ${DESTROOT}/usr/new/dsh

#
#   clean out the reproducible files
#
clean:
	@rm -f *.o dbid dsh
clobber: clean
	@rm -f Makefile *.[ch]

sccsget:
	@sccs get -s SCCS

sccsinfo:
	@sccs info
