# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.
include ../config.mk

SRCDIR := src
BUILDDIR := build
TESTDIR := tests
KEYDIR := keys

##########################################
EXECUTABLE = $(PROJECTNAME)enc

# list of test drivers (with main()) for development
# TESTSOURCES = $(wildcard $(TESTDIR)/test*.cpp)
TESTSOURCES = $(TESTDIR)/test_logistic.cpp
# names of test executables
TESTS       = $(patsubst $(TESTDIR)/%,%,$(TESTSOURCES:.cpp=))

# list of sources used in project
SOURCES     = $(shell find $(SRCDIR) -type f -name *.cpp)
# list of objects used in project
OBJECTS     = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.o))
DEBUGOBJECTS = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=_debug.o))

###########################################
CFLAGS=$(shell pkg-config oeenclave-$(C_COMPILER) --cflags)
CXXFLAGS=$(shell pkg-config oeenclave-$(CXX_COMPILER) --cflags)
LDFLAGS=$(shell pkg-config oeenclave-$(CXX_COMPILER) --libs)
INCDIR=$(shell pkg-config oeenclave-$(C_COMPILER) --variable=includedir)
CRYPTO_LDFLAGS=$(shell pkg-config oeenclave-$(COMPILER) --variable=${OE_CRYPTO_LIB}libs)

BOOST_ROOT = /usr/local/lib/boost_1_76_0/
# CXXFLAGS+= -I $(BOOST_ROOT)
CFLAGS+= -I ../../shared/include -I .. -I include -I .
CXXFLAGS+= -I ../../shared/include -I .. -I include -I .
LDFLAGS+= -I ../../shared/include -I .. -I include -I .


all:
	$(MAKE) build
	$(MAKE) keys
	$(MAKE) sign
pre:
	@ echo "Compilers used: $(CC), $(CXX)"
	oeedger8r ../$(EDL_FILE) --trusted \
		--search-path $(INCDIR) \
		--search-path $(INCDIR)/openenclave/edl/sgx

${BUILDDIR}/%.o: ${SRCDIR}/%.cpp
	@mkdir -p ${BUILDDIR}
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) -I. -std=c++11 -o $@ $<

$(BUILDDIR)/%_debug.o: ${SRCDIR}/%.cpp
	@mkdir -p ${BUILDDIR}
	$(CXX) -g -c $(CXXFLAGS) $(INCLUDES) -I. -std=c++11 -o $@ $<

build: pre $(OBJECTS)
	@ echo "Debug Mode Off"
	sed -i 's/Debug=.*/Debug=0/g' gwas_enc.conf
	$(CC) -c $(CFLAGS) $(PROJECTNAME)_t.c -o $(BUILDDIR)/$(PROJECTNAME)_t.o
	$(CXX) -o $(EXECUTABLE) $(BUILDDIR)/$(PROJECTNAME)_t.o $(OBJECTS) $(LDFLAGS) $(CRYPTO_LDFLAGS)

debug: pre $(DEBUGOBJECTS)
	@ echo "Debug Mode On"
	sed -i 's/Debug=.*/Debug=1/g' gwas_enc.conf
	$(CC) -g -c $(CFLAGS) $(PROJECTNAME)_t.c -o $(BUILDDIR)/$(PROJECTNAME)_t_debug.o
	$(CXX) -g -o $(EXECUTABLE)_debug $(BUILDDIR)/$(PROJECTNAME)_t_debug.o $(DEBUGOBJECTS) $(LDFLAGS) $(CRYPTO_LDFLAGS)
	$(MAKE) keys
	oesign sign -e $(PROJECTNAME)enc_debug -c $(PROJECTNAME)_enc.conf -k $(KEYDIR)/private.pem

sign:
	oesign sign -e $(PROJECTNAME)enc -c $(PROJECTNAME)_enc.conf -k $(KEYDIR)/private.pem

clean:
	rm -rf $(KEYDIR)
	rm -rf $(BUILDDIR)
	rm -f $(EXECUTABLE) $(EXECUTABLE).signed $(EXECUTABLE)_debug $(EXECUTABLE)_debug.signed \
	$(PROJECTNAME)_t.h $(PROJECTNAME)_t.c $(PROJECTNAME)_args.h $(PROJECT_NAME).signed
	rm -f test_*

keys:
	mkdir -p $(KEYDIR)
	openssl genrsa -out $(KEYDIR)/private.pem -3 3072
	openssl rsa -in $(KEYDIR)/private.pem -pubout -out $(KEYDIR)/public.pem

CXX_NONENC_FLAGS = -std=c++1z -pedantic
CXX_NONENC_FLAGS+= -I ../../include/shared -I .. -I include -I .
# non-enclave tests
define make_tests
    $(1): CXX_NONENC_FLAGS += -g3 -DDEBUG -DENC_TEST
    $(1): $$(TESTDIR)/$(1).cpp $$(SOURCES)
	$$(CXX) $$(CXX_NONENC_FLAGS) $$(SOURCES) $$(TESTDIR)/$(1).cpp -o $(1)
	# valgrind -s --leak-check=yes ./$(1)
	./$(1)
endef
$(foreach test, $(TESTS), $(eval $(call make_tests,$(test))))

tests: $(TESTS)