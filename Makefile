#! /usr/bin/make -f

CC65DIR=/usr/share/cc65
CC65LIBDIR=$(CC65DIR)/lib
CC65ASMINCDIR=$(CC65DIR)/asminc
CC65CFGDIR=$(CC65DIR)/asminc
#
AS=ca65
ASFLAGS=-t c64

SOURCES=$(wildcard lib-functions/*.s)
OBJS=$(SOURCES:.s=.o)

.PHONY:	all clean install uninstall

all:	LAMAlib.lib

clean:
	rm -f LAMAlib.lib
	rm -f lib-functions/*.o

install:	all
	echo mkdir -p $(CC65LIBDIR) $(CC65ASMINCDIR) $(CC65CFGDIR)
	echo cp LAMAlib.lib $(CC65LIBDIR)
	echo cp LAMAlib*.inc $(CC65ASMINCDIR)
	echo cp c64-basicfriendly-asm.cfg $(CC65CFGDIR)
	echo -e $green
	echo -e "*******************************************************************************"
	echo -e "* Congratulations, LAMAlib has been installed!                                *"
	echo -e "*                                                                             *"
	echo -e "* To use it, add the line                                                     *"
	echo -e "* \033[0;36m.include "LAMAlib.inc"${green} to the top of your assembler file and assemble with    *"
	echo -e "* \033[0;36mcl65 yourprog.s -lib LAMAlib.lib -C c64-asm.cfg -o yourprog.prg${green}             *"
	echo -e "* There is no overhead to your assembled program for unused functions         *"
	echo -e "*******************************************************************************${nocolor}"

uninstall:
	rm -f $(CC65LIBDIR)/LAMAlib.lib
	rm -f $(CC65ASMINCDIR)/LAMAlib*.inc
	rm -f $(CC65CFGDIR)/c64-basicfriendly-asm.cfg

LAMAlib.lib:	$(OBJS)
	ar65 a LAMAlib.lib $+
