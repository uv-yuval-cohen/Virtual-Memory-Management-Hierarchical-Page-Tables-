CC=g++
CXX=g++
AR=ar
RANLIB=ranlib

LIBSRC=VirtualMemory.cpp
LIBOBJ=$(LIBSRC:.cpp=.o)

HEADERS= VirtualMemoryHelpers.h VirtualMemory.h 

INCS=-I.
CFLAGS = -Wall -std=c++11 -g $(INCS)
CXXFLAGS = -Wall -std=c++11 -g $(INCS)

VMLIB = libVirtualMemory.a
TARGETS = $(VMLIB)

TAR=tar
TARFLAGS=-cvf
TARNAME=ex4.tar
TARSRCS=$(LIBSRC) $(HEADERS) Makefile README

all: $(TARGETS)

$(VMLIB): $(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	$(RM) $(TARGETS) $(LIBOBJ) *~ *core

tar: clean
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)

depend:
	makedepend -- $(CFLAGS) -- $(LIBSRC)

.PHONY: all clean tar depend
