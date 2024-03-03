#! /usr/bin/make -f

AS=ca65
ASFLAGS=-t c64

SOURCES=$(wildcard *.s)
OBJS=$(SOURCES:.s=.o)

.PHONY: all clean

all: ../LAMAlib.lib

clean:
	rm -f ../LAMAlib.lib
	rm -f *.o

../LAMAlib.lib:	$(OBJS)
	$(foreach obj, $(filter-out $(OBJS), $(wildcard *.o)), rm -f $(obj))
	rm -f ../LAMAlib.lib
	ar65 a ../LAMAlib.lib $+
