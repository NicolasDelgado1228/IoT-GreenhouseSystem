#
# Makefile for make
#
# Compiler: gcc
#

PROGS   = CoffeeMachineFSM CoffeeMachineFSMtrc
CFLAGST = -D_REENTRANT -Wall
LIBS    = pthread

all: $(PROGS)

CoffeeMachineFSM : phtrdsMsgLyr.o CoffeeMachine.o
	$(CC) $(CFLAGST) -o CoffeeMachineFSM CoffeeMachine.o phtrdsMsgLyr.o -l $(LIBS)

CoffeeMachineFSMtrc : phtrdsMsgLyr.o CoffeeMachineTrc.o
	$(CC) $(CFLAGST) -o CoffeeMachineFSMtrc CoffeeMachineTrc.o phtrdsMsgLyr.o -l $(LIBS)

phtrdsMsgLyr.o : phtrdsMsgLyr.c phtrdsMsgLyr.h pMLusrConf.h
	$(CC) $(CFLAGST) -g -c phtrdsMsgLyr.c

CoffeeMachine.o : CoffeeMachine.c pMLusrConf.h
	$(CC) $(CFLAGST) -g -c CoffeeMachine.c

CoffeeMachineTrc.o : CoffeeMachineTrc.c pMLusrConf.h
	$(CC) $(CFLAGST) -g -c CoffeeMachineTrc.c

clean:
	rm -f $(PROGS) *~ *.o
