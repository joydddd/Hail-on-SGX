# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.
include ../config.mk


##########################################
EXECUTABLE = $(PROJECTNAME)enc

# list of test drivers (with main()) for development
TESTSOURCES = $(wildcard test*.cpp)
EXCLUSTSOURCES = enclave_new.cpp enclave_old.cpp

# list of sources used in project
SOURCES     = $(wildcard *.cpp)
SOURCES     := $(filter-out $(TESTSOURCES) $(EXCLUSTSOURCES), $(SOURCES))
# list of objects used in project
OBJECTS     = $(SOURCES:%.cpp=%.o)

###########################################
CFLAGS=$(shell pkg-config oeenclave-$(C_COMPILER) --cflags)
CXXFLAGS=$(shell pkg-config oeenclave-$(CXX_COMPILER) --cflags)
LDFLAGS=$(shell pkg-config oeenclave-$(CXX_COMPILER) --libs)
INCDIR=$(shell pkg-config oeenclave-$(C_COMPILER) --variable=includedir)
CRYPTO_LDFLAGS=$(shell pkg-config oeenclave-$(COMPILER) --variable=${OE_CRYPTO_LIB}libs)

BOOST_ROOT = /usr/local/lib/boost_1_76_0/
# CXXFLAGS+= -I $(BOOST_ROOT)
CFLAGS+= -I ../../include -I ..
CXXFLAGS+= -I ../../include -I ..
LDFLAGS+= -I ../../include -I ..


all:
	$(MAKE) build
	$(MAKE) keys
	$(MAKE) sign

build: 
	@ echo "Compilers used: $(CC), $(CXX)"
	@ echo "Debug Mode Off"
	sed -i 's/Debug=.*/Debug=0/g' gwas_enc.conf
	oeedger8r ../$(EDL_FILE) --trusted \
		--search-path $(INCDIR) \
		--search-path $(INCDIR)/openenclave/edl/sgx
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) -I. -std=c++11 $(SOURCES)
	$(CC) -c $(CFLAGS) -I. $(PROJECTNAME)_t.c -o $(PROJECTNAME)_t.o
	$(CXX) -o $(EXECUTABLE) $(PROJECTNAME)_t.o $(OBJECTS) $(LDFLAGS) $(CRYPTO_LDFLAGS)

debug:
	@ echo "Compilers used: $(CC), $(CXX)"
	@ echo "Debug Mode On"
	sed -i 's/Debug=.*/Debug=1/g' gwas_enc.conf
	oeedger8r ../$(EDL_FILE) --trusted \
		--search-path $(INCDIR) \
		--search-path $(INCDIR)/openenclave/edl/sgx
	$(CXX) -g -c $(CXXFLAGS) $(INCLUDES) -I. -std=c++11 $(SOURCES)
	$(CC) -g -c $(CFLAGS) -I. $(PROJECTNAME)_t.c -o $(PROJECTNAME)_t.o
	$(CXX) -g -o $(EXECUTABLE) $(PROJECTNAME)_t.o $(OBJECTS) $(LDFLAGS) $(CRYPTO_LDFLAGS)
	$(MAKE) keys
	$(MAKE) sign

sign:
	oesign sign -e $(PROJECTNAME)enc -c $(PROJECTNAME)_enc.conf -k private.pem

clean:
	rm -f $(EXECUTABLE) $(EXECUTABLE).signed private.pem public.pem \
	$(PROJECTNAME)_t.o $(PROJECTNAME)_t.h $(PROJECTNAME)_t.c $(PROJECTNAME)_args.h $(PROJECT_NAME).signed
	rm -f $(OBJECTS) $(ENCLAVE_O) *.o

keys:
	openssl genrsa -out private.pem -3 3072
	openssl rsa -in private.pem -pubout -out public.pem