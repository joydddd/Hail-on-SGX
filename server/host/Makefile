# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

include ../config.mk
####################################

TESTSOURCES = $(wildcard test*.cpp)
# names of test executables
TESTS       = $(TESTSOURCES:%.cpp=%host)

######################################

CFLAGS=$(shell pkg-config oehost-$(C_COMPILER) --cflags)
CXXFLAGS=$(shell pkg-config oehost-$(CXX_COMPILER) --cflags)
LDFLAGS=$(shell pkg-config oehost-$(C_COMPILER) --libs)
INCDIR=$(shell pkg-config oehost-$(C_COMPILER) --variable=includedir)

CFLAGS+= -I ../../include -I ..
CXXFLAGS+= -I ../../include -I ..
LDFLAGS+= -I ../../include -I ..

all: build

pre:
	@ echo "Compilers used: $(CC), $(CXX)"
	oeedger8r ../$(PROJECTNAME).edl --untrusted \
		--search-path $(INCDIR) \
		--search-path $(INCDIR)/openenclave/edl/sgx
	$(CC) -c $(CFLAGS) $(PROJECTNAME)_u.c

build: pre

# Automatically generate any build rules for test*.cpp files
define make_tests
	$(CXX) -c $(CXXFLAGS) $(1).cpp
	$(CXX) -o $(1)host $(PROJECTNAME)_u.o $(1).o $(LDFLAGS)
endef

test%host: test%.cpp
	$(call make_tests,test$*)

	
tests: pre $(TESTS)

runtests: 

clean:
	rm -f $(PROJECTNAME)host $(PROJECTNAME)host.o $(PROJECTNAME)_u.o $(PROJECTNAME)_u.c $(PROJECTNAME)_u.h $(PROJECTNAME)_args.h
	rm -f *.o $(TESTS)
