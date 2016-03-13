UNAME := $(shell uname)

ifeq  "$(UNAME)" "Linux"
-include makefile.linux
else
ifeq "$(UNAME)" "Darwin"
-include makefile.osx
else #win
-include makefile.win
endif
endif

ifndef BUILDDIR
BUILDDIR=.
endif

OBJDIR=$(BUILDDIR)/obj
DEPSDIR=$(BUILDDIR)/deps

include makefile.main

gen:
	@genmake
