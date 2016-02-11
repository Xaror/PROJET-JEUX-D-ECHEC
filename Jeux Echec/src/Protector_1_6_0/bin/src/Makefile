#
# $Id: Makefile,v 1.8 2010-12-25 21:24:17 raimund Exp $
#
# Call './configure' first in order to create a dependencies file.
#
# Then call 'make install' or simply 'make' in order to create
# the protector chess program.
#
# Rerun './configure' after adding new source files.
#

MAKEFILE_DEPEND = depend
MAKE = make
MAKEDEP = gccmakedep
#COMPILER=ICC
COMPILER=GCC

ifeq ($(COMPILER),ICC)
   CC=icc
   CPPC=icc -w0
   LN=icc
   CFLAGS = -ansi -O3 -DNDEBUG -DTARGET_LINUX -DSUPPORT_SMP
   CDFLAGS = -DICC
else         
   CC=gcc
   CPPC=g++
   LN=g++
   CFLAGS = -O3 -Wall -Wunused -Wuninitialized -Wimplicit-function-declaration \
            -Wmissing-prototypes -Wmissing-declarations -ansi -pedantic \
            -pthread -Wno-long-long \
            -DNDEBUG -DTARGET_LINUX -DSUPPORT_SMP -DGCC_INLINE_STYLE
   CDFLAGS = 
endif

protectorobjects = protector.o bitboard.o position.o fen.o io.o \
                   movegeneration.o matesearch.o keytable.o hash.o test.o \
                   pgn.o tools.o search.o evaluation.o coordination.o \
                   xboard.o book.o tablebase.o kpkp.o

protectorsources = $(protectorobjects:.o=.c) egtb.cpp
#protectorsources = $(protectorobjects:.o=.c)

.c.o:
	$(CC) $(CFLAGS) $(CDFLAGS) -c $*.c

.cpp.o:
	$(CPPC) -O3 -c $*.cpp

protector: $(protectorobjects) egtb.o
	$(LN) -lstdc++ -pthread -o protector $(protectorobjects) -lm egtb.o -lm	

# protector: $(protectorobjects) # egtb.o
#	$(LN) -lstdc++ -pthread -o protector $(protectorobjects) -lm
	
install:
	$(MAKE) clean
	$(MAKE)
	sudo install ./protector /usr/games

debug:
	$(MAKE) clean
	$(MAKE) protector CFLAGS='-DTRACE_EVAL'

test:
	$(MAKE) clean
	$(MAKE) protector CFLAGS='-DCHECK_EVAL'
	time protector -m -e /home/heid/egtb

alltests:
	$(MAKE) debug
	time protector -m -t problems.pgn

easytests:
	$(MAKE) debug
	time protector -t easy.pgn -e /home/heid/egtb

hamming: hamming.o
	$(CC) -O3 -Wall -ansi -o hamming hamming.o

.PHONY: dep
dep:
	touch $(MAKEFILE_DEPEND)
	$(MAKEDEP) $(CFLAGS) $(CDFLAGS) -cc $(CC) -f $(MAKEFILE_DEPEND) $(protectorsources) 
	rm depend.bak

.PHONY: clean
clean:
	@rm -f *.o *.bak *.bin *.lrn game.* log.* *~ .#*

.PHONY: pretty
pretty: 
	indent -bad -bap -sob -npsl -npcs -bl -bli0 -i3 -ci3 -nut *.c *.h
	
DEPEND = $(shell test -r $(MAKEFILE_DEPEND) && echo "true")

ifeq ($(DEPEND),true)
	include $(MAKEFILE_DEPEND)
endif
