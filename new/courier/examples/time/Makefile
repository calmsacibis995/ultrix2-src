#	1.3	Makefile	84/10/11
CFLAGS = -O
USEROBJS1 = datime.o implicit.o
USEROBJS2 = timex.o explicit.o
SRVROBJS = Time.o Time_server.o
LIBS = /usr/new/lib/libcr.a
TARGET = /usr/new/lib/courier
DESTROOT=

all:	datime timex Time

datime:	$(USEROBJS1)
	cc -o datime $(USEROBJS1) $(LIBS)

implicit.o:	Time.cr
	/usr/new/courier Time.cr
	cc -c $(CFLAGS) Time_client.c
	mv Time_client.o implicit.o

timex:	$(USEROBJS2)
	cc -o timex $(USEROBJS2) $(LIBS)

explicit.o:	Time.cr
	/usr/new/courier -x Time.cr

sccsget:
		@sccs get -s SCCS

sccsinfo:
		@sccs info

Time:	$(SRVROBJS)
	cc -o Time $(SRVROBJS) $(LIBS)

datime.o timex.o Time.o:	Time.h

Time_server.c Time.h:	Time.cr
	/usr/new/courier Time.cr

install:	all
	install -s Time $(DESTROOT)${TARGET}

clean:
	@rm -f *.o Time_*.c Time.h 
clobber: clean
	@rm -f Makefile *.c
