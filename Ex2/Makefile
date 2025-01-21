CXX=g++
RANLIB=ranlib

INCS=-I.
CXXFLAGS = -Wall -std=c++11 -g $(INCS)

LIBSRC= IDManager.cpp timer_utils.cpp Thread.cpp uthreads.cpp macros_and_errors.cpp
LIBOBJ=$(LIBSRC:.cpp=.o)

OSMLIB = libuthreads.a
TARGETS = $(OSMLIB)

TAR= tar
TARFLAGS= -cvf
TARNAME= ex2.tar
TARSRCS= $(LIBSRC) Makefile README IDManager.h timer_utils.h Thread.h macros_and_errors.h

all: $(TARGETS)

$(TARGETS): $(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	$(RM) $(TARGETS) $(OSMLIB) $(OBJ) $(LIBOBJ) *~ *core

depend:
	makedepend -- $(CFLAGS) -- $(SRC) $(LIBSRC)

tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)

