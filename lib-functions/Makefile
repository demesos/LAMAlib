#! /usr/bin/make -f

AS=ca65
ASFLAGS=-t c64

SOURCES=$(wildcard *.s)
OBJS=$(SOURCES:.s=.o)

.PHONY: all clean

all: ../LAMAlib.lib ../LAMAlib128.lib ../LAMAlib20.lib

clean:
	rm -f ../LAMAlib.lib
	rm -f *.o

../LAMAlib.lib: $(OBJS) systemaddresses_c64.o
	$(foreach obj, $(filter-out $(OBJS) systemaddresses_c64.o systemaddresses_c128.o systemaddresses_vc20.o, $(wildcard *.o)), rm -f $(obj))
	rm -f $@
	ar65 a $@ $+

../LAMAlib128.lib: $(OBJS) systemaddresses_c128.o
	rm -f $@
	ar65 a $@ $+

../LAMAlib20.lib: $(OBJS) systemaddresses_vc20.o
	rm -f $@
	ar65 a $@ $+

systemaddresses_c64.o: systemdependencies.as
	$(AS) -tc64 $< -o $@

systemaddresses_c128.o: systemdependencies.as
	$(AS) -tc128 $< -o $@

systemaddresses_vc20.o: systemdependencies.as
	$(AS) -tvic20 $< -o $@