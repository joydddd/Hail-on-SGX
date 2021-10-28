# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.
SRCDIR := src
BUILDDIR := build
TESTDIR := tests
include ../config.mk
####################################
EXECUTABLE = $(PROJECTNAME)host

TESTSOURCES = $(wildcard $(TESTDIR)/test*.cpp)
# names of test executables
TESTS       = $(patsubst $(TESTDIR)/%,%,$(TESTSOURCES:.cpp=))

# List of source files for your thread library
SOURCES := $(shell find $(SRCDIR) -type f -name *.cpp)
# Generate the names of the thread library's object files
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.o))
DEBUGOBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=_debug.o))

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

CFLAGS+= -I ../../include -I .. -pthread -I include -I . 
CXXFLAGS+= -I ../../include -I .. -pthread -I include -I .
LDFLAGS+= -I ../../include -I .. -pthread -I include -I . $(LIBTHREAD)

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
	$(CXX) -o $(EXECUTABLE) $(BUILDDIR)/$(PROJECTNAME)_u.o $(OBJECTS) $(LDFLAGS)

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
	$(foreach test,$(TESTS),./$(test) ../enclave/$(PROJECTNAME)enc.signed;)

debug: pre $(DEBUGOBJECTS) 
	$(CC) -c -g $(CFLAGS) $(PROJECTNAME)_u.c -o $(BUILDDIR)/$(PROJECTNAME)_u_debug.o
	$(CXX) -g -o $(EXECUTABLE)_debug $(BUILDDIR)/$(PROJECTNAME)_u_debug.o $(DEBUGOBJECTS) $(LDFLAGS)

debugrun:
	$(DEBUGER) ./$(EXECUTABLE)_debug $(HOST_PORT) ../enclave/$(PROJECTNAME)enc_debug.signed

clean:
	rm -rf ${BUILDDIR}
	rm -f $(PROJECTNAME)host $(PROJECTNAME)host.o $(PROJECTNAME)_u.o $(PROJECTNAME)_u.c $(PROJECTNAME)_u.h $(PROJECTNAME)_args.h $(PROJECT_NAME).signed
	rm -f *.o $(TESTS) *.out *_debug test_*
