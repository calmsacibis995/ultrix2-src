# $Header$
#
DESTROOT=
CFLAGS	      = -D$(VERSION) -I../../../include -O

DEST	      = $(DESTROOT)/usr/new

EXTHDRS	      = ../../../include/getarg.h \
		../../../include/macro.h \
		../../../include/null.h \
		../../../include/path.h \
		../../../include/slist.h \
		../../../include/spms.h \
		../../../include/system.h \
		../../../include/yesno.h \
		/usr/include/ctype.h \
		/usr/include/signal.h \
		/usr/include/stdio.h \
		/usr/include/sys/dir.h \
		/usr/include/sys/file.h \
		/usr/include/sys/types.h

HDRS	      = Mkmf.h \
		defaultsfx.h \
		hash.h \
		suffix.h \
		target.h

LDFLAGS	      =

LIBS	      = ../../../lib/libslist.a \
		../../../lib/libspms.a

LINKER	      = cc

LINTFLAGS     = -D$(VERSION) -I../../../include

LINTLIST      = ../../../lib/llib-lslist.ln \
		../../../lib/llib-lpdb.ln \
		../../../lib/llib-lspms.ln \
		-lc

MAKEFILE      = Makefile

OBJS	      = Mkmf.o \
		buildlist.o \
		depend.o \
		editmf.o \
		hash.o \
		iolin.o \
		macro.o \
		misc.o \
		suffix.o

PRINT	      = pr

PROGRAM	      = mkmf

SRCS	      = Mkmf.c \
		buildlist.c \
		depend.c \
		editmf.c \
		hash.c \
		iolin.c \
		macro.c \
		misc.c \
		suffix.c

VERSION	      = V4BSD

all:		$(PROGRAM)

$(PROGRAM):     $(OBJS) $(LIBS)
		@echo -n "Loading $(PROGRAM) ... "
		@$(LINKER) $(LDFLAGS) $(OBJS) $(LIBS) -o $(PROGRAM)
		@echo "done"

clean:;		@rm -f $(OBJS)

co:;		@co -r$(VERSION) $(HDRS) $(SRCS)

depend:;	@mkmf -f $(MAKEFILE) PROGRAM=$(PROGRAM) DEST=$(DEST)

diff:;		@rcsdiff -r$(VERSION) $(HDRS) $(SRCS)

index:;		@ctags -wx $(HDRS) $(SRCS)

install:	$(PROGRAM)
		@echo Installing $(PROGRAM) in $(DEST)
		@install -s $(PROGRAM) $(DEST)

lint:;	        @lint $(LINTFLAGS) $(SRCS) $(LINTLIST)

print:;		@$(PRINT) $(HDRS) $(SRCS)

program:        $(PROGRAM)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

update:		$(DEST)/$(PROGRAM)

$(DEST)/$(PROGRAM): $(SRCS) $(LIBS) $(HDRS) $(EXTHDRS)
		@make -f $(MAKEFILE) DEST=$(DEST) install tags
