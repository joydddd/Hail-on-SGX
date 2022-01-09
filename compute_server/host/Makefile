# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.
SRCDIR := src
SHAREDBUILDDIR := ../../shared/build
BUILDDIR := build
TESTDIR := tests
include ../config.mk
####################################
EXECUTABLE = $(PROJECTNAME)host

TESTSOURCES = $(wildcard $(TESTDIR)/test*.cpp)
# names of test executables
TESTS       = $(patsubst $(TESTDIR)/%,%,$(TESTSOURCES:.cpp=))
# List of source files for your thread library
SOURCES = $(shell find $(SRCDIR) -type f -name *.cpp)

# Generate the names of the thread library's object files
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.o))
OBJECTS += $(shell find $(SHAREDBUILDDIR) -type f -name *.o)
DEBUGOBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=_debug.o))
INC := -I ../../shared/include -I .. -pthread -I include -I .
CRYPTFLAG := -lcryptopp

######################################

# Use Mac library if on Mac
ifeq ($(detected_OS),Darwin)  # Important to not have a space after the comma
	LIBTHREAD = -lboost_thread-mt
	LIBCHRONO = -lboost_chrono-mt
else
	LIBTHREAD = -lboost_thread
	LIBCHRONO = -lboost_chrono
endif

CFLAGS=$(shell pkg-config oehost-$(C_COMPILER) --cflags)
CXXFLAGS=$(shell pkg-config oehost-$(CXX_COMPILER) --cflags)
LDFLAGS=$(shell pkg-config oehost-$(C_COMPILER) --libs)
INCDIR=$(shell pkg-config oehost-$(C_COMPILER) --variable=includedir)

CFLAGS+= $(INC) 
CXXFLAGS+= $(INC) 
LDFLAGS+= $(INC) $(LIBTHREAD)

all: build

pre:
	@ echo "Compilers used: $(CC), $(CXX)"
	oeedger8r ../$(PROJECTNAME).edl --untrusted \
		--search-path $(INCDIR) \
		--search-path $(INCDIR)/openenclave/edl/sgx
	@mkdir -p ${BUILDDIR}
	$(CC) -c $(CFLAGS) $(PROJECTNAME)_u.c -o $(BUILDDIR)/$(PROJECTNAME)_u.o

# Generic rules for compiling a source file to an object file
${BUILDDIR}/%.o: ${SRCDIR}/%.cpp
	@mkdir -p ${BUILDDIR}
	${CXX} ${CXXFLAGS} -c -o $@ $<

build: pre $(OBJECTS)
	cd ../../shared && $(MAKE) && cd ../compute_server/host
	$(CXX) -o $(EXECUTABLE) $(BUILDDIR)/$(PROJECTNAME)_u.o $(OBJECTS) $(LDFLAGS) ${CRYPTFLAG}

run: 
	./$(EXECUTABLE) $(HOST_PORT) ../enclave/$(PROJECTNAME)enc.signed

# Automatically generate any build rules for test*.cpp files
define make_tests
	$(CXX) -c $(CXXFLAGS) -g $(TESTDIR)/$(1).cpp
	$(CXX) -g -o $(1) $(BUILDDIR)/$(PROJECTNAME)_u.o $(1).o $(LDFLAGS)
endef


test%: $(TESTDIR)/test%.cpp
	$(call make_tests,test$*)

	
tests: pre $(TESTS)

runtests: $(TESTS)
	$(foreach test,$(TESTS),./$(test) ../enclave/$(PROJECTNAME)enc.signed;)


# Generic rules for compiling a source file to an debug object file
${BUILDDIR}/%_debug.o: ${SRCDIR}/%.cpp
	@mkdir -p ${BUILDDIR}
	${CXX} ${CXXFLAGS} -g -c -o $@ $<

debugtests: $(TESTS)
	$(MAKE) -C .. debug
	$(foreach test,$(TESTS), $(DEBUGER) ./$(test) ../enclave/$(PROJECTNAME)enc_debug.signed --debug)

debug: pre $(DEBUGOBJECTS) 
	$(CC) -c -g $(CFLAGS) $(PROJECTNAME)_u.c -o $(BUILDDIR)/$(PROJECTNAME)_u_debug.o
	$(CXX) -g -o $(EXECUTABLE)_debug $(BUILDDIR)/$(PROJECTNAME)_u_debug.o $(DEBUGOBJECTS) $(LDFLAGS)

debugrun:
	$(DEBUGER) ./$(EXECUTABLE)_debug $(HOST_PORT) ../enclave/$(PROJECTNAME)enc_debug.signed --debug

clean:
	rm -rf ${BUILDDIR}
	rm -f $(PROJECTNAME)host $(PROJECTNAME)host.o $(PROJECTNAME)_u.o $(PROJECTNAME)_u.c $(PROJECTNAME)_u.h $(PROJECTNAME)_args.h $(PROJECT_NAME).signed
	rm -f *.o $(TESTS) *.out *_debug test_*
