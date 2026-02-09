#! /usr/bin/make -f

AS=ca65
ASFLAGS=-t c64

SOURCES=$(wildcard *.s)
OBJS=$(SOURCES:.s=.o)

.PHONY: all clean clean-temps

all: ../LAMAlib.lib ../LAMAlib128.lib ../LAMAlib20.lib
	@echo "Cleaning up temporary files..."
	@rm -f *.lib.temp-* ../LAMAlib*.lib.temp-* 2>/dev/null || true
	@echo "Build complete!"

clean: clean-temps
	rm -f ../LAMAlib.lib ../LAMAlib128.lib ../LAMAlib20.lib
	rm -f *.o

clean-temps:
	@echo "Removing ar65 temporary files..."
	@rm -f *.lib.temp-* ../LAMAlib*.lib.temp-* 2>/dev/null || true

../LAMAlib.lib: $(OBJS) systemaddresses_c64.o
	$(foreach obj, $(filter-out $(OBJS) systemaddresses_c64.o systemaddresses_c128.o systemaddresses_vc20.o, $(wildcard *.o)), rm -f $(obj))
	@rm -f $@ $@.temp-* 2>/dev/null || true
	ar65 a $@ $+
	@rm -f $@.temp-* 2>/dev/null || true

../LAMAlib128.lib: $(OBJS) systemaddresses_c128.o
	@rm -f $@ $@.temp-* 2>/dev/null || true
	ar65 a $@ $+
	@rm -f $@.temp-* 2>/dev/null || true

../LAMAlib20.lib: $(OBJS) systemaddresses_vc20.o
	@rm -f $@ $@.temp-* 2>/dev/null || true
	ar65 a $@ $+
	@rm -f $@.temp-* 2>/dev/null || true

systemaddresses_c64.o: systemdependencies.as
	$(AS) -tc64 $< -o $@

systemaddresses_c128.o: systemdependencies.as
	$(AS) -tc128 $< -o $@

systemaddresses_vc20.o: systemdependencies.as
	$(AS) -tvic20 $< -o $@
