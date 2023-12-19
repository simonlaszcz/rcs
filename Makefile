# $Id: makefile.,v 1.4 1991/01/30 17:33:42 apratt Exp $
# Copyright (C) 1982, 1988, 1989 Walter Tichy
#   Copyright 1990 by Paul Eggert
#   Distributed under license by the Free Software Foundation, Inc.
#
# This file is part of RCS.
#
# RCS is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 1, or (at your option)
# any later version.
#
# RCS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with RCS; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
#
# Report problems and direct all questions to:
#
#    rcs-bugs@cs.purdue.edu
#
#               INSTRUCTIONS
#               ============
#
#	This is a stripped down makefile just for Atari TOS.
#   conf.h contains Atari ST specific definitions
#   See unused/ for older makefiles
#   	- slaszcz 2023
CC=m68k-atari-mint-gcc
AS=m68k-atari-mint-as -m68000
AR=m68k-atari-mint-ar
# using stdlib
CFLAGS=-s -O2 -g0
LDFLAGS=
# tried libcmini but doesn't build
CRT=

PREF=_

# binary commands
BCOMMANDS = $(PREF)ci.ttp $(PREF)ident.ttp $(PREF)rcs.ttp $(PREF)rcsdiff.ttp $(PREF)rlog.ttp $(PREF)co.ttp # $(PREF)rcsmerge.ttp 

# all commands
RCSCOMMANDS = ${BCOMMANDS} # merge 

all :: ${RCSCOMMANDS}

clean ::
	rm -f a.* *.o conf.error

CIFILES = ci.o rcslex.o rcssyn.o rcsgen.o rcsedit.o rcskeys.o rcsmap.o \
	rcsrev.o rcsutil.o rcsfnms.o partime.o maketime.o rcskeep.o rcsfcmp.o system.o
$(PREF)ci.ttp : ${CIFILES}
	${CC} ${CFLAGS} ${CRT} ${CIFILES} ${LDFLAGS} -o $@

COFILES = co.o rcslex.o rcssyn.o rcsgen.o rcsedit.o rcskeys.o rcsmap.o \
	rcsrev.o rcsutil.o rcsfnms.o partime.o maketime.o system.o
$(PREF)co.ttp : ${COFILES} 
	${CC} ${CFLAGS} ${CRT} ${COFILES}  ${LDFLAGS} -o $@

$(PREF)ident.ttp : ident.o rcsmap.o
	${CC} ${CFLAGS} ident.o rcsmap.o system.o ${LDFLAGS} -o $@

RLOG = rlog.o rcslex.o rcsmap.o rcssyn.o rcsrev.o rcsutil.o partime.o \
	maketime.o rcsfnms.o system.o
$(PREF)rlog.ttp : ${RLOG} 
	${CC} ${CFLAGS} ${CRT} ${RLOG}  ${LDFLAGS} -o $@

RCS = rcs.o rcslex.o rcssyn.o rcsrev.o rcsutil.o rcsgen.o rcsedit.o rcskeys.o \
	rcsmap.o rcsfnms.o system.o
$(PREF)rcs.ttp : ${RCS} 
	${CC} ${CFLAGS} ${CRT} ${RCS}  ${LDFLAGS} -o $@

RCSDIFF = rcsdiff.o rcsutil.o rcsfnms.o rcsmap.o rcsrev.o rcssyn.o rcslex.o \
	maketime.o partime.o system.o
$(PREF)rcsdiff.ttp : ${RCSDIFF} 
	${CC} ${CFLAGS} ${CRT} ${RCSDIFF}  ${LDFLAGS} -o $@

#RCSMERGE = rcsmerge.o rcsutil.o rcsfnms.o rcsmap.o rcsrev.o rcssyn.o rcslex.o
#$(PREF)rcsmerge.ttp : ${RCSMERGE}  
#	${CC} ${CFLAGS} ${CRT} ${RCSMERGE}  ${LDFLAGS} -o $@
