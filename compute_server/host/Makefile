# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.
SRCDIR := src
SHAREDBUILDDIR := ../../shared/build
BUILDDIR := build
TESTDIR := tests
include ../config.mk
####################################
EXECUTABLE = $(PROJECTNAME)host
CONFIG = compute_server_config.json

TESTSOURCES = $(wildcard $(TESTDIR)/test*.cpp)
# names of test executables
TESTS       = $(patsubst $(TESTDIR)/%,%,$(TESTSOURCES:.cpp=))
# List of source files for your thread library
SOURCES = $(shell find $(SRCDIR) -type f -name *.cpp)

# Generate the names of the thread library's object files
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.o))
SHAREDOBJECTS += $(shell find $(SHAREDBUILDDIR) -type f -name *.o)
OBJECTS += $(SHAREDOBJECTS)
DEBUGOBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=_debug.o))
DEBUGOBJECTS += $(SHAREDOBJECTS)

NONOEOBJECTS = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=_nonoe.o))
NONOEOBJECTS += $(shell find $(SHAREDBUILDDIR) -type f -name *.o)
CXX_NONENC_FLAGS = -std=c++14 -pedantic
INC := -I ../../shared/include -I .. -pthread -I include -I .
CRYPTFLAG := -lcryptopp -lmbedcrypto -lmbedtls
CURLFLAG := -lcurl

######################################

# Use Mac library if on Mac
ifeq ($(detected_OS),Darwin)  # Important to not have a space after the comma
	LIBTHREAD = -lboost_thread-mt
	LIBCHRONO = -lboost_chrono-mt
else
	LIBTHREAD = -lboost_thread -lboost_system
	LIBCHRONO = -lboost_chrono
endif

CFLAGS=$(shell pkg-config oehost-$(C_COMPILER) --cflags)
CXXFLAGS=$(shell pkg-config oehost-$(CXX_COMPILER) --cflags)
LDFLAGS=$(shell pkg-config oehost-$(C_COMPILER) --libs)
INCDIR=$(shell pkg-config oehost-$(C_COMPILER) --variable=includedir)

CFLAGS+= $(INC) 
CXXFLAGS+= $(INC) 
LDFLAGS+= $(INC) $(LIBTHREAD)
CXX_NONENC_FLAGS += $(INC)

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
	$(CXX) -o $(EXECUTABLE) $(BUILDDIR)/$(PROJECTNAME)_u.o $(OBJECTS) $(LDFLAGS) ${CRYPTFLAG} ${CURLFLAG}

run: 
	./$(EXECUTABLE) $(CONFIG)

simulate:
	./$(EXECUTABLE) $(CONFIG) --simulate

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
	cd ../../shared && $(MAKE) && cd ../compute_server/host
	$(CXX) -g -o $(EXECUTABLE)_debug $(BUILDDIR)/$(PROJECTNAME)_u.o $(DEBUGOBJECTS) $(LDFLAGS) ${CRYPTFLAG} ${CURLFLAG}

debugrun:
	./$(EXECUTABLE)_debug $(CONFIG)

clean:
	rm -rf ${BUILDDIR}
	rm -f $(PROJECTNAME)host $(PROJECTNAME)host.o $(PROJECTNAME)_u.o $(PROJECTNAME)_u.c $(PROJECTNAME)_u.h $(PROJECTNAME)_args.h $(PROJECTNAME).signed
	rm -f *.o $(TESTS) *.out *_debug test_*
	rm -f *_nonoe

# Generic rules for compiling a source file to an non oe object file
${BUILDDIR}/%_nonoe.o: ${SRCDIR}/%.cpp
	@mkdir -p ${BUILDDIR}
	${CXX} -DNON_OE ${CXX_NONENC_FLAGS} -c -o $@ $<


ENCLAVE_NONOEOBJECTS = $(wildcard ../enclave/build/*_nonoe.o)

nonoe: $(NONOEOBJECTS)
	$(CXX) $(CXX_NONENC_FLAGS) $(LIBTHREAD) $(LIBCHRONO) ${CRYPTFLAG} ${CURLFLAG} $(NONOEOBJECTS) $(ENCLAVE_NONOEOBJECTS) -o $(PROJECTNAME)_nonoe

run_nonoe: $(PROJECTNAME)_nonoe
	./$< $(CONFIG)

.PHONY: all build clean run debug simulate