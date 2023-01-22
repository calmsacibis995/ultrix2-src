

# 	@(#)Makefile	1.4	(ULTRIX)	1/8/85

DEBUG = 
CC = cc
CCFLAGS = $(DEBUG) -O -I../hdr

LD = -ld
LDFLAGS = -x

HDRS = ../hdr/*.h
.c.o:
	$(CC) -c $(CCFLAGS) $<


OBJ  = chkid.o chksid.o del_ba.o  date_ba.o dodelt.o \
       del_ab.o dofile.o dohist.o doie.o dolist.o eqsid.o flushto.o newstats.o \
       permiss.o logname.o pf_ab.o date_ab.o rdmod.o setup.o \
       sid_ab.o sid_ba.o sidtoser.o sinit.o stats_ab.o \
       fmterr.o getline.o putline.o auxf.o cmrcheck.o error.o filehand.o \
       fmalloc.o gf.o xcreat.o xmsg.o deltack.o

comobj.a:	$(OBJ) 
	-rm -f comobj.a tmp.a
	ar r comobj.a $(OBJ)
	ranlib comobj.a


# chkid.o chksid.o del_ba.o  date_ba.o dodelt.o \
# del_ab.o dofile.o dohist.o doie.o dolist.o eqsid.o flushto.o newstats.o \
# permiss.o logname.o pf_ab.o date_ab.o rdmod.o setup.o \
# sid_ab.o sid_ba.o sidtoser.o sinit.o stats_ab.o \
# fmterr.o getline.o putline.o auxf.o:  $(OBJ) 


clean:
	-rm -f *.o

clobber: 
	-rm -f *.o *.a
	-rm -f [a-z]*

sccsinfo:
	-sccs info

sccsget:
	-sccs get SCCS
