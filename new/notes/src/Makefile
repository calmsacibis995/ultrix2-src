#
# Sccsid = @(#)Makefile	1.6	2/12/85
#
#	Makefile for University of Illinois Notesfiles - release 1.0
#	May 9, 1982
#
#	Ray Essick
#	214 DCL
#	University of Illinois at Urbana-Champaign
#	1304 W. Springfield
#	Urbana, Illinois 61801
#	217-333-7937
#
#       Modified by
#
#	Rick L Spickelmier
#	UC Berkeley
#	321A Cory Hall
#	Berkeley, CA  94704
#	...!ucbvax!ucbcad!ricks
#	ucbcad.ricks@berkeley
#
#	Lou Salkind
#	New York University
#	251 Mercer Street
#	NYC, NY   10012
#	salkind@NYU
#
#	Modified for Binary Kit
#
#	Miriam Amos
#	20-Jun-84
#
DESTROOT=
CFLAGS=
CC=cc
LFLAGS=
TERMCAP=termcap


all:	main nfpipe nfprint nfstats checknotes nfxmit nfrcv \
	mknf rmnf newsinput bnewsinput newsoutput nfarchive


#
#	User programs
#

main:	main.o control.o index.o dropt.o access.o compress.o delete.o  \
	gdate.o pattern.o readem.o adnote.o adresp.o dsply.o gtext.o \
	limindx.o asearch.o help.o mailit.o next.o lrsp.o note.o pagein.o \
	permit.o prtind.o resp.o savtxt.o preptxt.o pageout.o talkto.o times.o \
	tsearch.o cursor.o recsio.o check.o gname.o gtime.o perms.o \
	misc.o strsave.o expand.o pipeopen.o miscio.o
	@echo loading main
	@cc $(CFLAGS) $(LFLAGS) -o main \
		main.o control.o index.o dropt.o access.o compress.o delete.o \
		gdate.o pattern.o readem.o adnote.o adresp.o dsply.o gtext.o \
		limindx.o asearch.o help.o mailit.o next.o lrsp.o note.o \
		pagein.o permit.o prtind.o resp.o savtxt.o preptxt.o pageout.o \
		talkto.o times.o tsearch.o cursor.o recsio.o check.o gname.o \
		gtime.o perms.o misc.o miscio.o strsave.o pipeopen.o expand.o \
		-l$(TERMCAP)

prepass: prepass.o check.o pattern.o strsave.o
	@echo loading prepass
	@cc $(CFLAGS) $(LFLAGS) -o prepass prepass.o check.o pattern.o strsave.o	
nfpipe:	nfpipe.o note.o pagein.o permit.o recsio.o check.o gname.o gtime.o \
	perms.o cursor.o miscio.o misc.o
	@echo loading nfpipe
	@cc $(CFLAGS) $(LFLAGS) -o nfpipe nfpipe.o note.o pagein.o permit.o \
		recsio.o check.o gname.o gtime.o perms.o \
		misc.o cursor.o miscio.o -l$(TERMCAP)

nfstats: nfstats.o pattern.o cursor.o recsio.o check.o gname.o gtime.o \
	perms.o misc.o expand.o strsave.o miscio.o
	@echo loading nfstats
	@cc $(CFLAGS) $(LFLAGS) -o nfstats nfstats.o pattern.o cursor.o \
		recsio.o check.o gname.o gtime.o perms.o misc.o miscio.o \
		expand.o strsave.o -l$(TERMCAP)

nfprint: nfprint.o lprnote.o lrsp.o permit.o pageout.o cursor.o recsio.o \
	check.o gname.o gtime.o perms.o misc.o miscio.o
	@echo loading nfprint
	@cc $(CFLAGS) $(LFLAGS) -o nfprint nfprint.o lprnote.o lrsp.o \
		permit.o pageout.o cursor.o recsio.o check.o gname.o gtime.o \
		perms.o misc.o miscio.o -l$(TERMCAP)

checknotes: checknotes.o permit.o times.o pattern.o next.o lrsp.o recsio.o \
	perms.o gname.o check.o gtime.o \
	misc.o strsave.o expand.o miscio.o cursor.o
	@echo loading checknotes
	@cc $(CFLAGS) $(LFLAGS) -o checknotes checknotes.o permit.o times.o \
		pattern.o next.o lrsp.o recsio.o perms.o gname.o check.o \
		gtime.o misc.o strsave.o miscio.o expand.o cursor.o -l$(TERMCAP)
#
#	Network Transmission and reception
#

nfxmit:	nfxmit.o getnet.o nfalias.o nfsend.o dmpnote.o dmpresp.o pattern.o \
	next.o lrsp.o permit.o pageout.o times.o cursor.o recsio.o check.o \
	gname.o gtime.o perms.o misc.o expand.o strsave.o miscio.o
	@echo loading nfxmit
	@cc $(CFLAGS) $(LFLAGS) -o nfxmit nfxmit.o getnet.o nfalias.o \
		nfsend.o dmpnote.o dmpresp.o pattern.o next.o lrsp.o \
		permit.o pageout.o times.o cursor.o recsio.o check.o \
		gname.o gtime.o perms.o misc.o miscio.o expand.o \
		strsave.o -l$(TERMCAP)

nfbatch: nfbatch.o getnet.o nfalias.o nfsend.o dmpnote.o dmpresp.o pattern.o \
	next.o lrsp.o permit.o pageout.o times.o cursor.o recsio.o check.o \
	gname.o gtime.o perms.o misc.o expand.o strsave.o miscio.o
	@echo loading nfbatch
	@cc $(CFLAGS) $(LFLAGS) -o nfbatch nfbatch.o getnet.o nfalias.o \
		nfsend.o dmpnote.o dmpresp.o pattern.o next.o lrsp.o permit.o \
		pageout.o times.o cursor.o recsio.o check.o gname.o gtime.o \
		perms.o misc.o miscio.o expand.o strsave.o -l$(TERMCAP)

nfrcv: nfrcv.o loadem.o find.o ldmisc.o lrsp.o note.o permit.o resp.o \
	cursor.o recsio.o check.o gname.o gtime.o perms.o misc.o \
	nfcomment.o miscio.o
	@echo loading nfrcv
	@cc $(CFLAGS) $(LFLAGS) -o nfrcv nfrcv.o loadem.o find.o ldmisc.o \
		lrsp.o note.o permit.o resp.o cursor.o recsio.o check.o \
		gname.o gtime.o perms.o misc.o miscio.o nfcomment.o -l$(TERMCAP)

nfrec: nfrec.o loadem.o find.o ldmisc.o lrsp.o note.o permit.o resp.o \
	cursor.o recsio.o check.o gname.o gtime.o perms.o misc.o \
	nfcomment.o miscio.o
	@echo loading nfrec
	@cc $(CFLAGS) $(LFLAGS) -o nfrec nfrec.o loadem.o find.o ldmisc.o \
		lrsp.o note.o permit.o resp.o cursor.o recsio.o check.o \
		gname.o gtime.o perms.o misc.o miscio.o nfcomment.o -l$(TERMCAP)

#
#	Utility programs - create/delete notesfiles, news gateways,
#	and archiving
#

mknf: mknf.o check.o gname.o gtime.o misc.o cursor.o miscio.o
	@echo loading mknf
	@cc $(CFLAGS) $(LFLAGS) -o mknf mknf.o check.o gname.o gtime.o \
		misc.o cursor.o miscio.o -l$(TERMCAP)

rmnf: rmnf.o check.o misc.o cursor.o miscio.o
	@echo loading rmnf
	@cc $(CFLAGS) $(LFLAGS) -o rmnf rmnf.o check.o misc.o \
		cursor.o miscio.o -l$(TERMCAP)

newsinput: newsinput.o datein.o find.o newsgroup.o lrsp.o note.o pagein.o \
	permit.o resp.o cursor.o recsio.o check.o gname.o gtime.o perms.o \
	misc.o nfcomment.o miscio.o tsearch.o
	@echo loading newsinput
	@cc $(CFLAGS) $(LFLAGS) -o newsinput newsinput.o datein.o find.o \
		newsgroup.o lrsp.o note.o pagein.o permit.o resp.o cursor.o \
		recsio.o check.o gname.o gtime.o perms.o misc.o miscio.o \
		nfcomment.o miscio.o tsearch.o -l$(TERMCAP)

bnewsinput: bnewsinput.o datein.o find.o newsgroup.o lrsp.o note.o pagein.o \
	permit.o resp.o cursor.o recsio.o check.o gname.o gtime.o perms.o \
	misc.o nfcomment.o bnews.o newsfuncs.o miscio.o tsearch.o
	@echo loading bnewsinput
	@cc $(CFLAGS) $(LFLAGS) -o bnewsinput bnewsinput.o datein.o find.o \
		newsgroup.o lrsp.o note.o pagein.o permit.o resp.o cursor.o \
		recsio.o check.o gname.o gtime.o perms.o misc.o miscio.o \
		nfcomment.o miscio.o bnews.o newsfuncs.o tsearch.o -l$(TERMCAP)

newsoutput: newsoutput.o newsout.o newsdump.o newsgroup.o pattern.o next.o \
	lrsp.o pageout.o times.o cursor.o recsio.o check.o gname.o gtime.o \
	perms.o misc.o expand.o strsave.o miscio.o
	@echo loading newsoutput
	@cc $(CFLAGS) $(LFLAGS) -o newsoutput newsoutput.o newsout.o \
		newsdump.o newsgroup.o pattern.o next.o lrsp.o pageout.o \
		times.o cursor.o recsio.o check.o gname.o gtime.o perms.o \
		misc.o miscio.o expand.o strsave.o -l$(TERMCAP)

nfarchive: nfarchive.o archiver.o compress.o dmpnote.o dmpresp.o pattern.o \
	next.o lrsp.o note.o pagein.o permit.o resp.o pageout.o cursor.o \
	recsio.o check.o gname.o gtime.o perms.o misc.o expand.o strsave.o \
	nfcomment.o miscio.o
	@echo loading nfarchive
	@cc $(CFLAGS) $(LFLAGS) -o nfarchive nfarchive.o archiver.o \
		compress.o dmpnote.o dmpresp.o pattern.o next.o lrsp.o note.o \
		pagein.o permit.o resp.o pageout.o cursor.o recsio.o check.o \
		gname.o gtime.o perms.o misc.o miscio.o expand.o nfcomment.o \
		strsave.o -l$(TERMCAP)

#
#	User subroutines
#

subs:	nfcomment.o
	ar r libnfcom.a nfcomment.o
	@echo "Remember to place libnfcom.a in /usr/lib"

clean:
	-rm -f *.o 
	-rm -f main nfpipe nfprint nfstats checknotes prepass nfxmit nfrcv
	-rm -f mknf rmnf newsinput bnewsinput newsoutput nfarchive

clobber: clean
	rm -f Makefile [a-z]* C* T*

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

# dependancies

access.o : parms.h structs.h
adnote.o : parms.h structs.h
adresp.o : parms.h structs.h
archiver.o : parms.h structs.h
asearch.o : parms.h structs.h
bnews.o: parms.h structs.h newsgate.h
bnewsinput.o : parms.h structs.h globs.h newsgate.h main.i
check.o : parms.h structs.h
checknotes.o : parms.h structs.h globs.h main.i
compress.o : parms.h structs.h
control.o : parms.h structs.h
cursor.o : parms.h structs.h
datein.o : parms.h structs.h
delete.o : parms.h structs.h
dmpnote.o : parms.h structs.h
dmpresp.o : parms.h structs.h
dropt.o : parms.h structs.h
dsply.o : parms.h structs.h
find.o : parms.h structs.h
gdate.o : parms.h structs.h
strsave.o :
getnet.o : parms.h structs.h net.h
gname.o : parms.h structs.h
gtext.o : parms.h structs.h
gtime.o : parms.h structs.h
help.o : parms.h structs.h
index.o : parms.h structs.h newsgate.h
ldmisc.o : parms.h structs.h
limindx.o : parms.h structs.h
loadem.o : parms.h structs.h
lprnote.o : parms.h structs.h
lrsp.o : parms.h structs.h
mailit.o : parms.h structs.h
main.o : parms.h structs.h globs.h main.i
misc.o : parms.h structs.h
miscio.o : parms.h structs.h
mknf.o : parms.h structs.h globs.h main.i
nameparse.o :
newsdump.o : parms.h structs.h newsgate.h
newsgroup.o : parms.h structs.h newsgate.h
newsinput.o : parms.h structs.h globs.h newsgate.h main.i
newsout.o : parms.h structs.h newsgate.h
newsoutput.o : parms.h structs.h globs.h newsgate.h main.i
next.o : parms.h structs.h
nfalias.o : parms.h structs.h net.h
nfarchive.o : parms.h structs.h globs.h main.i
nfcomment.o : parms.h structs.h
nfpipe.o : parms.h structs.h globs.h main.i
nfprint.o : parms.h structs.h globs.h main.i
nfrcv.o : parms.h structs.h net.h globs.h main.i
nfrec.o : parms.h structs.h net.h globs.h main.i
nfsend.o : parms.h structs.h
nfstats.o : parms.h structs.h globs.h main.i
nfxmit.o : parms.h structs.h net.h globs.h main.i
nfbatch.o : parms.h structs.h net.h globs.h main.i
note.o : parms.h structs.h
pagein.o : parms.h structs.h
pageout.o : parms.h structs.h
pattern.o : parms.h structs.h
permit.o : parms.h structs.h
perms.o : parms.h structs.h
prepass.o :
preptxt.o : parms.h structs.h
prtind.o : parms.h structs.h
readem.o : parms.h structs.h newsgate.h
recsio.o : parms.h structs.h
resp.o : parms.h structs.h
rmnf.o : parms.h structs.h globs.h main.i
savtxt.o : parms.h structs.h
strsave.o :
talkto.o : parms.h structs.h
times.o : parms.h structs.h
tsearch.o : parms.h structs.h
canon.h :
doc.h :
globs.h :
net.h :
newsgate.h :
parms.h :
structs.h :

# end of dependancies
