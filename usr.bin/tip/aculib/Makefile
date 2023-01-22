# @(#)Makefile	1.6   ULTRIX  4/18/85
#
# make file for tip device drivers
#
# Current drivers:
#       BIZCOMP
#       DEC DF02-AC, DF03-AC
#       DEC DN-11/Able Quadracall
#       VENTEL 212+ (w/o echo)
#       VADIC 831 RS232 adaptor
#       VADIC 3451
#       DEC DF112
#       Generic (a way of handling other modems)

CFLAGS= -O -DGENACU -DONDELAY -I../
OBJS=   remcap.o biz22.o biz31.o df.o dn11.o ventel.o v831.o v3451.o df112.o generic.o gen.o
DESTROOT =

aculib.a: ${OBJS}
	ar cu aculib.a *.o
	ranlib aculib.a

${OBJS}: ../tip.h

clean:
	rm -f aculib.a *.o core errs a.out

clobber: clean
	rm -f *.c Makefile

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

.DEFAULT:
	sccs get $<
