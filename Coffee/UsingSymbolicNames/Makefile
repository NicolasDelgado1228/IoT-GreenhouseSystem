#
# Makefile for make
#
# Compiler: gcc
#

PROGS   = CoffeeMachineFSMtrc
CFLAGST = -D_REENTRANT -Wall
LIBS    = pthread

all: $(PROGS)

CoffeeMachineFSMtrc : phtrdsMsgLyr.o CoffeeMachineTrc.o
	$(CC) $(CFLAGST) -o CoffeeMachineFSMtrc CoffeeMachineTrc.o phtrdsMsgLyr.o -l $(LIBS)

phtrdsMsgLyr.o : phtrdsMsgLyr.c phtrdsMsgLyr.h pMLusrConf.h
	$(CC) $(CFLAGST) -g -c phtrdsMsgLyr.c

CoffeeMachineTrc.o : CoffeeMachineTrc.c pMLusrConf.h
	$(CC) $(CFLAGST) -g -c CoffeeMachineTrc.c

clean:
	rm -f $(PROGS) *~ *.o
