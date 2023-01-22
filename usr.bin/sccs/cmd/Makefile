
# 	@(#)Makefile	1.4	(ULTRIX)	1/8/85

#	 This is the description file used by the make(I) command
#	 to reconstruct all or part of SCCS

#	(All Macro Definitions are described on line following definition)

HDRS = ../hdr/*.h

HDRDIR = ../hdr
#	Directory containing SCCS header files
LIBDIR = ../lib
#	SCCS common object library directory
DEBUG = 
#	C compiler debug option
ARGS =
#
FECFLAGS = -DUIDUSER
#	C compiler
CFLAGS = $(DEBUG) -c -O -I$(HDRDIR)
#	Flags used by C compiler
FFLAG =
#	Floating-point option
LDFLAGS =  -n $(FFLAG)
#	Load flags used by C compiler
LIBES = ../libPW/libPW.a
#	Libraries used during loading

all:	admin bdiff comb  delta get help prs rmchg \
	sccs unget val vc what 


admin:	admin.o $(LIBDIR)/comobj.a $(LIBES) 
	$(CC) $(LDFLAGS) -o admin admin.o $(LIBDIR)/comobj.a $(LIBES)

admin.o:	admin.c $(HDRS)
	$(CC) $(CFLAGS) admin.c

bdiff:	bdiff.o $(LIBDIR)/comobj.a $(LIBES)
	$(CC) $(LDFLAGS) -o bdiff bdiff.o $(LIBDIR)/comobj.a $(LIBES)

bdiff.o:	bdiff.c $(HDRS)
	$(CC) $(CFLAGS) bdiff.c

comb:	comb.o $(LIBDIR)/comobj.a $(LIBES)
	$(CC) $(LDFLAGS) -o comb comb.o $(LIBDIR)/comobj.a $(LIBES)

comb.o:		comb.c $(HDRS)
	$(CC) $(CFLAGS) comb.c

delta:	delta.o $(LIBDIR)/comobj.a $(LIBES)
	$(CC) $(LDFLAGS) -o delta delta.o $(LIBDIR)/comobj.a $(LIBES)

delta.o:	delta.c $(HDRS)
	$(CC) $(CFLAGS) delta.c

get:	get.o $(LIBDIR)/comobj.a $(LIBES)
	$(CC) $(LDFLAGS) -o get get.o $(LIBDIR)/comobj.a $(LIBES)

get.o:		get.c $(HDRS)                             
	$(CC) $(CFLAGS) get.c

help:	help.o $(LIBDIR)/comobj.a $(LIBES)
	$(CC) $(LDFLAGS) -o help help.o $(LIBDIR)/comobj.a $(LIBES)

help.o:		help.c $(HDRS)
	$(CC) $(CFLAGS) help.c

prs:	prs.o $(LIBDIR)/comobj.a $(LIBES)
	$(CC) $(LDFLAGS) -o prs prs.o $(LIBDIR)/comobj.a $(LIBES)

prs.o:		prs.c $(HDRS)
	$(CC) $(CFLAGS) prs.c

rmchg:	rmchg.o $(LIBDIR)/comobj.a $(LIBES)
	$(CC) $(LDFLAGS) -o rmchg rmchg.o $(LIBDIR)/comobj.a $(LIBES)

rmchg.o:	rmchg.c $(HDRS)
	$(CC) $(CFLAGS) rmchg.c

sccs:	sccs.o
	$(CC) -o sccs sccs.o

sccs.o:
	$(CC) $(CFLAGS) $(FECFLAGS) sccs.c


unget:	unget.o $(LIBDIR)/comobj.a $(LIBES)
	$(CC) $(LDFLAGS) -o unget unget.o $(LIBDIR)/comobj.a $(LIBES)

unget.o:	unget.c $(HDRS)
	$(CC) $(CFLAGS) unget.c

val:	val.o $(LIBDIR)/comobj.a $(LIBES)
	$(CC) $(LDFLAGS) -o val val.o $(LIBDIR)/comobj.a $(LIBES)

val.o:	val.c $(HDRS)                                 
	$(CC) $(CFLAGS) val.c

vc:	vc.o $(LIBDIR)/comobj.a
	$(CC) $(LDFLAGS) -o vc vc.o $(LIBDIR)/comobj.a $(LIBES)

vc.o:	vc.c $(HDRS)                                 
	$(CC) $(CFLAGS) vc.c

what:	what.o $(LIBDIR)/comobj.a $(LIBES)
	$(CC) $(LDFLAGS) -o what what.o $(LIBDIR)/comobj.a  $(LIBES)

what.o:		what.c $(HDRS)                               
	$(CC) $(CFLAGS) what.c

$(LIBDIR)/comobj.a:
	cd ../lib ; make TARGET=vax

$(LIBES):
	cd ../libPW ; make TARGET=vax

clean:		# Procedure to clean up unwanted files
	-rm -f *.o
	-rm -f  admin	bdiff	comb	delta	\
		get	help	prs	rmchg	sc\
		sccs 	unget	val	vc	what 

clobber: clean
	-rm -f [a-z]*

sccsinfo:
	-sccs info

sccsget:
	-sccs get SCCS
