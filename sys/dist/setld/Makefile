# Makefile for setld and friends
#  @(#)Makefile	1.8	ULTRIX	1/9/87
#
#	001	ccb	oct2 1986
#	add in mop, inet, bsc, umail, nfs scp's.
#	add in workstation scp's

MAINS=setld ready
SCPS=ULTACCT020.scp ULTBASE020.scp ULTBIN020.scp ULTCOMM020.scp\
	ULTBSC020.scp ULTINET020.scp ULTMOP020.scp\
	ULTNFS020.scp ULTUMAIL020.scp\
	ULTDCMT020.scp ULTEXER020.scp ULTF77020.scp ULTMAN020.scp\
	ULTPASCAL020.scp ULTPGMR020.scp ULTSCCS020.scp ULTUUCP020.scp\
	ULXAPL020.scp ULXBASE020.scp ULXBIB020.scp ULXCOURIER020.scp\
	ULXCPM020.scp ULXDOC020.scp ULXEDIT020.scp\
	ULXF77020.scp ULXGAMES020.scp\
	ULXHYPER020.scp ULXICON020.scp ULXINGRES020.scp ULXLEARN020.scp\
	ULXLISP020.scp ULXMH020.scp ULXMOD2020.scp ULXNEWS020.scp\
	ULXNOTES020.scp ULXRCS020.scp ULXSHELLS020.scp ULXSPMS020.scp\
	ULXTOOLS020.scp ULXVARIAN020.scp ULXMAN020.scp\
	UWDDEMOS010.scp UWDGKS010.scp\
	UWDQIL010.scp UWDX010.scp UWVDEMOS010.scp UWVGKS010.scp UWVX010.scp\
	ULCCRYPT020.scp

WSfiles=UWDDEMOS010.ctrl UWDGKS010.ctrl UWDQIL010.ctrl UWDX010.ctrl\
	UWVDEMOS010.ctrl UWVGKS010.ctrl UWVX010.ctrl UWV.image UWD.image\
	UWVX010.inv UWVGKS010.inv UWVDEMOS010.inv\
	UWDQIL010.inv UWDX010.inv UWDGKS010.inv UWDDEMOS010.inv


sccsget:
	sccs get SCCS

install:	$(MAINS) $(SCPS) $(WSfiles)
	install -c -m 755 setld $(DESTROOT)/etc
	install -c -m 755 ready $(DESTROOT)/usr/adm/bin
	for file in $(SCPS); \
	do \
		install -c -m 744 $$file $(DESTROOT)/usr/sys/dist; \
	done
	for file in $(WSfiles); \
	do \
		install -c -m 644 $$file $(DESTROOT)/usr/sys/dist; \
	done


