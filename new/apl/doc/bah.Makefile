#   Copyright (c) 1984 by Digital Equipment Corporation, Maynard MA.
#                         All Rights Reserved.
#
# This software is provided under  a  license  and  may  be  used  and
# copied  only  in  accordance with the terms of such license and with
# the inclusion of the above copyright notice.  This software  or  any
# other   copies  thereof  may  not  be  provided  or  otherwise  made
# available to any other person.  No title to  and  ownership  of  the
# software is hereby transferred.
#
# The information in  this  software  is  subject  to  change  without
# notice  and  should  not  be  construed  as  a commitment by Digital
# Equipment Corporation.
#
# Digital assumes no responsibility for the use or reliability of its
# software on equipment which is not supplied by Digital.

SOURCES= man.0 man.1 man.2 man.3 man.4 man.5 man.6 man.7 man.8 man.9\
	purdue charset syscom quadf ibeams
INDENT= 0.7i
WIDTH=  7.0i

nman:	${SOURCES}
	echo ".nr PO" ${INDENT} > man.indent
	echo ".nr LL" ${WIDTH} >> man.indent
	echo ".nr LT" ${WIDTH} >> man.indent
	echo ".nr FL" ${WIDTH} >> man.indent
	nroff -ms man.indent man.? | tr -d "\010_" > manual.nrf

tman:	${SOURCES}
	echo ".nr LL" ${WIDTH} > man.indent
	echo ".nr LT" ${WIDTH} >> man.indent
	echo ".nr FL" ${WIDTH} >> man.indent
	troff -ms -t man.indent man.? > manual.trf

clean:
	rm -f manual.nrf manual.trf man.indent
