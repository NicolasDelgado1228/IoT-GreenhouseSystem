#
# Makefile for make
#
# Compiler: gcc
#

PROGS   = greenhouseFSM
CFLAGST = -D_REENTRANT -Wall
LIBS    = pthread

all: $(PROGS)

greenhouseFSM : phtrdsMsgLyr.o greenhouse.o
	$(CC) $(CFLAGST) -o greenhouseFSM greenhouse.o phtrdsMsgLyr.o -l $(LIBS)

phtrdsMsgLyr.o : phtrdsMsgLyr.c phtrdsMsgLyr.h greenhouse.h
	$(CC) $(CFLAGST) -g -c phtrdsMsgLyr.c

greenhouse.o : greenhouse.c greenhouse.h
	$(CC) $(CFLAGST) -g -c greenhouse.c

clean:
	rm -f $(PROGS) *~ *.o
